#pragma once
#ifndef _IMGUIX_UTILS_I18N_LANG_STORE_HPP_INCLUDED
#define _IMGUIX_UTILS_I18N_LANG_STORE_HPP_INCLUDED

#include <imguix/config/i18n.hpp>

/// \file LangStore.hpp
/// \brief Localized strings and markdown loader with JSON (monolingual + polyglot) support,
///        using pluggable plural rules (PluralRules).
///
/// Features:
/// - JSON loader supports:
///    1) monolingual:  { "Key": "Text", ... }
///    2) polyglot:     { "Key": { "en":"...", "ru":"...", ... }, ... }
///   JSON is preprocessed with strip_json_comments(...) before parsing.
/// - Markdown: *.md files are monolingual (per-language folder), lazy-loaded with cache.
/// - Fallback: current_lang -> default_lang ("en").
/// - Formatting: fmt-based textf_*() helpers.
/// - ImGui ID: cached "label##id" via label(key) (lazy init per key+lang).
/// - Pluralization: via PluralRules (JSON-driven, with built-ins in the rules).
///
/// Layout:
///   <base_dir>/<lang>/*.json   (short strings)
///   <base_dir>/<lang>/*.md     (long texts)
///   <base_dir>/plurals.json    (optional; plural rules)
///
/// Notes:
/// - Not thread-safe by design; guard externally if needed.
/// - All caches are invalidated on language switch.

namespace ImGuiX::I18N {

    namespace fs = std::filesystem;

    /// \class LangStore
    /// \brief Localized string and Markdown loader with plural support.
    class LangStore {
    public:

        /// \brief Construct store with default directory and English fallback.
        LangStore()
            : LangStore(default_i18n_base_dir(), u8"en") {
        }

        /// \brief Construct i18n store.
        /// \param base_dir Root folder with per-language subfolders (e.g., "<root>/en/", "<root>/ru/").
        /// \param default_lang Default language (fallback), typically "en".
        explicit LangStore(std::string base_dir, std::string default_lang = u8"en");

        /// \brief Replace the resource base directory and reload all language data.
        /// \param base_dir Root folder containing per-language subdirectories.
        /// \param default_lang Fallback language identifier.
        void set_base_dir(std::string base_dir, std::string default_lang = u8"en");

        /// \brief Change current language and rebuild caches.
        /// \param lang New language code (e.g., "ru", "en").
        void set_language(std::string lang);

        /// \brief Current language code (e.g., "ru", "en").
        /// \return Active language code.
        const std::string& language() const noexcept { return m_current_lang; }

        /// \brief Set external plural rules implementation.
        /// \param rules New rules instance; takes ownership.
        void set_plural_rules(std::unique_ptr<PluralRules> rules) {
            if (rules) m_plural_rules = std::move(rules);
        }

        /// \brief Load plural rules from a JSON file.
        /// \param path Path to JSON file.
        /// \return True on success.
        bool load_plural_rules_from_file(const std::string& path);

        /// \brief Plain localized text for a key with fallback to default language.
        /// \param key Lookup key.
        /// \return Reference to internal storage; invalidated on language switch or map reload.
        const std::string& text(std::string_view key) const;

        /// \brief Format using a compile-time checked string.
        /// \tparam Args Format arguments.
        /// \param fmt_str Format string.
        /// \param args Format arguments.
        /// \return Formatted string.
        template <typename... Args>
        std::string textf(fmt::format_string<Args...> fmt_str, Args&&... args) const {
            return fmt::format(fmt_str, std::forward<Args>(args)...);
        }

        /// \brief Format by key where template comes from JSON.
        /// \tparam Args Format arguments.
        /// \param key Lookup key.
        /// \param args Format arguments.
        /// \return Formatted string.
        template <typename... Args>
        std::string textf_key(std::string_view key, Args&&... args) const {
            return fmt::format(text(key), std::forward<Args>(args)...);
        }

        /// \brief vformat by key using pre-built \c fmt::format_args.
        /// \param key Lookup key.
        /// \param args Pre-built arguments.
        /// \return Formatted string.
        inline std::string textvf_key(std::string_view key, fmt::format_args args) const {
            return fmt::vformat(text(key), args);
        }

        /// \brief Get "label##id" cached string for ImGui.
        /// \param key Lookup key.
        /// \return Cached label string per key and language.
        const char* label(std::string_view key) const;

        /// \brief Load markdown content for a document key.
        /// \param doc_key Document key.
        /// \return Markdown content or empty string if missing.
        /// \note Resolves: <base>/<lang>/<key>.md then <base>/<default>/<key>.md.
        std::string doc(std::string_view doc_key) const;

        /// \brief Clear all runtime caches (labels and markdown).
        void clear_caches() {
            m_label_cache.clear();
            m_md_cache.clear();
        }

        // ------------------ Pluralization API ------------------

        /// \brief Category suffix for \p n via PluralRules.
        /// \param n Numeric value.
        /// \return "one", "few", "many", "other", etc., depending on the active language.
        std::string plural_suffix(long long n) const;

