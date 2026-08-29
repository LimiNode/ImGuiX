#include <imgui.h>
#include <imguix/utils/path_utils.hpp>
#include <fstream>
#include <iterator>

namespace ImGuiX {

    WindowInstance::WindowInstance(int id, ApplicationContext& app, std::string name)
        : EventMediator(app.eventBus()),
          m_window_id(id),
          m_window_name(std::move(name)),
          m_application(app) {
        m_theme_manager.registerTheme("light", std::make_unique<Themes::LightTheme>());
        m_theme_manager.registerTheme("dark", std::make_unique<Themes::DarkTheme>());
    }

    void WindowInstance::drawContent() {
        setCurrentWindow();
        for (auto& ctrl : m_controllers) {
            ctrl->drawContent();
        }
    }

    void WindowInstance::drawUi() {
        setCurrentWindow();
        for (auto& ctrl : m_controllers) {
            ctrl->drawUi();
        }
        notifications().render();
    }

    void WindowInstance::initializePendingControllers() {
        setCurrentWindow();
        for (auto* ctrl : m_pending_controllers) {
            if (ctrl) ctrl->onInit();
        }
        m_pending_controllers.clear();
    }

// --- WindowInterface interface ---

    int WindowInstance::id() const {
        return m_window_id;
    }

    const std::string& WindowInstance::name() const {
        return m_window_name;
    }

    int WindowInstance::width() const {
        return m_width;
    }

    int WindowInstance::height() const {
        return m_height;
    }
    
    void WindowInstance::setTheme(std::string id) {
        themeManager().setTheme(std::move(id));
    }
    
    Pubsub::EventBus& WindowInstance::eventBus() {
        return m_application.eventBus();
    }

    ResourceRegistry& WindowInstance::registry() {
        return m_application.registry();
    }

    OptionsStore::Control& WindowInstance::options() {
        return m_application.registry().getResource<OptionsStore>().control();
    }

    const OptionsStore::View& WindowInstance::options() const {
        return m_application.registry().getResource<OptionsStore>().view();
    }
    
    ApplicationContext& WindowInstance::application() {
        return m_application;
    }
    
#   ifdef IMGUIX_USE_SFML_BACKEND
    sf::RenderWindow& WindowInstance::getRenderTarget() {
        return m_window;
    }
#   endif

    const ImGuiX::I18N::LangStore& WindowInstance::langStore() const {
        return m_lang_store;
    }
    
    ImFont* WindowInstance::getFont(ImGuiX::Fonts::FontRole role) const {
        return m_font_manager.getFont(role);
    }
    
    ImGuiX::Fonts::FontManager::View& WindowInstance::fontsView() noexcept { 
        return m_font_manager.view(); 
    }

    ImGuiX::Fonts::FontManager::Control& WindowInstance::fontsControl() noexcept { 
        return m_font_manager.control(); 
    }

    void WindowInstance::fontsSetDpi(float dpi) {
        m_font_manager.setDpi(dpi);
    }

    ImGuiX::Themes::ThemeManager& WindowInstance::themeManager() noexcept { 
        return m_theme_manager; 
    }
    
    ImGuiX::Notify::NotificationManager& WindowInstance::notifications() {
        return m_notification_manager;
    }

    std::filesystem::path WindowInstance::iniPathFs() const {
#   ifdef __EMSCRIPTEN__
#       ifdef IMGUIX_EMSCRIPTEN_IDBFS
        return std::filesystem::path(
            u8"/imguix_fs/imgui-" + std::to_string(m_window_id) + u8".ini").lexically_normal();
#       else
        return {};
#       endif
#   else
        return ImGuiX::Utils::resolveExecPathFs(
            std::filesystem::path(IMGUIX_CONFIG_DIR) / ("imgui-" + std::to_string(m_window_id) + ".ini"));
#   endif
    }

    std::string WindowInstance::iniPath() const {
        return iniPathFs().u8string();
    }

    void WindowInstance::initIni() {
        if (m_is_ini_once) return;
        setCurrentWindow();
        m_ini_path = iniPathFs();
#   ifdef __EMSCRIPTEN__
#       ifdef IMGUIX_EMSCRIPTEN_IDBFS
        ImGui::GetIO().IniFilename = nullptr;
#       else
        ImGui::GetIO().IniFilename = nullptr;
#       endif
#   else
        ImGui::GetIO().IniFilename = nullptr;
#   endif
        m_is_ini_once = true;
    }

    void WindowInstance::loadIni() {
        if (m_is_ini_loaded) return;
        setCurrentWindow();
        if (!m_ini_path.parent_path().empty()) {
            Utils::createDirectories(m_ini_path.parent_path());
        }
        std::ifstream input(m_ini_path, std::ios::binary);
        if (input.good()) {
            const std::string content(
                (std::istreambuf_iterator<char>(input)),
                std::istreambuf_iterator<char>());
            if (!content.empty()) {
                ImGui::LoadIniSettingsFromMemory(content.c_str(), content.size());
            }
        }
        m_is_ini_loaded = true;
    }

