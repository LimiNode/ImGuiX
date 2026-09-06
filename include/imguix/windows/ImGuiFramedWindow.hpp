#pragma once
#ifndef _IMGUIX_WINDOWS_IMGUI_FRAMED_WINDOW_HPP_INCLUDED
#define _IMGUIX_WINDOWS_IMGUI_FRAMED_WINDOW_HPP_INCLUDED

/// \file ImGuiFramedWindow.hpp
/// \brief Framed ImGui window with custom title bar, optional side panel, and backend-specific chrome.
/// \details
/// Typical setup flow:
/// - pass \ref WindowFlags to enable menu bar, control buttons, transparency, or background override;
/// - configure layout via \ref ImGuiFramedWindowConfig (title bar height, side panel width, resize border);
/// - override drawing hooks for custom content:
///   - \ref drawTitleBarText for left title area content;
///   - \ref drawSidePanel for optional side panel content;
///   - \ref drawCornerIcon for optional top-left icon slot content when \ref WindowFlags::HasCornerIconArea is set;
///   - \ref drawMenuBar for menu bar actions when \ref WindowFlags::HasMenuBar is set.
///
/// How to enable side panel:
/// - classic layout: set `ImGuiFramedWindowConfig::side_panel_width` to a positive value;
/// - corner-icon layout (\ref WindowFlags::HasCornerIconArea): keep `side_panel_width <= 0` for auto width
///   (`corner_icon_mode_area_width` when set, otherwise `title_bar_height + corner_icon_mode_gap`)
///   or set a positive value for custom width;
/// - override \ref drawSidePanel to render panel widgets.
///
/// Menu bar geometry:
/// - when \ref WindowFlags::HasMenuBar is enabled, menu bar is anchored to the top edge of the body region
///   (directly below title-bar row) in both classic and corner-icon layouts.
/// - corner-icon layout supports menu placement modes via
///   \ref ImGuiFramedWindowConfig::corner_menu_bar_placement:
///   `MainRegion` (backward-compatible), `InTitleBar`, `BelowTitleBar`.
///
/// Minimal customization example:
/// \code{.cpp}
/// class MainWindow final : public ImGuiX::Windows::ImGuiFramedWindow {
/// public:
///     MainWindow(int id, ImGuiX::ApplicationContext& app)
///         : ImGuiFramedWindow(
///               id,
///               app,
///               "main_window",
///               "Main Window",
///               ImGuiX::Windows::WindowFlags::HasMenuBar |
///                   ImGuiX::Windows::WindowFlags::DefaultControlButtons,
///               [] {
///                   ImGuiX::Windows::ImGuiFramedWindowConfig cfg{};
///                   cfg.side_panel_width = 56;
///                   cfg.title_bar_height = 34;
///                   return cfg;
///               }()) {}
///
/// protected:
///     void drawMenuBar() override;
///     void drawSidePanel() override;
///     void drawCornerIcon() override;
/// };
/// \endcode

#ifdef _WIN32
#   include <windows.h>
#endif

#include <imgui.h>

#include "corner_layout_options.hpp"
#include "window_flags.hpp"

namespace ImGuiX::Windows {

    /// \brief Side-panel content alignment policy.
    enum class SidePanelContentAlignment {
        LegacyLeftInset, ///< Preserve legacy left-inset placement.
        SymmetricInset   ///< Apply symmetric horizontal inset without changing side-panel width.
    };

