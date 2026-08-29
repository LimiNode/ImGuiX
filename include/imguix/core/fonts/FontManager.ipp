#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>
#include <sstream>
#include <utility>
#include <iostream>

#ifdef IMGUIX_FONTS_ENABLE_JSON
#include <nlohmann/json.hpp>
#endif

#ifdef IMGUI_ENABLE_FREETYPE
#include <imgui_freetype.h>
#endif

#include <imguix/utils/path_utils.hpp>
#include <imguix/config/fonts.hpp>

#if defined(IMGUIX_USE_SFML_BACKEND)
#   include <imgui-SFML.h>
#elif defined(IMGUIX_USE_GLFW_BACKEND) || defined(IMGUIX_USE_SDL2_BACKEND)
#   include <imgui_impl_opengl3.h>
#endif

#ifndef IMGUIX_SUPPRESS_EMOJI_CONTROL_GLYPHS
#define IMGUIX_SUPPRESS_EMOJI_CONTROL_GLYPHS 1
#endif

#ifndef IMGUIX_SUPPRESS_KEYCAP_COMBINING
#define IMGUIX_SUPPRESS_KEYCAP_COMBINING 0   // 1, если хотите скрыть 'keycap' U+20E3
#endif

namespace ImGuiX::Fonts {
    namespace fs = std::filesystem;

    /// ----------------------------
    /// Inline trivial setters/getters
    /// ----------------------------

    inline void FontManager::setConfigPath(std::string path) {
      m_config_path = std::move(path);
    }

    inline void FontManager::setBaseDir(std::string base_dir) {
      m_params.base_dir = std::move(base_dir);
      markDirty();
    }

    inline void FontManager::setDpi(float dpi) {
        const float clamped_dpi = std::max(1.0F, dpi);
        if (m_params.dpi == clamped_dpi) return;
        m_params.dpi = clamped_dpi;
        markDirty();
    }

    inline void FontManager::setLocale(std::string locale) {
        if (locale == m_active_locale) return;
        m_active_locale = std::move(locale);
        markDirty();
    }
    
    inline void FontManager::setRanges(std::string preset) {
        m_manual.ranges_preset = std::move(preset);
        m_manual.ranges.clear();
        markDirty();
    }

    inline void FontManager::setRanges(const std::vector<ImWchar>& pairs) {
        m_manual.ranges_preset.clear();
        m_manual.ranges = pairs;
        markDirty();
    }

    inline void FontManager::clearRanges() {
        m_manual.ranges_preset.clear();
        m_manual.ranges.clear();
        markDirty();
    }

    inline void FontManager::setMarkdownSizes(
            float body_px, 
            float h1_px,
            float h2_px, 
            float h3_px
        ) {
        m_px_body = body_px;
        m_px_h1 = h1_px;
        m_px_h2 = h2_px;
        m_px_h3 = h3_px;
        markDirty();
    }

    inline void FontManager::setLocalePack(const LocalePack &pack) {
      m_packs[pack.locale] = pack;
      // Not marking dirty automatically; active locale may be different.
    }

    inline void FontManager::clearPacks() {
      m_packs.clear();
      markDirty();
    }

    inline void FontManager::beginManual() {
        m_manual = {};
        m_manual.active = true;
        markDirty();
    }

    inline void FontManager::addFontBody(const FontFile &ff) {
      m_manual.has_body = true;
      m_manual.body = ff;
    }

    inline void FontManager::addFontHeadline(FontRole role_h, const FontFile &ff) {
      m_manual.headlines[role_h] = ff;
    }

    inline void FontManager::addFontMerge(FontRole role, const FontFile &ff) {
      if (!m_manual.active) {
        m_manual.active = true;
      } // allow call without beginManual()
      if (role == FontRole::Icons) {
        m_manual.merges_icons.push_back(ff);
      } else if (role == FontRole::Emoji) {
        m_manual.merges_emoji.push_back(ff);
      } else {
        // ignore invalid roles to keep invariants
        return;
      }
      markDirty();
    }

    inline void FontManager::addFontMerge(const FontFile &ff) {
      if (!m_manual.active) {
        m_manual.active = true;
      }
      m_manual.merges_unknown.push_back(ff);
      markDirty();
    }

    inline void FontManager::markDirty() { m_dirty = true; }

    inline ImFont *FontManager::getFont(FontRole role) const {
      auto it = m_fonts.find(role);
      return (it != m_fonts.end()) ? it->second : nullptr;
    }

    inline const std::string &FontManager::activeLocale() const {
      return m_active_locale;
    }
    inline const BuildParams &FontManager::params() const { return m_params; }

    /// ----------------------------
    /// Heavy helpers (implementation)
    /// ----------------------------

    /// \brief Compute effective pixel size (px @ runtime) given base size at 96
    /// DPI.
    inline float FontManager::scalePx(float px_96, const BuildParams &p) {
      return px_96 * (p.dpi / 96.0f) * p.ui_scale;
    }

