#include <deque>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <system_error>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include <imguix/utils/path_utils.hpp>
#include <imguix/utils/strip_json_comments.hpp>
#include <imguix/core/i18n/PluralRules.hpp>

namespace ImGuiX::I18N {

    LangStore::LangStore(std::string base_dir, std::string default_lang)
        : m_base_dir(std::move(base_dir)),
          m_default_lang(std::move(default_lang)),
          m_current_lang(m_default_lang),
          m_plural_rules(std::make_unique<PluralRules>()) { // always present; has built-ins
        // Load fallback language (default) once.
        m_en_map = load_language_map(m_default_lang);
        m_current_map = &m_en_map;

        // Try optional plurals.json in <base_dir>/plurals.json (ignore if absent).
        try_load_plural_rules_from_default_location();
    }

    void LangStore::set_base_dir(std::string base_dir, std::string default_lang) {
        const std::string previous_default_lang = m_default_lang;
        const std::string previous_current_lang = m_current_lang;
        const bool was_using_default = previous_current_lang.empty() ||
            previous_current_lang == previous_default_lang;

        m_base_dir = std::move(base_dir);
        m_default_lang = std::move(default_lang);
        m_current_lang = was_using_default ? m_default_lang : previous_current_lang;

        // Keep the pointer valid while the maps and their string-view key pool are rebuilt.
        m_current_map = &m_en_map;
        m_en_map.clear();
        m_lang_cache.clear();
        m_label_cache.clear();
        m_md_cache.clear();
        m_key_pool.clear();
        m_plural_rules = std::make_unique<PluralRules>();

        try {
            m_en_map = load_language_map(m_default_lang);
            if (m_current_lang == m_default_lang) {
                m_current_map = &m_en_map;
            } else {
                auto map = load_language_map(m_current_lang);
                auto it = m_lang_cache.emplace(m_current_lang, std::move(map)).first;
                m_current_map = &it->second;
            }
            try_load_plural_rules_from_default_location();
        } catch (...) {
            // Leave a valid empty fallback state even when filesystem/allocation work fails.
            m_current_map = &m_en_map;
            m_en_map.clear();
            m_lang_cache.clear();
            m_label_cache.clear();
            m_md_cache.clear();
            m_key_pool.clear();
            m_current_lang = m_default_lang;
            throw;
        }
    }

    void LangStore::set_language(std::string lang) {
        if (lang == m_current_lang) return;
        m_current_lang = std::move(lang);

        if (m_current_lang == m_default_lang) {
            m_current_map = &m_en_map;
        } else {
            // build or reuse from cache
            auto it = m_lang_cache.find(m_current_lang);
            if (it == m_lang_cache.end()) {
                auto map = load_language_map(m_current_lang);
                it = m_lang_cache.emplace(m_current_lang, std::move(map)).first;
            }
            m_current_map = &it->second;
        }

        // Reset per-language caches
        m_label_cache.clear();
        m_md_cache.clear();
    }

    bool LangStore::load_plural_rules_from_file(const std::string& path) {
        return m_plural_rules ? m_plural_rules->load_from_file(path) : false;
    }

    const std::string& LangStore::text(std::string_view key) const {
        if (const auto* s = find_in(*m_current_map, key)) return *s;
        if (const auto* s = find_in(m_en_map, key))       return *s;
        return missing_string();
    }

    const char* LangStore::label(std::string_view key) const {
        if (auto it = m_label_cache.find(key); it != m_label_cache.end())
            return it->second.c_str();

        const std::string& base = text(key);
        std::string combined;
        combined.reserve(base.size() + 2 + key.size());
        combined += base;
        combined += u8"##";
        combined.append(key.begin(), key.end());

        auto [ins, _] = m_label_cache.emplace(intern_key(std::string(key)), std::move(combined));
        return ins->second.c_str();
    }

    std::string LangStore::doc(std::string_view doc_key) const {
        if (auto s = load_md_cached(m_current_lang, doc_key); !s.empty()) return s;
        if (m_current_lang != m_default_lang) {
            if (auto s = load_md_cached(m_default_lang, doc_key); !s.empty()) return s;
        }
        return {};
    }

    std::string LangStore::plural_suffix(long long n) const {
        // PluralRules::category() already has built-ins for "en" and "ru".
        return m_plural_rules ? m_plural_rules->category(m_current_lang, n)
                              : default_plural_suffix(m_current_lang, n);
    }

    const std::string& LangStore::text_plural(std::string_view base_key, long long n) const {
        const std::string suf = plural_suffix(n);
        if (const auto* s = find_in(*m_current_map, join_dotted(base_key, suf))) return *s;
        if (const auto* s = find_in(m_en_map,      join_dotted(base_key, suf)))   return *s;
        if (const auto* s = find_in(*m_current_map, base_key)) return *s;
        if (const auto* s = find_in(m_en_map,       base_key)) return *s;
        return missing_string();
    }

