#pragma once
#ifndef _IMGUIX_CORE_WINDOW_INSTANCE_IWINDOW_INSTANCE_HPP_INCLUDED
#define _IMGUIX_CORE_WINDOW_INSTANCE_IWINDOW_INSTANCE_HPP_INCLUDED

/// \file WindowInstance.hpp
/// \brief Abstract base class for a window instance in ImGuiX.
/// \ingroup Core

#ifdef IMGUIX_USE_SFML_BACKEND
#    include "DeltaClockSfml.hpp"
#    include <imgui-SFML.h>
#    include <SFML/Graphics.hpp>
#endif
#ifdef IMGUIX_USE_GLFW_BACKEND
#    include <imgui_impl_glfw.h>
#    include <imgui_impl_opengl3.h>
#    include <GLFW/glfw3.h>
#    include "imgui_glsl_version.hpp"
#endif
#ifdef IMGUIX_USE_SDL2_BACKEND
#    include <imgui_impl_sdl2.h>
#    include <imgui_impl_opengl3.h>
#    include <SDL.h>
#    include <SDL_opengles2.h>
#    include "imgui_glsl_version.hpp"
#endif

#ifdef IMGUIX_ENABLE_IMPLOT
#include <implot.h>
#endif

#ifdef IMGUIX_ENABLE_IMPLOT3D
#include <implot3d.h>
#endif