    /// \brief Add standard ranges based on locale id.
    /// \note Keep conservative defaults to avoid huge atlases.
    inline void FontManager::addLocaleRanges(
            ImFontGlyphRangesBuilder &b,
            ImGuiIO &io,
            const std::string &locale
        ) {
        // Always include default UI symbols
        b.AddRanges(io.Fonts->GetGlyphRangesDefault());

        // Common add-ons: punctuation seen in many UIs
        b.AddText(u8"–—…•“”‘’"); // en/em dashes, ellipsis, bullets, quotes

        // Latin-based locales already covered by Default()
        // Add Cyrillic
        if (locale == u8"ru" || 
            locale == u8"uk" ||
            locale == u8"bg" || 
            locale == u8"kk" ||
            locale == u8"sr") {
            b.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
        }

        // Vietnamese
        if (locale == u8"vi") b.AddRanges(io.Fonts->GetGlyphRangesVietnamese());

        // CJK (careful: large)
        if (locale == u8"ja")
            b.AddRanges(io.Fonts->GetGlyphRangesJapanese());
        else if (locale == u8"zh" || locale == u8"zh-CN" || locale == u8"zh-TW")
            b.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
        else if (locale == u8"ko")
            b.AddRanges(io.Fonts->GetGlyphRangesKorean());

        // RTL (Arabic/Hebrew) – ImGui does not provide built-ins for all subsets;
        // user usually supplies proper ranges via JSON LocalePack::ranges or custom
        // font files.
        if (locale == u8"ar" || locale == u8"fa" || locale == u8"he") {
            // Leave to JSON-provided ranges; if absent, fallback stays Latin-only.
            // You may add custom ranges here if you ship appropriate fonts.
        }
    }