        /// \brief Get pluralized text for base key and number.
        /// \param base_key Base lookup key.
        /// \param n Numeric value.
        /// \return Matching localized string.
        /// \note Fallback order: "<key>.<suffix>" (current), "<key>.<suffix>" (default),
        ///       "<key>" (current), "<key>" (default).
        const std::string& text_plural(std::string_view base_key, long long n) const;

        /// \brief Format pluralized text for base key and number.
        /// \tparam Args Format arguments.
        /// \param base_key Base lookup key.
        /// \param n Numeric value.
        /// \param args Format arguments.
        /// \return Formatted string.
        template <typename... Args>
        std::string textf_plural(std::string_view base_key, long long n, Args&&... args) const {
            return fmt::format(text_plural(base_key, n), std::forward<Args>(args)...);
        }

    private:

        // --- key types and maps ---
        using KeyView = std::string_view;

        struct SvHash {
            size_t operator()(KeyView s) const noexcept { return std::hash<KeyView>{}(s); }
        };

        struct SvEq {
            bool operator()(KeyView a, KeyView b) const noexcept { return a == b; }
        };

        using StrMap = std::unordered_map<KeyView, std::string, SvHash, SvEq>;

        mutable std::deque<std::string> m_key_pool; ///< Key pool (owns storage so KeyView stays stable)

        // Intern a key to guarantee stable storage
        KeyView intern_key(const std::string& s) const {
            m_key_pool.emplace_back(s);
            return KeyView{m_key_pool.back()};
        }

        KeyView intern_key(std::string&& s) const {
            m_key_pool.emplace_back(std::move(s));
            return KeyView{m_key_pool.back()};
        }

        static std::string default_i18n_base_dir();

        // ---------- JSON loading ----------

        StrMap load_language_map(const std::string& lang) const;

        static void merge_object_for_lang(
                const nlohmann::json& obj,
                const std::string& lang,
                StrMap& out,
                const LangStore& self
            );

        static std::string get_string_or_join_array(const nlohmann::json& j);

        static std::string read_file(const std::string& path);

        // ---------- Markdown loading (lazy) ----------

        std::string load_md_cached(const std::string& lang, std::string_view doc_key) const;

        // ---------- Helpers ----------

        static const std::string* find_in(const StrMap& m, std::string_view key) {
            auto it = m.find(key);
            return (it == m.end()) ? nullptr : &it->second;
        }

        static const std::string& missing_string() {
            static const std::string k = u8"##null";
            return k;
        }

        static std::string join_dotted(std::string_view base, std::string_view suffix) {
            std::string s;
            s.reserve(base.size() + 1 + suffix.size());
            s.append(base.begin(), base.end());
            s.push_back('.');
            s.append(suffix.begin(), suffix.end());
            return s;
        }

        static std::string default_plural_suffix(const std::string& lang, long long n);

        void try_load_plural_rules_from_default_location();

        // config
        std::string m_base_dir;
        std::string m_default_lang;
        std::string m_current_lang;

        // string maps
        StrMap      m_en_map;
        StrMap*     m_current_map{ &m_en_map };

        // caches
        mutable std::unordered_map<std::string, StrMap>        m_lang_cache;   // lang -> map
        //mutable std::unordered_map<std::string, std::string>   m_md_cache;     // (lang+'\n'+key) -> content
        mutable std::unordered_map<KeyView, std::string, SvHash, SvEq> m_label_cache; // key -> "text##key"
        mutable std::unordered_map<std::string, std::unordered_map<KeyView, std::string, SvHash, SvEq>> m_md_cache; // nested map: lang -> (doc_key -> markdown content)
        // plural rules
        std::unique_ptr<PluralRules> m_plural_rules;
    };

    // -------------------- Optional: tiny plural facade --------------------
    /// \brief Convenience wrappers for pluralized lookups.
    struct Plural {
        /// \brief Get plural category suffix for number.
        /// \param ls Language store.
        /// \param n Numeric value.
        /// \return Category suffix such as "one" or "other".
        static std::string suffix(const LangStore& ls, long long n) {
            return ls.plural_suffix(n);
        }

        /// \brief Get pluralized text for base key and number.
        /// \param ls Language store.
        /// \param base_key Base lookup key.
        /// \param n Numeric value.
        /// \return Localized string.
        static const std::string& text(const LangStore& ls, std::string_view base_key, long long n) {
            return ls.text_plural(base_key, n);
        }

        /// \brief Format pluralized text for base key and number.
        /// \tparam Args Format arguments.
        /// \param ls Language store.
        /// \param base_key Base lookup key.
        /// \param n Numeric value.
        /// \param args Format arguments.
        /// \return Formatted string.
        template <typename... Args>
        static std::string textf(const LangStore& ls, std::string_view base_key, long long n, Args&&... args) {
            return ls.textf_plural(base_key, n, std::forward<Args>(args)...);
        }
    };

} // namespace ImGuiX::I18N

#ifdef IMGUIX_HEADER_ONLY
#   include "LangStore.ipp"
#endif

#endif // _IMGUIX_UTILS_I18N_LANG_STORE_HPP_INCLUDED