#include "WindowInterface.hpp"
#include <filesystem>
#include <imguix/config/paths.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ImGuiX {

    class Controller;

    /// \brief Abstract base class for a window instance.
    ///
    /// Combines event handling, rendering, and controller orchestration.
    /// Derive from this class to implement platform-specific windows.
    class WindowInstance :
        private WindowInterface,
        public Pubsub::EventMediator {
    public:
        /// \brief Construct window with unique ID and name.
        /// \param id Unique window identifier.
        /// \param app Reference to application context interface.
        /// \param name Window name (title).
        explicit WindowInstance(int id, ApplicationContext& app, std::string name);
        
        WindowInstance(const WindowInstance&) = delete;
        WindowInstance& operator=(WindowInstance) = delete;

        WindowInstance(WindowInstance&&) = delete;
        WindowInstance& operator=(WindowInstance&&) = delete;

        virtual ~WindowInstance() noexcept;
        
        /// \brief Optional callback invoked after the window is added to the manager.
        virtual void onInit() {}

        /// \brief Initialize window and create backend resources.
        /// \return True on success.
        virtual bool create();

        /// \brief Initialize window with explicit dimensions.
        /// \param w Width in pixels.
        /// \param h Height in pixels.
        /// \return True on success.
        virtual bool create(int w, int h);

        /// \brief Process input events and window messages.
        virtual void handleEvents();

        /// \brief Update logic for the current frame.
        /// \note Invokes feature models of all attached controllers.
        virtual void tick();

        /// \brief Render scene content (e.g., world, game objects).
        virtual void drawContent();

        /// \brief Render UI overlays, HUDs, debug panels.
        virtual void drawUi();

        /// \brief Finalize frame and present to screen.
        virtual void present();

        /// \brief Creates and registers a controller of given type.
        /// \tparam ControllerType Controller type derived from Controller.
        /// \tparam Args Arguments passed to the controller constructor.
        /// \param args Arguments forwarded to the controller constructor.
        /// \return Reference to the created controller.
        /// \code
        /// auto& ctrl = createController<MyController>(42);
        /// \endcode
        template <typename ControllerType, typename... Args>
        ControllerType& createController(Args&&... args);

        /// \brief Call onInit() on controllers pending initialization.
        void initializePendingControllers();

        // --- WindowInterface interface ---

        /// \brief Return unique window ID.
        /// \return Window identifier.
        int id() const override;

        /// \brief Return window name used as title.
        /// \return Window name.
        const std::string& name() const override;

        /// \brief Return current window width in pixels.
        /// \return Width in pixels.
        int width() const override;

        /// \brief Return current window height in pixels.
        /// \return Height in pixels.
        int height() const override;

        /// \brief Set window dimensions in pixels.
        /// \param w New width in pixels.
        /// \param h New height in pixels.
        void setSize(int w, int h) override;
        
        /// \brief Set the window icon from an image file (currently SFML only).
        /// \param path Path to the icon image file (must be .png or .bmp, 32x32 or 64x64 recommended).
        /// \return True if the icon was loaded and applied successfully.
        bool setWindowIcon(const std::string& path) override;
        
        /// \brief Set active theme identifier.
        /// \param id Identifier of registered theme.
        void setTheme(std::string id) override;
        
        /// \brief Enable or disable clearing the background between frames.
        /// \param disable True to disable clearing.
        void setDisableBackground(bool disable) override { (void)disable; }

        /// \brief Request window to close.
        void close() override;

        /// \brief Minimize the window.
        void minimize() override;

        /// \brief Maximize the window.
        void maximize() override;

        /// \brief Restore the window from minimized or maximized state.
        void restore() override;

        /// \brief Return true if the window is maximized.
        /// \return True when maximized.
        bool isMaximized() const override;

        /// \brief Toggle between maximized and restored states.
        void toggleMaximizeRestore() override;

        /// \brief Activate or deactivate the window.
        /// \param active True to activate.
        /// \return True on success.
        bool setActive(bool active) override;

        /// \brief Return true if the window has focus.
        /// \return True when active.
        bool isActive() const override;

        /// \brief Show or hide the window.
        void setVisible(bool visible) override;

        /// \brief Return true if the window is open.
        /// \return True while the window exists.
        bool isOpen() const override;
        
        /// \brief Make the window context current for rendering.
        /// \note Call only between frames before ImGui::NewFrame().
        virtual void setCurrentWindow();

        /// \brief Provide access to the global event bus.
        /// \return Event bus.
        Pubsub::EventBus& eventBus() override;

        /// \brief Provide access to the shared resource registry.
        /// \return Resource registry.
        ResourceRegistry& registry() override;

        /// \brief Provide access to the global options store.
        /// \return Options store control interface.
        OptionsStore::Control& options() override;

        /// \brief Provide read-only access to the global options store.
        /// \return Options store view.
        const OptionsStore::View& options() const override;

        /// \brief Return reference to the owning application.
        /// \return Application context interface.
        ApplicationContext& application() override;

#       ifdef IMGUIX_USE_SFML_BACKEND
        /// \brief Access the underlying SFML render window.
        /// \return SFML render target.
        sf::RenderWindow& getRenderTarget() override;
#       endif

        // --- Lang and Fonts ---

        /// \brief Get language store.
        /// \return Language store.
        const ImGuiX::I18N::LangStore& langStore() const override;
        
        /// \brief Get font by role from the current font atlas.
        /// \param role Logical font role.
        /// \return Font pointer or nullptr if role is unavailable.
        ImFont* getFont(ImGuiX::Fonts::FontRole role) const override;
        
        /// \brief Read-only view of the font manager.
        /// \return Font manager view.
        ImGuiX::Fonts::FontManager::View& fontsView() noexcept;

        /// \brief Control interface for the font manager.
        /// \return Font manager control interface.
        ImGuiX::Fonts::FontManager::Control& fontsControl() noexcept;

        /// \brief Set logical DPI used when rebuilding this window's fonts.
        /// \param dpi Display DPI where 96 equals 1.0 scale.
        void fontsSetDpi(float dpi);
        
        /// \brief Request window to switch its UI language.
        /// \param lang Language code.
        void requestLanguageChange(const std::string& lang);
        
        /// \brief Apply pending language change.
        /// \note Internal use.
        void applyPendingLanguageChange();
        
        /// \brief Start font initialization.
        /// \note Internal use.
        void fontsStartInit();

        /// \brief Build fonts atlas.
        /// \note Internal use.
        void buildFonts();

        // --- Themes ---

        /// \brief Access the theme manager.
        /// \return Theme manager.
        ImGuiX::Themes::ThemeManager& themeManager() noexcept override;

        /// \brief Apply the currently selected theme.
        void updateCurrentTheme() { themeManager().updateCurrentTheme(); }

        // --- Notification ---
        
        /// \brief Access the notification manager.
        /// \return Notification manager instance.
        ImGuiX::Notify::NotificationManager &notifications() override;

        // --- ImGui ini ---

        /// \brief Compute file path for storing ImGui ini settings.
        /// \return Absolute path to ini file.
        /// \note Internal use.
        std::filesystem::path iniPathFs() const;

        /// \brief Compute file path for storing ImGui ini settings as UTF-8 string.
        /// \return Absolute path to ini file.
        /// \note Compatibility wrapper over iniPathFs().
        std::string iniPath() const;

        /// \brief Prepare ImGui to use the window-specific ini file.
        /// \note Internal use.
        void initIni();

        /// \brief Load ImGui settings from the ini file if not already loaded.
        /// \note Internal use.
        void loadIni();

        /// \brief Save ImGui ini settings to disk.
        /// \note Internal use.
        void saveIniNow();

    protected:

        // --- onInit phase: manual atlas assembly ---

        /// \brief Begin manual font configuration.
        /// \note Call during onInit() before building fonts.
        void fontsBeginManual();

        /// \brief Set locale for subsequent font operations.
        /// \param locale Locale identifier.
        void fontsSetLocale(std::string locale);

        /// \brief Select preset character ranges.
        /// \param preset '+'-separated named or numeric tokens.
        void fontsSetRangesPreset(std::string preset);

        /// \brief Define explicit character ranges.
        /// \param pairs Consecutive ImWchar pairs defining ranges.
        void fontsSetRangesExplicit(const std::vector<ImWchar>& pairs);

        /// \brief Clear previously specified character ranges.
        void fontsClearRanges();

        /// \brief Add body font file.
        /// \param ff Font file.
        void fontsAddBody(const ImGuiX::Fonts::FontFile& ff);

        /// \brief Add headline font file.
        /// \param role Font role.
        /// \param ff Font file.
        void fontsAddHeadline(ImGuiX::Fonts::FontRole role, const ImGuiX::Fonts::FontFile& ff);

        /// \brief Add merge font file for role.
        /// \param role Font role.
        /// \param ff Font file.
        void fontsAddMerge(ImGuiX::Fonts::FontRole role, const ImGuiX::Fonts::FontFile& ff);
        
        /// \brief Add merge font file to body chain.
        /// \param ff Font file.
        void fontsAddMerge(const ImGuiX::Fonts::FontFile& ff);
        
        /// \brief Build fonts immediately.
        /// \return True on success.
        bool fontsBuildNow();

    protected:
#ifdef IMGUIX_USE_SFML_BACKEND
        sf::RenderWindow m_window; ///< Underlying SFML render window.
#elif defined(IMGUIX_USE_GLFW_BACKEND)
        GLFWwindow* m_window = nullptr; ///< Pointer to the GLFW window.
        ImGuiContext* m_imgui_ctx = nullptr;
        const char* selectGlslForGlfw(GLFWwindow* w) noexcept;
#elif defined(IMGUIX_USE_SDL2_BACKEND)
        SDL_Window* m_window = nullptr; ///< SDL window handle.
        SDL_GLContext m_gl_context = nullptr; ///< Associated GL context.
        ImGuiContext* m_imgui_ctx = nullptr;
        const char* selectGlslForSdl(SDL_Window* w) noexcept;
#endif
#ifdef IMGUIX_ENABLE_IMPLOT
        ImPlotContext* m_implot_ctx = nullptr; ///< ImPlot context.
#endif
#ifdef IMGUIX_ENABLE_IMPLOT3D
        ImPlot3DContext* m_implot3d_ctx = nullptr; ///< ImPlot3D context.
#endif
        int m_window_id;                    ///< Unique window identifier.
        std::string m_window_name;          ///< Internal window name.
        int m_width = 1280;                 ///< Current window width.
        int m_height = 720;                 ///< Current window height.
        bool m_is_active = true;            ///< Whether the window has focus.
        bool m_is_open = false;             ///< Whether the window is currently open.
        bool m_is_visible = true;           ///< Visibility flag.

        ApplicationContext& m_application;  ///< Reference to the owning application.
        std::vector<std::unique_ptr<Controller>> m_controllers; ///< Attached controllers.
        std::vector<Controller*> m_pending_controllers; ///< Controllers awaiting onInit.
        std::filesystem::path m_ini_path;   ///< Path to the window-specific ImGui ini file.
        bool m_is_ini_once = false;         ///< Ensures imgui ini is saved only once.
        bool m_is_ini_loaded = false;       ///< Indicates whether ini settings have been loaded.
    
        bool m_is_fonts_manual = false;     ///< True when manual font configuration is active.
        bool m_in_init_phase = false;       ///< Guard flag for onInit phase operations.
        bool m_is_fonts_init = false;       ///< Indicates whether fonts have been built.
        ImGuiX::Fonts::FontManager m_font_manager;    ///< Manages ImGui font atlas.
        ImGuiX::Themes::ThemeManager m_theme_manager; ///< Manages ImGui style themes.
        ImGuiX::I18N::LangStore    m_lang_store{};    ///< Localization storage for this window.
        std::string                m_pending_lang;    ///< Language code pending to apply.
        ImGuiX::Notify::NotificationManager m_notification_manager{}; ///< Toast notifications manager.


        /// \brief Hook before applying requested language.
        /// \param lang Language code to apply.
        virtual void onBeforeLanguageApply(const std::string& lang) { (void)lang; }
    };

} // namespace ImGuiX

#include "WindowInstance.tpp"

#ifdef IMGUIX_HEADER_ONLY
#    include "WindowInstance.ipp"
#    ifdef IMGUIX_USE_SFML_BACKEND
#        include "SfmlWindowInstance.ipp"
#    elif defined(IMGUIX_USE_GLFW_BACKEND)
#        include "GlfwWindowInstance.ipp"
#    elif defined(IMGUIX_USE_SDL2_BACKEND)
#        include "Sdl2WindowInstance.ipp"
#    endif
#endif

#endif // _IMGUIX_CORE_WINDOW_INSTANCE_IWINDOW_INSTANCE_HPP_INCLUDED