    std::string LangStore::default_i18n_base_dir() {
#if IMGUIX_RESOLVE_PATHS_REL_TO_EXE
        return ImGuiX::Utils::resolveExecPath(IMGUIX_I18N_DIR);
#else
        return std::string(IMGUIX_I18N_DIR);
#endif
    }

    LangStore::StrMap LangStore::load_language_map(const std::string& lang) const {
        StrMap out;
        const fs::path dir = fs::path(m_base_dir) / lang;
        std::error_code ec;
        if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
            return out; // allowed to be empty
        }

        for (auto& de : fs::directory_iterator(dir, ec)) {
            if (ec) break;
            if (!de.is_regular_file()) continue;
            const auto& p = de.path();
            if (p.extension() != u8".json") continue;

            const std::string raw = read_file(p.string());
            if (raw.empty()) continue;

            const std::string clean = ImGuiX::Utils::strip_json_comments(raw, /*with_whitespace*/ false);

            nlohmann::json j;
            try {
                j = nlohmann::json::parse(clean);
            } catch (...) {
                // ignore broken files (could log)
                continue;
            }

            merge_object_for_lang(j, lang, out, *this);
        }
        return out;
    }

    void LangStore::merge_object_for_lang(
            const nlohmann::json& obj,
            const std::string& lang,
            StrMap& out,
            const LangStore& self
        ) {
        if (!obj.is_object()) return;

        for (auto it = obj.begin(); it != obj.end(); ++it) {
            const std::string& key_str = it.key();   // JSON provides std::string only once
            KeyView k = self.intern_key(key_str);    // intern -> KeyView stays stable
            const nlohmann::json& val = it.value();

            auto get_sv = [&](const nlohmann::json& j) -> std::string {
                if (j.is_string()) return j.get<std::string>();
                if (j.is_array()) {
                    std::string s;
                    for (const auto& e : j) if (e.is_string()) s += e.get<std::string>();
                    return s;
                }
                return {};
            };

            if (val.is_object()) {
                if (auto jt = val.find(lang); jt != val.end()) {
                    auto s = get_sv(*jt);
                    if (!s.empty()) { out.emplace(k, std::move(s)); continue; }
                }
                if (auto jt = val.find(u8"en"); jt != val.end()) {
                    auto s = get_sv(*jt);
                    if (!s.empty()) { out.emplace(k, std::move(s)); continue; }
                }
            } else {
                auto s = get_sv(val);
                if (!s.empty()) { out.emplace(k, std::move(s)); }
            }
        }
    }

    std::string LangStore::get_string_or_join_array(const nlohmann::json& j) {
        if (j.is_string()) return j.get<std::string>();
        if (j.is_array()) {
            std::string s;
            for (const auto& e : j) {
                if (e.is_string()) s += e.get<std::string>();
            }
            return s;
        }
        return {};
    }

    std::string LangStore::read_file(const std::string& path) {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs) return {};
        std::ostringstream oss; oss << ifs.rdbuf();
        return oss.str();
    }

    std::string LangStore::load_md_cached(const std::string& lang, std::string_view doc_key) const {
        auto& by_key = m_md_cache[lang]; // creates empty sub-table on first access
        if (auto it = by_key.find(doc_key); it != by_key.end())
            return it->second;

        const fs::path p = fs::path(m_base_dir) / lang / (std::string(doc_key) + u8".md");
        auto s = read_file(p.string());
        if (!s.empty()) {
            by_key.emplace(intern_key(std::string(doc_key)), s);
        }
        return s;
    }

    std::string LangStore::default_plural_suffix(const std::string& lang, long long n) {
        // Minimal fallback; PluralRules has richer built-ins.
        if (lang == u8"ru") {
            long long n10 = n % 10;
            long long n100 = n % 100;
            if (n10 == 1 && n100 != 11) return u8"one";
            if (n10 >= 2 && n10 <= 4 && !(n100 >= 12 && n100 <= 14)) return u8"few";
            if (n10 == 0 || (n10 >= 5 && n10 <= 9) || (n100 >= 11 && n100 <= 14)) return u8"many";
            return u8"other";
        }
        return (n == 1) ? u8"one" : u8"other";
    }

    void LangStore::try_load_plural_rules_from_default_location() {
        std::error_code ec;
        const fs::path path = fs::u8path(m_base_dir) / IMGUIX_I18N_PLURALS_FILENAME;
        if (fs::exists(path, ec) && m_plural_rules) {
            (void)m_plural_rules->load_from_file(path.u8string()); // ignore failure
        }
    }

} // namespace ImGuiX::I18N