    inline void FontManager::addNamedRanges(
            ImFontGlyphRangesBuilder& b,
            ImGuiIO& io, 
            const std::string& spec
        ) {
        auto split = [](const std::string& s) {
            auto trim = [](const std::string& token) {
                size_t begin = 0;
                while (begin < token.size() && std::isspace(static_cast<unsigned char>(token[begin])) != 0) {
                    ++begin;
                }

                size_t end = token.size();
                while (end > begin && std::isspace(static_cast<unsigned char>(token[end - 1])) != 0) {
                    --end;
                }

                return token.substr(begin, end - begin);
            };

            std::vector<std::string> out;
            std::string cur;
            auto push = [&](const std::string& token) {
                const std::string trimmed = trim(token);
                if (!trimmed.empty()) {
                    out.push_back(trimmed);
                }
            };
            for (char c : s) {
                if (c == '+') {
                    push(cur);
                    cur.clear();
                } else cur.push_back(c);
            }
            push(cur);
            return out;
        };
        for (auto tok : split(spec)) {
            if (tok == u8"Default")          b.AddRanges(io.Fonts->GetGlyphRangesDefault());
            else if (tok == u8"Cyrillic")    b.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
            else if (tok == u8"Vietnamese")  b.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
            else if (tok == u8"Japanese" || tok == u8"JapaneseFull") b.AddRanges(io.Fonts->GetGlyphRangesJapanese());
            else if (tok == u8"Chinese"  || tok == u8"ChineseFull")  b.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
            else if (tok == u8"Korean")      b.AddRanges(io.Fonts->GetGlyphRangesKorean());
            else if (tok == u8"Punct")       b.AddText(u8"–—…•“”‘’");
            else if (tok == u8"PUA" || tok == u8"Icons" || tok == u8"PrivateUse") {
                // Private Use Area (BMP
                static const ImWchar kPUA[] = { 0xE000, 0xF8FF, 0 };
                b.AddRanges(kPUA);
            } else if (tok == u8"MiscSymbols" || tok == u8"Misc") {
                // ⚠, ☀, ☂, ☺, 
                static const ImWchar kMiscSymbols[] = { 0x2600, 0x26FF, 0 };
                b.AddRanges(kMiscSymbols);
            } else if (tok == u8"Dingbats") {
                // ✂, ✈, ✔, ✖, …
                static const ImWchar kDingbats[] = { 0x2700, 0x27BF, 0 };
                b.AddRanges(kDingbats);
            } else if (tok == u8"MiscTechnical" || tok == u8"MiscTech") {
                // ←, ↑, →, ↔, …
                static const ImWchar kArrows[] = { 0x2300, 0x23FF, 0 };
                b.AddRanges(kArrows);
            } else if (tok == u8"Arrows") {
                // ←, ↑, →, ↔, …
                static const ImWchar kArrows[] = { 0x2190, 0x21FF, 0 };
                b.AddRanges(kArrows);
            } else if (tok == u8"LatinExtA") {      // U+0100..U+017F   (œ/Œ здесь)
                static const ImWchar kLatinExtA[] = { 0x0100, 0x017F, 0 };
                b.AddRanges(kLatinExtA);
            } else if (tok == u8"LatinExtB") {        // U+0180..U+024F   (реже нужно)
                static const ImWchar kLatinExtB[] = { 0x0180, 0x024F, 0 };
                b.AddRanges(kLatinExtB);
            } else if (tok == u8"Latin1Sup") {      // U+0080..U+00FF (многие акценты здесь)
                static const ImWchar kLatin1[] = { 0x0080, 0x00FF, 0 };
                b.AddRanges(kLatin1);
            } else if (tok == u8"LatinExtAdditional") { // U+1E00..U+1EFF (редко, но встречается)
                static const ImWchar kLatinExtAdd[] = { 0x1E00, 0x1EFF, 0 };
                b.AddRanges(kLatinExtAdd);
            } else if (tok == u8"EmojiBasic") {           // 🙂😀 …
                static_assert(sizeof(ImWchar) == 4, "IMGUI_USE_WCHAR32 is not active here");
                static const ImWchar kEmojiBasic[] = { 0x1F600, 0x1F64F, 0 };
                b.AddRanges(kEmojiBasic);
            } else if (tok == u8"EmojiSupp") {          // 🥲🤖 …
                static_assert(sizeof(ImWchar) == 4, "IMGUI_USE_WCHAR32 is not active here");
                static const ImWchar kEmojiSupp[]  = { 0x1F900, 0x1F9FF, 0 };
                b.AddRanges(kEmojiSupp);
            } else if (tok == u8"EmojiPict") {          // 🎨📦 …
                static_assert(sizeof(ImWchar) == 4, "IMGUI_USE_WCHAR32 is not active here");
                static const ImWchar kPict[]       = { 0x1F300, 0x1F5FF, 0 };
                b.AddRanges(kPict);
            } else if (tok == u8"EmojiTransport") {     // 🚀🛳 …
                static_assert(sizeof(ImWchar) == 4, "IMGUI_USE_WCHAR32 is not active here");
                static const ImWchar kTrans[]      = { 0x1F680, 0x1F6FF, 0 };
                b.AddRanges(kTrans);
            } else if (tok == u8"EmojiExtA") {          // 🪓🪙 …
                static_assert(sizeof(ImWchar) == 4, "IMGUI_USE_WCHAR32 is not active here");
                static const ImWchar kExtA[]       = { 0x1FA70, 0x1FAFF, 0 };
                b.AddRanges(kExtA);
            } else if (tok == u8"EmojiAll") {
                static_assert(sizeof(ImWchar) == 4, "IMGUI_USE_WCHAR32 is not active here");

                // BMP helpers
                static const ImWchar kMiscTech[]   = { 0x2300, 0x23FF, 0 };
                static const ImWchar kMiscSym[]    = { 0x2600, 0x26FF, 0 };
                static const ImWchar kDingbats[]   = { 0x2700, 0x27BF, 0 };
                // Core emoji planes (SMP)
                static const ImWchar kPict[]       = { 0x1F300, 0x1F5FF, 0 };
                static const ImWchar kEmoticons[]  = { 0x1F600, 0x1F64F, 0 };
                static const ImWchar kTransport[]  = { 0x1F680, 0x1F6FF, 0 };
                static const ImWchar kSuppSym[]    = { 0x1F900, 0x1F9FF, 0 };
                static const ImWchar kExtA[]       = { 0x1FA70, 0x1FAFF, 0 };
                static const ImWchar kRegional[]   = { 0x1F1E6, 0x1F1FF, 0 };
                static const ImWchar kSkinTones[]  = { 0x1F3FB, 0x1F3FF, 0 };

                b.AddRanges(kMiscTech);
                b.AddRanges(kMiscSym);
                b.AddRanges(kDingbats);
                b.AddRanges(kPict);
                b.AddRanges(kEmoticons);
                b.AddRanges(kTransport);
                b.AddRanges(kSuppSym);
                b.AddRanges(kExtA);
                b.AddRanges(kRegional);
                b.AddRanges(kSkinTones);

                // Composition helpers
                //b.AddChar(0x0023); // '#'
                //b.AddChar(0x002A); // '*'

                for (ImWchar cp = 0x0030; cp <= 0x0039; ++cp) b.AddChar(cp);
            } else if (tok == u8"EmojiTGCore") {
                static_assert(sizeof(ImWchar) == 4, "IMGUI_USE_WCHAR32 is not active here");

                // 1) Emoticons
                static const ImWchar kEmoticons[]  = { 0x1F600, 0x1F64F, 0 };
                // 2) Hearts/checks/warnings etc.
                static const ImWchar kDingbats[]   = { 0x2700, 0x27BF, 0 };
                static const ImWchar kMiscSym[]    = { 0x2600, 0x26FF, 0 };
                // 3) Timers/clocks
                static const ImWchar kMiscTech[]   = { 0x2300, 0x23FF, 0 };

                b.AddRanges(kEmoticons);
                b.AddRanges(kDingbats);
                b.AddRanges(kMiscSym);
                b.AddRanges(kMiscTech);

                // 4) Hand-picked frequent pictographs
                b.AddChar(0x1F389); // 🎉
                b.AddChar(0x1F38A); // 🎊
                b.AddChar(0x1F525); // 🔥
                b.AddChar(0x1F680); // 🚀
                b.AddChar(0x1F4AF); // 💯
                b.AddChar(0x1F517); // 🔗
                b.AddChar(0x1F4CE); // 📎

                // 5) Presentation helpers
                b.AddChar(0x2764);  // ❤ (heart)
                b.AddChar(0x2757);  // ❗
            } else if (tryAddNumericPresetToken(b, tok)) {
                continue;
            }
            // add more named ranges as needed (Latin, Greek, Thai, etc.)
        }
    }