    void WindowInstance::saveIniNow() {
        setCurrentWindow();
        if (m_ini_path.empty()) {
            m_ini_path = iniPathFs();
        }
        size_t ini_size = 0;
        const char* ini_data = ImGui::SaveIniSettingsToMemory(&ini_size);
        if (ini_data == nullptr) {
            ImGui::GetIO().WantSaveIniSettings = false;
            return;
        }
        if (!m_ini_path.parent_path().empty()) {
            Utils::createDirectories(m_ini_path.parent_path());
        }
        std::ofstream output(m_ini_path, std::ios::binary | std::ios::trunc);
        if (output.good() && ini_size > 0) {
            output.write(ini_data, static_cast<std::streamsize>(ini_size));
        }
#   if defined(__EMSCRIPTEN__) && defined(IMGUIX_EMSCRIPTEN_IDBFS)
        EM_ASM({ FS.syncfs(false, function(){}); });
#   endif
        ImGui::GetIO().WantSaveIniSettings = false;
    }
    
    void WindowInstance::requestLanguageChange(const std::string& lang) {
        if (!lang.empty()) m_pending_lang = lang;
    }
    
    void WindowInstance::applyPendingLanguageChange() {
        if (m_pending_lang.empty()) return;
        setCurrentWindow();                        // активировать контекст окна
        onBeforeLanguageApply(m_pending_lang);     // виртуальный хук (пересборка шрифтов и т.п.)
        m_lang_store.set_language(m_pending_lang); // фактическая смена языка
        m_font_manager.rebuildIfNeeded();
        m_pending_lang.clear();
    }
    
    // ---
    
    void WindowInstance::fontsStartInit() {
        m_in_init_phase = true;
    }

    void WindowInstance::fontsBeginManual() {
        assert(m_in_init_phase && !m_is_fonts_init && u8"fontsBeginManual() only allowed in onInit(), before initFonts()");
        m_font_manager.beginManual();
        m_is_fonts_manual = true;
    }
    
    void WindowInstance::fontsSetLocale(std::string locale) {
        if (locale == m_font_manager.activeLocale()) {
            return;
        }
        m_font_manager.setLocale(std::move(locale));
    }
    
    void WindowInstance::fontsSetRangesPreset(std::string preset) {
        assert(m_in_init_phase && u8"fontsSetRangesPreset() только в onInit()");
        m_font_manager.setRanges(std::move(preset));
    }

    void WindowInstance::fontsSetRangesExplicit(const std::vector<ImWchar>& pairs) {
        assert(m_in_init_phase && u8"fontsSetRangesExplicit() только в onInit()");
        m_font_manager.setRanges(pairs);
    }

    void WindowInstance::fontsClearRanges() {
        assert(m_in_init_phase && u8"fontsClearRanges() только в onInit()");
        m_font_manager.clearRanges();
    }

    void WindowInstance::fontsAddBody(const ImGuiX::Fonts::FontFile& ff) {
        assert(m_in_init_phase && u8"Only allowed in onInit()");
        m_font_manager.addFontBody(ff);
    }

    void WindowInstance::fontsAddHeadline(ImGuiX::Fonts::FontRole role, const ImGuiX::Fonts::FontFile& ff) {
        assert(m_in_init_phase && u8"Only allowed in onInit()");
        m_font_manager.addFontHeadline(role, ff);
    }

    void WindowInstance::fontsAddMerge(ImGuiX::Fonts::FontRole role, const ImGuiX::Fonts::FontFile& ff) {
        assert(m_in_init_phase && u8"Only allowed in onInit()");
        m_font_manager.addFontMerge(role, ff);
    }

    void WindowInstance::fontsAddMerge(const ImGuiX::Fonts::FontFile& ff) {
        assert(m_in_init_phase && u8"Only allowed in onInit()");
        m_font_manager.addFontMerge(ff);
    }

    bool WindowInstance::fontsBuildNow() {
        assert(m_in_init_phase && u8"Only allowed in onInit()");
        auto br = m_font_manager.buildNow();
        if (br.success) m_is_fonts_init = true;
        if (!br.success) {
            notify(IMGUIX_LOG_EVENT(ImGuiX::Events::LogLevel::Error, u8"Font init failed: {}"));
        }
        return br.success;
    }

    void WindowInstance::buildFonts() {
        m_in_init_phase = false;
        if (m_is_fonts_init) return;
        if (m_is_fonts_manual) return;
        
        auto br = m_font_manager.initFromJsonOrDefaults();
        m_is_fonts_init = br.success;
        if (!br.success) {
            notify(IMGUIX_LOG_EVENT(ImGuiX::Events::LogLevel::Error, u8"Font init failed: {}"));
        }
    }
}