    /// \brief Runtime configuration for framed window geometry and chrome labels.
    /// \details
    /// In classic mode, `side_panel_width <= 0` disables side panel region completely.
    /// In corner-icon mode, `side_panel_width <= 0` enables auto width (`corner_icon_mode_area_width` when set,
    /// otherwise `title_bar_height + corner_icon_mode_gap`).
    /// `title_bar_height` controls both title region height and default side panel button sizing.
    /// Corner icon mode options apply only when \ref WindowFlags::HasCornerIconArea is enabled.
    /// Corner mode rounding/border toggles are controlled by
    /// \ref WindowFlags::CornerModeRounding and \ref WindowFlags::CornerModeBorder.
    /// `corner_menu_bar_placement` selects where menu bar is rendered in corner mode.
    /// `corner_rounding_style` is effective only when \ref WindowFlags::CornerModeRounding is enabled.
    struct ImGuiFramedWindowConfig {
        int min_width = 640;           ///< Minimum client width in pixels.
        int min_height = 480;          ///< Minimum client height in pixels.
        int frame_corner_radius = 8;   ///< Corner radius used for native rounded region and root host-window border rounding.
        int resize_border = 8;         ///< Manual resize hit-test thickness in pixels.
        int title_bar_height = 32;     ///< Custom title bar height in pixels.
        int side_panel_width = 0;      ///< Classic mode: `<= 0` disables side panel. Corner mode: `<= 0` means auto width (`corner_icon_mode_area_width` when set, otherwise `title_bar_height + corner_icon_mode_gap`).
        float title_content_left_inset = -1.0f; ///< Left inset in px for title content baseline (`drawTitleBarText`); `< 0` uses auto style-based inset.
        float side_panel_content_left_inset = -1.0f; ///< Left inset in px for side-panel content baseline (`drawSidePanel`); `< 0` uses auto style-based inset.
        ImVec2 main_region_padding = ImVec2(-1.0f, -1.0f); ///< Inner padding for framed-window main content area; each component `< 0` falls back to runtime `style.WindowPadding` and does not affect title/menu/side regions.
        SidePanelContentAlignment side_panel_content_alignment = SidePanelContentAlignment::LegacyLeftInset; ///< Alignment policy for side-panel content region.
        float frame_stroke_thickness = 1.0f; ///< Thickness in px for classic/corner separator strokes (title/side split lines).
        float frame_outer_stroke_thickness = 2.0f; ///< Thickness in px for the outer host-frame stroke (background-colored line).
        float frame_inner_stroke_thickness = 1.0f; ///< Thickness in px for the inner host-frame stroke (ImGui border-colored line).
        float corner_icon_mode_rounding_radius = 6.0f; ///< Rounding radius in pixels used when \ref WindowFlags::CornerModeRounding is enabled.
        CornerRoundingStyle corner_rounding_style = CornerRoundingStyle::Legacy; ///< Corner rounding style for title/side in corner layout.
        float corner_icon_mode_area_width = -1.0f; ///< Width in px of reserved corner icon area; `< 0` uses `title_bar_height + corner_icon_mode_gap`.
        float corner_icon_mode_icon_size = -1.0f; ///< Icon size in px for corner icon slot; `< 0` uses auto-fit from current corner icon area geometry with frame-chrome compensation on left/top.
        float corner_icon_mode_gap = -1.0f; ///< Gap between icon-area and title/side; `< 0` uses runtime `style.WindowPadding.x`.
        CornerMenuBarPlacement corner_menu_bar_placement = CornerMenuBarPlacement::MainRegion; ///< Corner mode menu-bar placement strategy.
        const char* close_button_text = u8"X##imguix_btn_close"; ///< Close button label + hidden ImGui ID suffix.
        const char* minimize_button_text = u8"_##imguix_btn_minimize"; ///< Minimize button label + hidden ImGui ID suffix.
        const char* maximize_button_text = u8"[]##imguix_btn_maximize"; ///< Maximize button label + hidden ImGui ID suffix.
        ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); ///< Clear color used when transparency is disabled.
    };

    /// \brief Custom framed host window rendered through Dear ImGui.
    /// \details
    /// Feature switches are controlled by \ref WindowFlags:
    /// - \ref WindowFlags::HasMenuBar enables menu bar region and calls \ref drawMenuBar.
    /// - \ref WindowFlags::ShowControlButtons shows system controls in title bar.
    /// - \ref WindowFlags::ClassicStyledControlButtons, \ref WindowFlags::ImGuiStyledControlButtons or
    ///   \ref WindowFlags::MacStyledControlButtons selects button style (mutually exclusive).
    /// - \ref WindowFlags::DisableBackground makes root ImGui host transparent while keeping a visible border.
    /// - \ref WindowFlags::EnableTransparency requests transparent native surface on supported backends.
    /// - \ref WindowFlags::HasCornerIconArea reserves top-left square icon slot and offsets title bar/side panel layout.
    /// - \ref WindowFlags::CornerModeRounding and \ref WindowFlags::CornerModeBorder control corner-layout chrome.
    class ImGuiFramedWindow : public WindowInstance {
    public:
        /// \brief Constructs a framed window instance.
        /// \param id        Unique window identifier.
        /// \param app       Reference to the owning application.
        /// \param name      Internal window name.
        /// \param title     Title displayed in the custom frame.
        /// \param flags     Window behavior flags.
        /// \param config    Configuration parameters.
        ImGuiFramedWindow(
            int id,
            ApplicationContext& app,
            std::string name,
            std::string title,
            WindowFlags flags =
                WindowFlags::ShowControlButtons |
                WindowFlags::ImGuiStyledControlButtons,
            ImGuiFramedWindowConfig config = {});

        ~ImGuiFramedWindow() override = default;

        /// \brief Create backend resources and open the window.
        /// \return True if creation succeeds.
        bool create() override;

        /// \brief Create the window with the specified size.
        /// \param w Width in pixels.
        /// \param h Height in pixels.
        /// \return True if creation succeeds.
        bool create(int w, int h) override;

        /// \brief Set native window icon and update optional corner icon source.
        /// \param path Path to icon image file.
        /// \return SFML backend: result of native icon apply; other backends: false.
        bool setWindowIcon(const std::string& path) override;

        /// \brief Draw custom menu bar content.
        /// \details Called only when \ref WindowFlags::HasMenuBar is enabled.
        virtual void drawMenuBar() {};

        /// \brief Updates window logic each frame.
        void tick() override;

        /// \brief Draws the ImGui interface for this window.
        void drawUi() override;

        /// \brief Enable or disable clearing the backend render target background between frames.
        /// \param disable True to skip clearing the background.
        void setDisableBackground(bool disable) override;

    protected:
        std::string m_title;                  ///< Text displayed on the title bar.
        WindowFlags m_flags = WindowFlags::NoFlags; ///< Current window flags.
        ImGuiFramedWindowConfig m_config;        ///< Runtime configuration values.
        bool m_disable_background = false;       ///< Skip clearing the background when true.
        /// Client rectangle occupied by interactive title-bar controls.
        /// Coordinates are relative to the native client window for platform hit-testing.
        ImVec4 m_title_bar_interactive_rect = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
#       ifdef IMGUIX_USE_SFML_BACKEND
        int m_prev_width = -1;   ///< Width before entering fullscreen.
        int m_prev_height = -1;  ///< Height before entering fullscreen.
        sf::Texture m_corner_icon_texture; ///< Cached texture used by default drawCornerIcon implementation.
        bool m_has_corner_icon_texture = false; ///< Whether m_corner_icon_texture is ready for drawing.
        bool applyCommonWindowSetup();
#       ifdef _WIN32
        RECT m_minimize_btn_rect = {0}; ///< Rectangle of the minimize button.
        RECT m_maximize_btn_rect = {0}; ///< Rectangle of the maximize button.
        RECT m_close_btn_rect = {0};    ///< Rectangle of the close button.
        bool m_in_manual_sizing = false; ///< True while resizing manually.
        void setupWindowEffects(HWND hwnd);
        void applyRoundedRegion(HWND hwnd, int width, int height, int radius);
        static LRESULT CALLBACK SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                     UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    public:
        LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#       endif
#       endif
    
    protected:
        /// \brief Draw content in the left area of title bar.
        /// \details Override for logo/title text customization. Right-side control buttons are drawn separately.
        virtual void drawTitleBarText();

        /// \brief Draw content inside optional left side panel.
        /// \details Called only when `m_config.side_panel_width > 0`.
        virtual void drawSidePanel() {}

        /// \brief Draw content inside optional top-left corner icon slot.
        /// \details Called only when \ref WindowFlags::HasCornerIconArea is enabled.
        /// Default implementation draws icon loaded via setWindowIcon on SFML backend and no-op on others.
        virtual void drawCornerIcon();

        /// \brief Draws all registered controllers for current frame.
        void drawControllersContent();

        /// \brief Draw menu bar region for classic/main-column geometry.
        /// \param x Left position in current window coordinates.
        /// \param y Top position in current window coordinates.
        /// \param width Region width in pixels.
        /// \param menu_bar_height Menu bar height in pixels.
        /// \return Height consumed by menu bar (0 when disabled).
        float drawClassicMainMenuRegion(float x, float y, float width, float menu_bar_height);

        /// \brief Draw menu bar in corner `BelowTitleBar` region.
        /// \param x Left position in current window coordinates.
        /// \param y Top position in current window coordinates.
        /// \param width Region width in pixels.
        /// \param menu_bar_height Menu bar height in pixels.
        /// \return Height consumed by menu bar (0 when disabled).
        float drawCornerBelowTitleMenuRegion(float x, float y, float width, float menu_bar_height);

        /// \brief Draw menu bar inside corner title bar after title text.
        /// \param menu_bar_height Menu bar height in pixels.
        /// \param style Active Dear ImGui style reference.
        void drawCornerInTitleMenuRegion(float menu_bar_height, const ImGuiStyle& style);

        /// \brief Draw classic framed-window layout.
        /// \param menu_bar_height Menu bar height in pixels.
        void drawClassicLayout(float menu_bar_height);

        /// \brief Draw corner-icon framed-window layout.
        /// \param menu_bar_height Menu bar height in pixels.
        void drawCornerLayout(float menu_bar_height);

        /// \brief Draw control buttons in the title bar.
        void drawControlButtons();

        /// \brief Draw macOS-styled control buttons.
        void drawMacStyledControlButtons();

        /// \brief Draw ImGui-styled control buttons.
        void drawImGuiStyledControlButtons();

        /// \brief Compute horizontal strip width reserved for control buttons.
        /// \return Reserved width at the right side of title bar.
        float getControlButtonsReservedWidth() const;

        /// \brief True when corner chrome requires compact control-buttons geometry.
        bool isCompactControlButtonsMode() const;

        /// \brief Per-button size reduction in compact control-buttons mode.
        float getControlButtonsCompactDelta() const;

        /// \brief Additional right inset for control-buttons strip in compact mode.
        float getControlButtonsRightInset() const;

        /// \brief Render frame manually when required by backend.
        void renderFrameManually();

    private:
        enum class ControlButtonsStyle {
            Classic,
            ImGui,
            Mac
        };

        /// \brief Resolved side-panel content region inside the panel child.
        struct SidePanelContentRegion {
            float x = 0.0f;     ///< Local X offset from side-panel origin.
            float width = 0.0f; ///< Content width available for drawSidePanel().
        };

        /// \brief Host-frame stroke insets applied to layout regions that touch outer window edges.
        struct HostFrameStrokeInsets {
            float left = 0.0f;   ///< Left inset from outer+inner host-frame strokes.
            float top = 0.0f;    ///< Top inset from outer+inner host-frame strokes.
            float right = 0.0f;  ///< Right inset from outer+inner host-frame strokes.
            float bottom = 0.0f; ///< Bottom inset from outer+inner host-frame strokes.
        };

        /// \brief Rect geometry used by frame-aware layout helpers.
        struct LayoutRect {
            float x = 0.0f;      ///< Local X offset in current host window coordinates.
            float y = 0.0f;      ///< Local Y offset in current host window coordinates.
            float width = 0.0f;  ///< Rect width in pixels.
            float height = 0.0f; ///< Rect height in pixels.
        };

        /// \brief Compute baseline left inset for title-bar content.
        /// \param style Active Dear ImGui style.
        /// \return Left inset in pixels applied by layout before \ref drawTitleBarText.
        float computeTitleTextLeftInset(const ImGuiStyle& style) const;

        /// \brief Compute baseline left inset for side-panel content.
        /// \param style Active Dear ImGui style.
        /// \return Left inset in pixels applied by layout before \ref drawSidePanel.
        float computeSidePanelContentLeftInset(const ImGuiStyle& style) const;

        /// \brief Resolve effective padding for the framed-window main content area.
        /// \param style Active Dear ImGui style.
        /// \return Per-axis padding where negative config components fall back to `style.WindowPadding`.
        ImVec2 computeMainRegionPadding(const ImGuiStyle& style) const;

        /// \brief Compute nested content region for side-panel widgets.
        /// \param style Active Dear ImGui style.
        /// \param side_panel_width Width of the outer side-panel child.
        /// \return Content region geometry for drawSidePanel().
        SidePanelContentRegion computeSidePanelContentRegion(
            const ImGuiStyle& style,
            float side_panel_width) const;

        /// \brief Compute host-frame stroke insets for the active backend.
        /// \return Insets contributed by outer+inner host-frame strokes.
        HostFrameStrokeInsets computeHostFrameStrokeInsets() const;

        /// \brief Inset a layout rect by host-frame strokes on touched outer edges.
        /// \param x Local X offset in current host window coordinates.
        /// \param y Local Y offset in current host window coordinates.
        /// \param width Rect width in pixels.
        /// \param height Rect height in pixels.
        /// \param touches_left True when the rect touches the left outer host-frame edge.
        /// \param touches_top True when the rect touches the top outer host-frame edge.
        /// \param touches_right True when the rect touches the right outer host-frame edge.
        /// \param touches_bottom True when the rect touches the bottom outer host-frame edge.
        /// \return Frame-aware rect clamped to non-negative size.
        LayoutRect insetRectByHostFrameAdjacency(
            float x,
            float y,
            float width,
            float height,
            bool touches_left,
            bool touches_top,
            bool touches_right,
            bool touches_bottom) const;

        /// \brief Precomputed vertical band used to place control buttons inside title-bar chrome.
        /// \details
        /// The band excludes top frame strokes and, when applicable, bottom separator stroke.
        /// Button styles use this geometry to compute consistent height and vertical centering.
        struct ControlButtonsBand {
            bool separator_context = false; ///< True when corner/side-panel separator affects title-bar band.
            float top_chrome = 0.0f;        ///< Occupied height (px) at top by outer+inner host-frame strokes.
            float bottom_chrome = 0.0f;     ///< Occupied height (px) at bottom by separator stroke when active.
            float usable_h = 0.0f;          ///< Remaining usable title-band height (px) for control buttons.
        };

        /// \brief Compute control-buttons vertical band from current flags and frame stroke config.
        /// \return Filled \ref ControlButtonsBand with separator context and usable title-band height.
        ControlButtonsBand computeControlButtonsBand() const;

        /// \brief Compute vertically centered Y for button height inside a prepared control-buttons band.
        /// \param band Precomputed band returned by \ref computeControlButtonsBand.
        /// \param button_h Final button height in pixels.
        /// \return Pixel-snapped local Y coordinate for stable button placement.
        static float computeCenteredButtonY(const ControlButtonsBand& band, float button_h);

        /// \brief Resolve active control-button style from flags.
        /// \details Expects exactly one style flag (`Classic/ImGui/Mac`) in debug builds.
        /// If multiple are set in release builds, falls back to ImGui style.
        /// \return Resolved control-buttons style.
        ControlButtonsStyle resolveControlButtonsStyle() const;
    };

} // namespace ImGuiX::Windows


#ifdef IMGUIX_HEADER_ONLY
#   ifdef IMGUIX_USE_SFML_BACKEND
#       include "SfmlImGuiFramedWindow.ipp"
#   elif defined(IMGUIX_USE_GLFW_BACKEND)
#       include "GlfwImGuiFramedWindow.ipp"
#   elif defined(IMGUIX_USE_SDL2_BACKEND)
#       include "Sdl2ImGuiFramedWindow.ipp"
#   endif
#include "ImGuiFramedWindow.ipp"
#endif

#endif // _IMGUIX_WINDOWS_IMGUI_FRAMED_WINDOW_HPP_INCLUDED