    inline bool FontManager::tryParseNumericCodepointToken(
            const std::string& token,
            ImWchar& out
        ) {
        auto trim = [](const std::string& s) {
            size_t begin = 0;
            while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin])) != 0) {
                ++begin;
            }

            size_t end = s.size();
            while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1])) != 0) {
                --end;
            }

            return s.substr(begin, end - begin);
        };

        const std::string trimmed = trim(token);
        if (trimmed.empty()) {
            return false;
        }

        int base = 10;
        std::string digits = trimmed;
        if (digits.size() > 2 && digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X')) {
            base = 16;
            digits = digits.substr(2);
            if (digits.empty() || !std::all_of(digits.begin(), digits.end(), [](unsigned char c) {
                return std::isxdigit(c) != 0;
            })) {
                return false;
            }
        } else if (!std::all_of(digits.begin(), digits.end(), [](unsigned char c) {
            return std::isdigit(c) != 0;
        })) {
            return false;
        }

        try {
            size_t pos = 0;
            const unsigned long long value = std::stoull(digits, &pos, base);
            if (pos != digits.size() || value > static_cast<unsigned long long>(std::numeric_limits<ImWchar>::max())) {
                return false;
            }

            out = static_cast<ImWchar>(value);
            return true;
        } catch (...) {
            return false;
        }
    }

    inline bool FontManager::tryAddNumericPresetToken(
            ImFontGlyphRangesBuilder& b,
            const std::string& token
        ) {
        const size_t dash_pos = token.find('-');
        if (dash_pos == std::string::npos) {
            ImWchar cp = 0;
            if (!tryParseNumericCodepointToken(token, cp)) {
                return false;
            }

            b.AddChar(cp);
            return true;
        }

        if (token.find('-', dash_pos + 1) != std::string::npos) {
            return false;
        }

        ImWchar start = 0;
        ImWchar end = 0;
        if (!tryParseNumericCodepointToken(token.substr(0, dash_pos), start) ||
            !tryParseNumericCodepointToken(token.substr(dash_pos + 1), end) ||
            start > end) {
            return false;
        }

        const ImWchar range[] = { start, end, 0 };
        b.AddRanges(range);
        return true;
    }

    /// \brief Build glyph ranges from LocalePack + extra glyphs from all FontFiles.
    /// \details Combines named presets (ranges_preset), explicit pairs (ranges),
    ///          locale-based defaults, and per-file extra_glyphs (UTF-8).
    inline void FontManager::buildRangesFromPack(std::vector<ImWchar> &out,
                                                 const LocalePack *pack,
                                                 const std::string &active_locale) {
      ImGuiIO &io = ImGui::GetIO();
      ImFontGlyphRangesBuilder b;

      if (pack) {
        // 1) Preset string (named + numeric tokens).
        if (!pack->ranges_preset.empty())
          addNamedRanges(b, io, pack->ranges_preset);

        // 2) Explicit [start,end] pairs (0-terminated). Ensure terminator.
        if (!pack->ranges.empty()) {
          if (pack->ranges.back() == 0) {
            b.AddRanges(pack->ranges.data());
          } else {
            std::vector<ImWchar> tmp = pack->ranges;
            tmp.push_back(0);
            b.AddRanges(tmp.data());
          }
        }

        // 3) If neither presets nor explicit ranges given, fall back to locale
        // defaults.
        if (pack->ranges_preset.empty() && pack->ranges.empty())
          addLocaleRanges(b, io, active_locale);

        // 4) Common punctuation (harmless to add twice; builder deduplicates).
        b.AddText(u8"–—…•“”‘’");

        // 5) Per-file extra glyphs (UTF-8) — union across all roles/files.
        for (const auto &kv : pack->roles)
          for (const auto &ff : kv.second)
            if (!ff.extra_glyphs.empty())
              addExtraGlyphs(b, ff.extra_glyphs);
      } else {
        // No pack: locale-based defaults + punctuation.
        addLocaleRanges(b, io, active_locale);
        b.AddText(u8"–—…•“”‘’");
      }

      ImVector<ImWchar> built;
      b.BuildRanges(&built);
      out.assign(built.begin(), built.end());
    }

    /// \brief Add a font file to ImGui atlas (respecting merge flag, size scaling
    /// and ranges).
    inline ImFont *FontManager::addFontFile(
            const FontFile &ff,
            const BuildParams &params,
            const std::vector<ImWchar> &ranges,
            const fs::path &base_dir_abs,
            std::vector<std::vector<unsigned char>> &owned_font_data,
            const ImFontConfig &base_cfg
        ) {
        ImFontConfig cfg = base_cfg;
        cfg.MergeMode = cfg.MergeMode || ff.merge;
        cfg.GlyphOffset.y += ff.baseline_offset_px;
        cfg.FontDataOwnedByAtlas = false;
        
        const float px = (ff.size_px > 0.0f ? ff.size_px : 16.0f);
        const float eff_px = scalePx(px, params);

        fs::path p = fs::u8path(ff.path);
        const fs::path resolved_p = (p.is_absolute() ? p : (base_dir_abs / p)).lexically_normal();

        std::ifstream input(resolved_p, std::ios::binary);
        if (!input.good()) {
            return nullptr;
        }

        input.seekg(0, std::ios::end);
        const std::streamoff size = input.tellg();
        if (size <= 0) {
            return nullptr;
        }
        input.seekg(0, std::ios::beg);

        owned_font_data.emplace_back(static_cast<size_t>(size));
        std::vector<unsigned char>& font_data = owned_font_data.back();
        input.read(reinterpret_cast<char*>(font_data.data()), static_cast<std::streamsize>(size));
        if (!input.good() && !input.eof()) {
            owned_font_data.pop_back();
            return nullptr;
        }

        return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
            font_data.data(),
            static_cast<int>(font_data.size()),
            eff_px,
            &cfg,
            ranges.empty() ? nullptr : ranges.data());
    }

    /// \brief Switch FreeType builder for ImGui if requested and available.
    inline void FontManager::setupFreetypeIfNeeded(const BuildParams &params) {
#   ifdef IMGUI_ENABLE_FREETYPE
        if (!params.use_freetype) return;
#       if IMGUI_VERSION_NUM >= 19200
        ImGui::GetIO().Fonts->FontLoader = ImGuiFreeType::GetFontLoader();
        ImGui::GetIO().Fonts->FontLoaderFlags = 0;
#       else
        ImGui::GetIO().Fonts->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType();
        ImGui::GetIO().Fonts->FontBuilderFlags = 0;
#       endif
#   else
        (void)params;
#   endif
    }

    /// \brief After Fonts->Build(), update backend texture for active backend.
    inline bool FontManager::updateBackendTexture() {
#       ifdef IMGUIX_USE_SFML_BACKEND
        // ImGui-SFML provides UpdateFontTexture(); some versions return void.
        return ImGui::SFML::UpdateFontTexture();
#       elif defined(IMGUIX_USE_GLFW_BACKEND) || defined(IMGUIX_USE_SDL2_BACKEND)
        ImGui_ImplOpenGL3_DestroyFontsTexture();
        return ImGui_ImplOpenGL3_CreateFontsTexture();
#       else
        return true;
#       endif
    }

    /// ----------------------------
    /// Public methods implementation
    /// ----------------------------

    inline bool FontManager::bootstrap(const BuildParams &params) {
      m_params = params;
      m_dirty = true;
      return true;
    }
    
    
    static void AddInvisibleEmojiControls(ImFontAtlas* atlas, ImFont* font) {
        auto add = [&](ImWchar cp){
            // 1x1 «пустышка», advance=0.0f => символ не виден и не влияет на ширину.
            atlas->AddCustomRectFontGlyph(font, cp, 1, 1, /*advance_x=*/0.0f, ImVec2(0,0));
        };

        // Variation Selectors (VS1..VS16) — FE00..FE0F
        for (ImWchar cp = 0xFE00; cp <= 0xFE0F; ++cp) add(cp);

        // Zero-width & joiners
        add(0x200D); // ZWJ
        add(0x200C); // ZWNJ
        add(0x200B); // Zero Width Space
        add(0x2060); // Word Joiner
        add(0x034F); // Combining Grapheme Joiner

#       if IMGUIX_SUPPRESS_KEYCAP_COMBINING
        add(0x20E3); // Combining Enclosing Keycap (для '1️⃣' и т.п.)
#       endif
    }

    inline BuildResult FontManager::buildNow() {
        BuildResult br{};

        fs::path base_dir_abs;
#       ifdef __EMSCRIPTEN__
        base_dir_abs = fs::u8path(m_params.base_dir);
#       else
        base_dir_abs = ImGuiX::Utils::resolveExecPathFs(fs::u8path(m_params.base_dir));
#       endif

        // We support either manual buffer (if active) or the active locale pack.
        ImGuiIO &io = ImGui::GetIO();
        m_owned_font_data.clear();
        io.Fonts->Clear();
        io.FontDefault = nullptr;

        setupFreetypeIfNeeded(m_params);
        
        // 1) Select pack_ptr for active locale (if not manual)
        const LocalePack* pack_ptr = nullptr;
        if (!m_manual.active) {
            auto it = m_packs.find(m_active_locale);
            if (it == m_packs.end()) {
                auto it2 = m_packs.find(u8"default");
                if (it2 != m_packs.end()) pack_ptr = &it2->second;
            } else {
                pack_ptr = &it->second;
            }
        }
        
        // 2) Collect FreeType flags from all FontFile
        unsigned int ft_flags = 0;
        if (!m_manual.active) {
            const LocalePack* p = pack_ptr;
            if (p) {
                for (const auto& role_vec : p->roles) {
                    for (const auto& ff : role_vec.second) {
                        ft_flags |= ff.freetype_flags;
                    }
                }
            }
        } else {
            if (m_manual.has_body) ft_flags |= m_manual.body.freetype_flags;
            for (const auto& kv : m_manual.headlines) ft_flags |= kv.second.freetype_flags;
            for (const auto& ff : m_manual.merges_icons)   ft_flags |= ff.freetype_flags;
            for (const auto& ff : m_manual.merges_emoji)   ft_flags |= ff.freetype_flags;
            for (const auto& ff : m_manual.merges_unknown) ft_flags |= ff.freetype_flags;
        }

        // 3) Apply flags to atlas before adding any fonts into the atlas.
#   ifdef IMGUI_ENABLE_FREETYPE
#       if IMGUI_VERSION_NUM >= 19200
        ImGui::GetIO().Fonts->FontLoaderFlags = ft_flags;
#       else
        ImGui::GetIO().Fonts->FontBuilderFlags = ft_flags;
#       endif
#   endif

        // Build ranges
        // 4) Gather ranges
        std::vector<ImWchar> ranges;
        if (!m_manual.active) {
            buildRangesFromPack(ranges, pack_ptr, m_active_locale);
        } else {
            // Manual ranges: use default builder with extra glyphs from manual entries
            ImFontGlyphRangesBuilder b;

            // 1) Priority: preset → explicit pairs → locale fallback
            if (!m_manual.ranges_preset.empty()) {
                addNamedRanges(b, io, m_manual.ranges_preset);
            } else 
            if (!m_manual.ranges.empty()) {
                if (m_manual.ranges.back() == 0) {
                    b.AddRanges(m_manual.ranges.data());
                } else {
                    auto tmp = m_manual.ranges;
                    tmp.push_back(0);
                    b.AddRanges(tmp.data());
                }
            } else {
                addLocaleRanges(b, io, m_active_locale);
            }

            // 2) Useful punctuation (deduplicated inside builder)
            b.AddText(u8"–—…•“”‘’");

            // 3) Extra glyphs from all manual config sources
            if (m_manual.has_body) addExtraGlyphs(b, m_manual.body.extra_glyphs);
            for (const auto& kv : m_manual.headlines)      addExtraGlyphs(b, kv.second.extra_glyphs);
            for (const auto& ff : m_manual.merges_icons)   addExtraGlyphs(b, ff.extra_glyphs);
            for (const auto& ff : m_manual.merges_emoji)   addExtraGlyphs(b, ff.extra_glyphs);
            for (const auto& ff : m_manual.merges_unknown) addExtraGlyphs(b, ff.extra_glyphs);

            ImVector<ImWchar> r;
            b.BuildRanges(&r);
            ranges.assign(r.begin(), r.end());
        }

        // Base config for all fonts
        ImFontConfig cfg{};
        cfg.OversampleH = 3;
        cfg.OversampleV = 1;
        cfg.PixelSnapH = false;

        m_fonts.clear();

        // Add Body first (either from pack or from manual)
        ImFont *body = nullptr;

        auto add_role_vec = [&](FontRole role,
                              const std::vector<FontFile> &vec) -> ImFont * {
            ImFont *last = nullptr;
            for (size_t i = 0; i < vec.size(); ++i) {
                ImFontConfig local = cfg;
                // Merge mode depends on FontFile::merge; Body chain root usually
                // merge=false.
                local.MergeMode = (i > 0) ? 
                    (local.MergeMode || vec[i].merge) : 
                    vec[i].merge;
                ImFont *f = addFontFile(vec[i], m_params, ranges, base_dir_abs, m_owned_font_data, local);
                if (!f) {
                    br.message = u8"Failed to load font: " + vec[i].path;
                }
                last = f;
            }
            if (last) m_fonts[role] = last;
            return last;
        };

        auto add_single = [&](FontRole role, const FontFile &ff) -> ImFont * {
            ImFont *f = addFontFile(ff, m_params, ranges, base_dir_abs, m_owned_font_data, cfg);
            if (f) m_fonts[role] = f;
            else br.message = u8"Failed to load font: " + ff.path;
            return f;
        };

        // Strategy per mode
        if (!m_manual.active) {
        // --- PACK MODE ---
        // Body
        if (pack_ptr) {
          if (auto it = pack_ptr->roles.find(FontRole::Body);
              it != pack_ptr->roles.end() && !it->second.empty()) {
            body = add_role_vec(FontRole::Body, it->second);
          }
          // Merge Icons / Emoji into the last added (Body chain)
          if (auto it = pack_ptr->roles.find(FontRole::Icons);
              it != pack_ptr->roles.end()) {
            for (const auto &ff : it->second) {
              FontFile mff = ff;
              mff.merge = true;
              ImFont *f = addFontFile(mff, m_params, ranges, base_dir_abs, m_owned_font_data, cfg);
              (void)f; // merged; no separate role pointer necessary
            }
            // record role as present (point to Body for retrieval semantics)
            if (body)
              m_fonts[FontRole::Icons] = body;
          }
          if (auto it = pack_ptr->roles.find(FontRole::Emoji);
              it != pack_ptr->roles.end()) {
            for (const auto &ff : it->second) {
              FontFile mff = ff;
              mff.merge = true;
              ImFont *f = addFontFile(mff, m_params, ranges, base_dir_abs, m_owned_font_data, cfg);
              (void)f;
            }
            if (body)
              m_fonts[FontRole::Emoji] = body;
          }

          // Headings H1/H2/H3 (separate instances, may reuse Body TTF path with
          // different size)
          auto add_headline = [&](FontRole role, float px_default) -> ImFont * {
            const FontFile *chosen = nullptr;
            if (auto it = pack_ptr->roles.find(role);
                it != pack_ptr->roles.end() && !it->second.empty())
              chosen = &it->second.front();
            FontFile ff{};
            if (chosen) {
              ff = *chosen;
              if (ff.size_px <= 0.0f)
                ff.size_px = px_default;
            } else if (body) {
              // reuse first Body file path
              const auto &bodyv = pack_ptr->roles.at(FontRole::Body);
              ff = bodyv.front();
              ff.size_px = px_default;
            } else {
              return nullptr;
            }
            return add_single(role, ff);
          };

          add_headline(FontRole::H1, m_px_h1);
          add_headline(FontRole::H2, m_px_h2);
          add_headline(FontRole::H3, m_px_h3);

          // Bold/Italic/BoldItalic/Monospace
          auto add_optional_role = [&](FontRole role) -> ImFont * {
            if (auto it = pack_ptr->roles.find(role);
                it != pack_ptr->roles.end() && !it->second.empty())
              return add_single(role, it->second.front());
            return nullptr;
          };
          add_optional_role(FontRole::Bold);
          add_optional_role(FontRole::Italic);
          add_optional_role(FontRole::BoldItalic);
          add_optional_role(FontRole::Monospace);
        }

        // If still no Body, try a minimal fallback (Roboto + Icons) using base_dir
        if (!body) {
          FontFile fb{};
          fb.path = IMGUIX_FONTS_FALLBACK_BODY_BASENAME;
          fb.size_px = m_px_body;
          body = add_single(FontRole::Body, fb);

          FontFile ic{};
          ic.path = IMGUIX_FONTS_FALLBACK_ICONS_BASENAME;
          ic.size_px = m_px_body;
          ic.merge = true;
          addFontFile(ic, m_params, ranges, base_dir_abs, m_owned_font_data, cfg);
          if (body)
            m_fonts[FontRole::Icons] = body;

          // Headings reuse Body path with different sizes
          FontFile h{};
          h.path = fb.path;
          h.size_px = m_px_h1;
          add_single(FontRole::H1, h);
          h.size_px = m_px_h2;
          add_single(FontRole::H2, h);
          h.size_px = m_px_h3;
          add_single(FontRole::H3, h);
        }
      } else {
            // --- MANUAL MODE ---
            if (m_manual.has_body) {
                // Body root
                body = add_single(FontRole::Body, m_manual.body);

                auto do_merge_vec = [&](const std::vector<FontFile> &vec) {
                    for (auto ff : vec) {
                        ff.merge = true;
                        (void)addFontFile(ff, m_params, ranges, base_dir_abs, m_owned_font_data, cfg);
                    }
                };

                // Explicit roles
                bool merged_icons = !m_manual.merges_icons.empty();
                bool merged_emoji = !m_manual.merges_emoji.empty();
                do_merge_vec(m_manual.merges_icons);
                do_merge_vec(m_manual.merges_emoji);

                // Legacy: if old method used, include both roles
                if (!m_manual.merges_unknown.empty()) {
                    do_merge_vec(m_manual.merges_unknown);
                    merged_icons = true;
                    merged_emoji = true;
                }

                if (body) {
                    if (merged_icons)
                        m_fonts[FontRole::Icons] = body;
                    if (merged_emoji)
                        m_fonts[FontRole::Emoji] = body;
                }

                // Headlines
                auto ensure_headline = [&](FontRole role, float px_default) {
                    auto it = m_manual.headlines.find(role);
                    if (it != m_manual.headlines.end()) {
                        add_single(role, it->second);
                    } else 
                    if (body) {
                        // reuse body path as a convenience
                        FontFile ff = m_manual.body;
                        ff.size_px = px_default;
                        add_single(role, ff);
                    }
                };
                ensure_headline(FontRole::H1, m_px_h1);
                ensure_headline(FontRole::H2, m_px_h2);
                ensure_headline(FontRole::H3, m_px_h3);
                ensure_headline(FontRole::Bold, m_px_body);
                ensure_headline(FontRole::BoldItalic, m_px_body);
            } else {
                br.message = u8"Manual mode: Body font not provided";
            }
        }

#       if IMGUIX_SUPPRESS_EMOJI_CONTROL_GLYPHS
        {
            // Важно: ДО Build(). updateBackendTexture() обычно вызывает Build() внутри.
            ImFontAtlas* atlas = ImGui::GetIO().Fonts;
            // Можно пройтись по всем шрифтам в атласе
            for (ImFont* f : atlas->Fonts)
                AddInvisibleEmojiControls(atlas, f);
        }
#       endif

        // Update backend texture for active backend
        if (!updateBackendTexture()) {
            br.success = false;
            br.message = u8"Backend font texture update failed";
            return br;
        }

        // Default font
        if (ImFont *f = getFont(FontRole::Body)) {
            io.FontDefault = f;
        }

        // Done
        br.success = true;
        br.fonts = m_fonts;
        m_dirty = false;
        return br;
    }

    inline BuildResult FontManager::rebuildIfNeeded() {
        if (!m_dirty) {
            BuildResult ok{};
            ok.success = true;
            ok.fonts = m_fonts;
            return ok;
        }
        return buildNow();
    }

    inline BuildResult FontManager::initFromJsonOrDefaults() {
        using nlohmann::json;
#   ifdef IMGUIX_FONTS_ENABLE_JSON
        BuildResult br{};

        // Load JSON text (if exists)
        fs::path cfg_path;
#       ifdef __EMSCRIPTEN__
        cfg_path = fs::u8path(m_config_path);
#       else
        cfg_path = ImGuiX::Utils::resolveExecPathFs(fs::u8path(m_config_path));
        // Fallback: if empty or not found, try base_dir/fonts.json
        if (readTextFile(cfg_path).empty()) {
            const fs::path base_abs = ImGuiX::Utils::resolveExecPathFs(fs::u8path(m_params.base_dir));
            cfg_path = (base_abs / IMGUIX_FONTS_CONFIG_BASENAME).lexically_normal();
        }
#       endif

      const std::string json_text = readTextFile(cfg_path);

        if (!json_text.empty()) {
            json j;
            try {
                j = json::parse(json_text);

                // base_dir (optional)
                if (j.contains(u8"base_dir") && j[u8"base_dir"].is_string())
                m_params.base_dir = j[u8"base_dir"].get<std::string>();

                // markdown_sizes (optional)
                if (j.contains(u8"markdown_sizes") && j[u8"markdown_sizes"].is_object()) {
                    const auto &ms = j[u8"markdown_sizes"];
                    if (ms.contains(u8"body")) m_px_body = ms[u8"body"].get<float>();
                    if (ms.contains(u8"h1")) m_px_h1 = ms[u8"h1"].get<float>();
                    if (ms.contains(u8"h2")) m_px_h2 = ms[u8"h2"].get<float>();
                    if (ms.contains(u8"h3")) m_px_h3 = ms[u8"h3"].get<float>();
                }

                // locales (packs)
                if (j.contains(u8"locales") && j[u8"locales"].is_object()) {
                    for (auto it = j[u8"locales"].begin(); it != j[u8"locales"].end(); ++it) {
                        const std::string loc = it.key();
                        const json &L = it.value();

                        LocalePack pack{};
                        pack.locale = loc;

                        if (L.contains(u8"inherits") && L[u8"inherits"].is_string())
                        pack.inherits = L[u8"inherits"].get<std::string>();

                        // ranges: array of pairs OR string preset (e.g.,
                        // "Default+Cyrillic+Punct")
                        if (L.contains(u8"ranges")) {
                            if (L[u8"ranges"].is_array()) {
                                for (const auto &v : L[u8"ranges"])
                                    pack.ranges.push_back(static_cast<ImWchar>(v.get<int>()));
                                // Append 0 terminator later inside buildRangesFromPack()
                            } else if (L[u8"ranges"].is_string()) {
                                pack.ranges_preset =
                                L[u8"ranges"].get<std::string>(); // <-- IMPORTANT: keep
                            }
                        }

                        // roles...
                        if (L.contains(u8"roles") && L[u8"roles"].is_object()) {
                            const auto &R = L[u8"roles"];
                            auto parse_files = [&](const char *key, FontRole role) {
                                if (R.contains(key) && R[key].is_array()) {
                                for (const auto &item : R[key]) {
                                        FontFile ff{};
                                        if (item.contains(u8"path"))
                                            ff.path = item[u8"path"].get<std::string>();
                                        if (item.contains(u8"size_px"))
                                            ff.size_px = item[u8"size_px"].get<float>();
                                        if (item.contains(u8"merge"))
                                            ff.merge = item[u8"merge"].get<bool>();
                                        if (item.contains(u8"freetype_flags"))
                                            ff.freetype_flags = item[u8"freetype_flags"].get<unsigned>();
                                        if (item.contains(u8"extra_glyphs"))
                                            ff.extra_glyphs = item[u8"extra_glyphs"].get<std::string>();
                                        if (item.contains(u8"baseline_offset"))
                                            ff.baseline_offset = item[u8"baseline_offset"].get<float>();
                                        pack.roles[role].push_back(std::move(ff));
                                    }
                                }
                            };
                            parse_files(u8"Body", FontRole::Body);
                            parse_files(u8"H1", FontRole::H1);
                            parse_files(u8"H2", FontRole::H2);
                            parse_files(u8"H3", FontRole::H3);
                            parse_files(u8"Monospace", FontRole::Monospace);
                            parse_files(u8"Bold", FontRole::Bold);
                            parse_files(u8"Italic", FontRole::Italic);
                            parse_files(u8"BoldItalic", FontRole::BoldItalic);
                            parse_files(u8"Icons", FontRole::Icons);
                            parse_files(u8"Emoji", FontRole::Emoji);
                        }

                        m_packs[pack.locale] = std::move(pack);
                    }

                    // Second pass: resolve inheritance after all packs are read
                    {
                        bool changed = true;
                        int guard = 0;
                        while (changed && guard++ < 16) {
                            changed = false;
                            for (auto &kv : m_packs) {
                                auto &child = kv.second;
                                if (child.inherits.empty())
                                    continue;
                                auto pit = m_packs.find(child.inherits);
                                if (pit == m_packs.end())
                                    continue;
                                const auto &parent = pit->second;

                                // Roles: copy missing ones (do not overwrite existing)
                                for (const auto &rp : parent.roles)
                                if (!child.roles.count(rp.first)) {
                                    child.roles[rp.first] = rp.second;
                                    changed = true;
                                }

                                // Ranges: inherit if not set explicitly
                                if (child.ranges.empty() && !parent.ranges.empty()) {
                                    child.ranges = parent.ranges;
                                    changed = true;
                                }
                                // Preset: inherit if not set explicitly
                                if (child.ranges_preset.empty() &&
                                    !parent.ranges_preset.empty()) {
                                    child.ranges_preset = parent.ranges_preset;
                                    changed = true;
                                }
                            }
                        }
                    }
                }

            } catch (const std::exception &e) {
                br.success = false;
                br.message = std::string(u8"JSON parse error: ") + e.what();
                // Fallback to defaults below
            }
      }

        // Choose active locale, or fallback to "default"
        if (!m_packs.empty()) {
            if (!m_packs.count(m_active_locale)) {
                if (m_packs.count(u8"default"))
                m_active_locale = u8"default";
            }
        }

        // Build now (either from packs or fallback)
        return buildNow();
#   else
        // JSON disabled: just build fallback (Roboto + Icons) using current
        // BuildParams/base_dir.
        return buildNow();
#   endif
    }

    inline std::string FontManager::readTextFile(const std::filesystem::path &path) {
        std::ifstream ifs(path, std::ios::in | std::ios::binary);
        if (!ifs) return {};
        std::ostringstream oss;
        oss << ifs.rdbuf();
        return oss.str();
    }

    inline void FontManager::addExtraGlyphs(
            ImFontGlyphRangesBuilder &b,
            const std::string &utf8
        ) {
        if (!utf8.empty()) b.AddText(utf8.c_str());
    }

} // namespace ImGuiX::Fonts
