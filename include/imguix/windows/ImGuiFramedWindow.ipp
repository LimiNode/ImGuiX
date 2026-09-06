#include <imgui.h>
#include <cassert>
#include <imguix/widgets/controls/circle_button.hpp>
#include <imguix/widgets/controls/system_button.hpp>
#include <imguix/utils/path_utils.hpp>

namespace ImGuiX::Windows {

    ImGuiFramedWindow::ImGuiFramedWindow(
            int id, 
            ApplicationContext& app,
            std::string name,
            std::string title,
            WindowFlags flags, 
            ImGuiFramedWindowConfig config)
        : WindowInstance(id, app, std::move(name)), 
          m_title(std::move(title)), 
          m_flags(flags), 
          m_config(std::move(config)) {}
          
    bool ImGuiFramedWindow::create(int w, int h) {
        m_width = w;
        m_height = h;
        return create();
    }

    bool ImGuiFramedWindow::setWindowIcon(const std::string& path) {
#       ifdef IMGUIX_USE_SFML_BACKEND
        const bool applied = WindowInstance::setWindowIcon(path);
        m_has_corner_icon_texture =
            m_corner_icon_texture.loadFromFile(ImGuiX::Utils::resolveExecPathFs(std::filesystem::u8path(path)));
        if (m_has_corner_icon_texture) {
            m_corner_icon_texture.setSmooth(true);
        }
        return applied;
#       else
        (void)path;
        return false;
#       endif
    }
    
    void ImGuiFramedWindow::setDisableBackground(bool disable) {
        m_disable_background = disable;
    }

    float ImGuiFramedWindow::computeTitleTextLeftInset(const ImGuiStyle& style) const {
        if (m_config.title_content_left_inset >= 0.0f) {
            return m_config.title_content_left_inset;
        }
        const ImVec2 char_size = ImGui::CalcTextSize(u8"W");
        return style.WindowPadding.x * 2.0f + char_size.x;
    }

    float ImGuiFramedWindow::computeSidePanelContentLeftInset(const ImGuiStyle& style) const {
        if (m_config.side_panel_content_left_inset >= 0.0f) {
            return m_config.side_panel_content_left_inset;
        }
        return style.WindowPadding.x * 2.0f;
    }

    ImVec2 ImGuiFramedWindow::computeMainRegionPadding(const ImGuiStyle& style) const {
        return ImVec2(
            m_config.main_region_padding.x >= 0.0f ? m_config.main_region_padding.x : style.WindowPadding.x,
            m_config.main_region_padding.y >= 0.0f ? m_config.main_region_padding.y : style.WindowPadding.y);
    }

    ImGuiFramedWindow::SidePanelContentRegion ImGuiFramedWindow::computeSidePanelContentRegion(
        const ImGuiStyle& style,
        float side_panel_width) const {
        SidePanelContentRegion region{};
        region.x = ImMax(0.0f, computeSidePanelContentLeftInset(style));

        if (m_config.side_panel_content_alignment == SidePanelContentAlignment::SymmetricInset) {
            region.width = ImMax(0.0f, side_panel_width - (region.x * 2.0f));
            return region;
        }

        region.width = ImMax(0.0f, side_panel_width - region.x - ImMax(0.0f, style.WindowPadding.x));
        return region;
    }

    ImGuiFramedWindow::HostFrameStrokeInsets ImGuiFramedWindow::computeHostFrameStrokeInsets() const {
        HostFrameStrokeInsets insets{};
#       ifdef IMGUIX_USE_SFML_BACKEND
        const float total_stroke =
            ImMax(0.0f, m_config.frame_outer_stroke_thickness) +
            ImMax(0.0f, m_config.frame_inner_stroke_thickness);
        insets.left = total_stroke;
        insets.top = total_stroke;
        insets.right = total_stroke;
        insets.bottom = total_stroke;
#       endif
        return insets;
    }

    ImGuiFramedWindow::LayoutRect ImGuiFramedWindow::insetRectByHostFrameAdjacency(
        float x,
        float y,
        float width,
        float height,
        bool touches_left,
        bool touches_top,
        bool touches_right,
        bool touches_bottom) const {
        const HostFrameStrokeInsets insets = computeHostFrameStrokeInsets();
        LayoutRect rect{x, y, width, height};

        if (touches_left) {
            rect.x += insets.left;
            rect.width -= insets.left;
        }
        if (touches_top) {
            rect.y += insets.top;
            rect.height -= insets.top;
        }
        if (touches_right) {
            rect.width -= insets.right;
        }
        if (touches_bottom) {
            rect.height -= insets.bottom;
        }

        rect.width = ImMax(0.0f, rect.width);
        rect.height = ImMax(0.0f, rect.height);
        return rect;
    }

    void ImGuiFramedWindow::drawTitleBarText() {
        const float padding_y = (m_config.title_bar_height - ImGui::GetTextLineHeight()) * 0.5f;
        ImGui::SetCursorPosY(ImMax(0.0f, padding_y));
        ImGui::Text(u8"%s", m_title.c_str());
    }

    void ImGuiFramedWindow::drawCornerIcon() {
#       ifdef IMGUIX_USE_SFML_BACKEND
        if (!m_has_corner_icon_texture) {
            return;
        }

        const ImGuiStyle& style = ImGui::GetStyle();
        const float frame_inset =
            ImMax(0.0f, m_config.frame_outer_stroke_thickness) +
            ImMax(0.0f, m_config.frame_inner_stroke_thickness);
        const float base_inset = ImMax(0.0f, style.WindowPadding.x);
        const float left_inset = base_inset + frame_inset;
        const float top_inset = base_inset + frame_inset;
        const float right_inset = base_inset;
        const float bottom_inset = base_inset;

        const ImVec2 area = ImGui::GetWindowSize();
        const float usable_w = ImMax(0.0f, area.x - left_inset - right_inset);
        const float usable_h = ImMax(0.0f, area.y - top_inset - bottom_inset);
        if (usable_w <= 0.0f || usable_h <= 0.0f) {
            return;
        }
        const float max_icon_size = ImMin(usable_w, usable_h);

        float icon_size = m_config.corner_icon_mode_icon_size >= 0.0f
            ? m_config.corner_icon_mode_icon_size
            : max_icon_size;
        if (max_icon_size < 1.0f) {
            icon_size = max_icon_size;
        } else {
            icon_size = ImClamp(icon_size, 1.0f, max_icon_size);
        }

        const float icon_x = ImFloor(left_inset + ImMax(0.0f, (usable_w - icon_size) * 0.5f) + 0.5f);
        const float icon_y = ImFloor(top_inset + ImMax(0.0f, (usable_h - icon_size) * 0.5f) + 0.5f);
        ImGui::SetCursorPos(ImVec2(icon_x, icon_y));
        ImGui::Image(m_corner_icon_texture, sf::Vector2f(icon_size, icon_size));
#       endif
    }

    void ImGuiFramedWindow::drawControllersContent() {
        for (auto& ctrl : m_controllers) {
            ctrl->drawUi();
        }
    }

    float ImGuiFramedWindow::drawClassicMainMenuRegion(float x, float y, float width, float menu_bar_height) {
        if (!hasFlag(m_flags, WindowFlags::HasMenuBar)) {
            return 0.0f;
        }

        ImGui::SetCursorPos(ImVec2(x, y));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
        if (ImGui::BeginChild(
                u8"##imguix_main_region_menu",
                ImVec2(width, menu_bar_height),
                ImGuiChildFlags_AlwaysUseWindowPadding,
                ImGuiWindowFlags_MenuBar |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoBackground
            )) {
            drawMenuBar();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        return menu_bar_height;
    }

    float ImGuiFramedWindow::drawCornerBelowTitleMenuRegion(float x, float y, float width, float menu_bar_height) {
        if (!hasFlag(m_flags, WindowFlags::HasMenuBar)) {
            return 0.0f;
        }

        ImGui::SetCursorPos(ImVec2(x, y));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
        if (ImGui::BeginChild(
                u8"##imguix_corner_menu_bar",
                ImVec2(width, menu_bar_height),
                ImGuiChildFlags_AlwaysUseWindowPadding,
                ImGuiWindowFlags_MenuBar |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoBackground
            )) {
            drawMenuBar();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        return menu_bar_height;
    }

    void ImGuiFramedWindow::drawCornerInTitleMenuRegion(float menu_bar_height, const ImGuiStyle& style) {
        if (!hasFlag(m_flags, WindowFlags::HasMenuBar)) {
            return;
        }

        const float buttons_reserved = hasFlag(m_flags, WindowFlags::ShowControlButtons)
            ? getControlButtonsReservedWidth()
            : 0.0f;
        const float cursor_after_title = ImGui::GetCursorPosX();
        const ImVec2 last_item_max = ImGui::GetItemRectMax();
        const float last_item_right_local = ImMax(0.0f, last_item_max.x - ImGui::GetWindowPos().x);
        const float menu_start_x = ImMax(cursor_after_title, last_item_right_local) + style.ItemSpacing.x;
        const float menu_y = ImMax(0.0f, (m_config.title_bar_height - menu_bar_height) * 0.5f);
        const float menu_right_limit =
            ImGui::GetWindowWidth() - buttons_reserved - style.WindowPadding.x - style.ItemSpacing.x;
        const float title_menu_width = ImMax(0.0f, menu_right_limit - menu_start_x);
        m_title_bar_interactive_rect = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        if (title_menu_width <= 0.0f) {
            return;
        }

        // The rest of the custom title bar is draggable on Windows (HTCAPTION),
        // but menu items must remain client controls so Dear ImGui receives clicks.
        ImGui::SetCursorPos(ImVec2(menu_start_x, menu_y));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
        if (ImGui::BeginChild(
                u8"##imguix_title_menu_bar",
                ImVec2(title_menu_width, menu_bar_height),
                ImGuiChildFlags_None,
                ImGuiWindowFlags_MenuBar |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoBackground
            )) {
            const ImVec2 menu_window_pos = ImGui::GetWindowPos();
            m_title_bar_interactive_rect = ImVec4(
                menu_window_pos.x,
                menu_window_pos.y,
                menu_window_pos.x + title_menu_width,
                menu_window_pos.y + menu_bar_height);
            ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
            drawMenuBar();
            ImGui::PopStyleColor(2);
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

    void ImGuiFramedWindow::drawClassicLayout(float menu_bar_height) {
        const ImGuiStyle& style = ImGui::GetStyle();
        const ImVec2 host_size = ImGui::GetWindowSize();
        const float body_y = static_cast<float>(m_config.title_bar_height);
        const float body_h = ImMax(0.0f, host_size.y - body_y);
        const ImVec2 body_start(0.0f, body_y);
        const float body_width = ImMax(0.0f, host_size.x);
        const float requested_side_panel_width =
            m_config.side_panel_width > 0 ? static_cast<float>(m_config.side_panel_width) : 0.0f;
        const float side_panel_width = ImMin(requested_side_panel_width, body_width);
        const float main_region_width = ImMax(0.0f, body_width - side_panel_width);
        const float stroke = ImMax(0.0f, m_config.frame_stroke_thickness);
        const float half_stroke = stroke * 0.5f;

        ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::BeginChild(
            u8"##imguix_title_bar",
            ImVec2(body_width, static_cast<float>(m_config.title_bar_height)),
            ImGuiChildFlags_None,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoBackground);
        {
            const ImVec2 p_min = ImGui::GetWindowPos();
            const ImVec2 p_max = ImVec2(p_min.x + ImGui::GetWindowWidth(), p_min.y + m_config.title_bar_height);
            ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, ImGui::GetColorU32(ImGuiCol_TitleBgActive));
        }

        ImGui::SetCursorPosX(ImMax(ImGui::GetCursorPosX(), computeTitleTextLeftInset(ImGui::GetStyle())));
        drawTitleBarText();

        if (hasFlag(m_flags, WindowFlags::ShowControlButtons)) {
            switch (resolveControlButtonsStyle()) {
                case ControlButtonsStyle::Mac:
                    drawMacStyledControlButtons();
                    break;
                case ControlButtonsStyle::ImGui:
                    drawImGuiStyledControlButtons();
                    break;
                case ControlButtonsStyle::Classic:
                default:
                    drawControlButtons();
                    break;
            }
        }

        if (side_panel_width > 0.0f && stroke > 0.0f) {
            const ImVec2 p_min = ImGui::GetWindowPos();
            const ImVec2 p_max = ImVec2(p_min.x + ImGui::GetWindowWidth(), p_min.y + m_config.title_bar_height);
            const ImU32 sep_col = ImGui::GetColorU32(ImGuiCol_Border);
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(p_min.x, p_max.y - half_stroke),
                ImVec2(p_max.x, p_max.y - half_stroke),
                sep_col,
                stroke);
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();

        if (side_panel_width <= 0.0f) {
            const LayoutRect main_menu_rect = insetRectByHostFrameAdjacency(
                0.0f,
                body_y,
                body_width,
                menu_bar_height,
                true,
                false,
                true,
                false);
            const float main_menu_height = drawClassicMainMenuRegion(
                main_menu_rect.x,
                main_menu_rect.y,
                main_menu_rect.width,
                main_menu_rect.height);
            const LayoutRect main_region_rect = insetRectByHostFrameAdjacency(
                0.0f,
                body_y + main_menu_height,
                body_width,
                ImMax(0.0f, body_h - main_menu_height),
                true,
                false,
                true,
                true);
            const ImVec2 main_region_padding = computeMainRegionPadding(style);
            ImGui::SetCursorPos(ImVec2(main_region_rect.x, main_region_rect.y));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, main_region_padding);
            if (ImGui::BeginChild(
                    u8"##imguix_main_region",
                    ImVec2(main_region_rect.width, main_region_rect.height),
                    ImGuiChildFlags_AlwaysUseWindowPadding,
                    ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_NoDecoration |
                    ImGuiWindowFlags_NoBackground
                )) {
                drawControllersContent();
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
            return;
        }

        ImGui::SetCursorPos(body_start);
        if (ImGui::BeginChild(
                u8"##imguix_side_panel",
                ImVec2(side_panel_width, body_h),
                ImGuiChildFlags_AlwaysUseWindowPadding,
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoBackground
            )) {
            const ImVec2 p_min = ImGui::GetWindowPos();
            const ImVec2 p_max = ImVec2(p_min.x + ImGui::GetWindowWidth(), p_min.y + ImGui::GetWindowHeight());
            ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, ImGui::GetColorU32(ImGuiCol_TitleBgActive));
            if (stroke > 0.0f) {
                const ImU32 sep_col = ImGui::GetColorU32(ImGuiCol_Border);
                ImGui::GetWindowDrawList()->AddLine(
                    ImVec2(p_max.x - half_stroke, p_min.y),
                    ImVec2(p_max.x - half_stroke, p_max.y),
                    sep_col,
                    stroke);
            }
            const SidePanelContentRegion content_region =
                computeSidePanelContentRegion(style, side_panel_width);
            if (content_region.width > 0.0f) {
                const float content_y = ImGui::GetCursorPosY();
                const float content_h = ImMax(0.0f, ImGui::GetContentRegionAvail().y);
                ImGui::SetCursorPos(ImVec2(content_region.x, content_y));
                if (ImGui::BeginChild(
                        u8"##imguix_side_panel_content",
                        ImVec2(content_region.width, content_h),
                        ImGuiChildFlags_None,
                        ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoDecoration |
                        ImGuiWindowFlags_NoBackground
                    )) {
                    drawSidePanel();
                }
                ImGui::EndChild();
            }
        }
        ImGui::EndChild();

        const float main_region_x = side_panel_width;
        const LayoutRect main_menu_rect = insetRectByHostFrameAdjacency(
            main_region_x,
            body_y,
            main_region_width,
            menu_bar_height,
            false,
            false,
            true,
            false);
        const float main_menu_height = drawClassicMainMenuRegion(
            main_menu_rect.x,
            main_menu_rect.y,
            main_menu_rect.width,
            main_menu_rect.height);
        const LayoutRect main_region_rect = insetRectByHostFrameAdjacency(
            main_region_x,
            body_y + main_menu_height,
            main_region_width,
            ImMax(0.0f, body_h - main_menu_height),
            false,
            false,
            true,
            true);
        const ImVec2 main_region_padding = computeMainRegionPadding(style);
        ImGui::SetCursorPos(ImVec2(main_region_rect.x, main_region_rect.y));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, main_region_padding);
        if (ImGui::BeginChild(
                u8"##imguix_main_region",
                ImVec2(main_region_rect.width, main_region_rect.height),
                ImGuiChildFlags_AlwaysUseWindowPadding,
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoBackground
            )) {
            drawControllersContent();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

    void ImGuiFramedWindow::drawCornerLayout(float menu_bar_height) {
        const ImGuiStyle& style = ImGui::GetStyle();
        const ImVec2 host_size = ImGui::GetWindowSize();
        const float body_y = static_cast<float>(m_config.title_bar_height);
        const float body_h = ImMax(0.0f, host_size.y - body_y);
        const float body_width = ImMax(0.0f, host_size.x);
        const float icon_slot = static_cast<float>(m_config.title_bar_height);
        const float corner_gap = ImMax(
            0.0f,
            m_config.corner_icon_mode_gap >= 0.0f
                ? m_config.corner_icon_mode_gap
                : style.WindowPadding.x);
        const float icon_area_width = m_config.corner_icon_mode_area_width >= 0.0f
            ? m_config.corner_icon_mode_area_width
            : icon_slot + corner_gap;
        const float icon_surface_w = ImMin(body_width, ImMax(0.0f, icon_area_width));
        const float requested_side_panel_width =
            m_config.side_panel_width > 0 ? static_cast<float>(m_config.side_panel_width) : icon_surface_w;
        const float side_panel_width = ImMin(requested_side_panel_width, body_width);
        const float main_region_width = ImMax(0.0f, body_width - side_panel_width);
        const float side_h = ImMax(0.0f, body_h - corner_gap);
        const bool has_corner_menu = hasFlag(m_flags, WindowFlags::HasMenuBar);
        const bool menu_in_title =
            has_corner_menu && m_config.corner_menu_bar_placement == CornerMenuBarPlacement::InTitleBar;
        const bool menu_below_title =
            has_corner_menu && m_config.corner_menu_bar_placement == CornerMenuBarPlacement::BelowTitleBar;
        const bool menu_main_region =
            has_corner_menu && m_config.corner_menu_bar_placement == CornerMenuBarPlacement::MainRegion;

        ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
        if (ImGui::BeginChild(
                u8"##imguix_corner_icon",
                ImVec2(icon_surface_w, m_config.title_bar_height),
                ImGuiChildFlags_None,
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoBackground
            )) {
            drawCornerIcon();
        }
        ImGui::EndChild();

        ImGui::SetCursorPos(ImVec2(icon_surface_w, 0.0f));
        const float stroke = ImMax(0.0f, m_config.frame_stroke_thickness);
        const float title_w = ImMax(0.0f, body_width - icon_surface_w);
        const bool enable_rounding = hasFlag(m_flags, WindowFlags::CornerModeRounding);
        const float rounding = enable_rounding ? m_config.corner_icon_mode_rounding_radius : 0.0f;
        const bool no_top_left_corner = enable_rounding &&
            m_config.corner_rounding_style == CornerRoundingStyle::NoTopLeftOnTitleAndSide;
        const ImDrawFlags title_rounding_flags = enable_rounding
            ? (no_top_left_corner
                ? ImDrawFlags_RoundCornersBottomLeft
                : (ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomLeft))
            : ImDrawFlags_None;
        const ImDrawFlags side_rounding_flags = enable_rounding
            ? (no_top_left_corner
                ? ImDrawFlags_RoundCornersTopRight
                : (ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight))
            : ImDrawFlags_None;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::BeginChild(
            u8"##imguix_title_bar",
            ImVec2(title_w, m_config.title_bar_height),
            ImGuiChildFlags_None,
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoBackground);
        {
            const ImVec2 p_min = ImGui::GetWindowPos();
            const ImVec2 p_max = ImVec2(p_min.x + ImGui::GetWindowWidth(), p_min.y + m_config.title_bar_height);
            ImGui::GetWindowDrawList()->AddRectFilled(
                p_min,
                p_max,
                ImGui::GetColorU32(ImGuiCol_TitleBgActive),
                rounding,
                title_rounding_flags);
        }

        ImGui::SetCursorPosX(ImMax(ImGui::GetCursorPosX(), computeTitleTextLeftInset(ImGui::GetStyle())));
        drawTitleBarText();
        if (menu_in_title) {
            drawCornerInTitleMenuRegion(menu_bar_height, style);
        }

        if (hasFlag(m_flags, WindowFlags::ShowControlButtons)) {
            switch (resolveControlButtonsStyle()) {
                case ControlButtonsStyle::Mac:
                    drawMacStyledControlButtons();
                    break;
                case ControlButtonsStyle::ImGui:
                    drawImGuiStyledControlButtons();
                    break;
                case ControlButtonsStyle::Classic:
                default:
                    drawControlButtons();
                    break;
            }
        }

        if (hasFlag(m_flags, WindowFlags::CornerModeBorder) && stroke > 0.0f) {
            const ImVec2 p_min = ImGui::GetWindowPos();
            const ImVec2 p_max = ImVec2(p_min.x + ImGui::GetWindowWidth(), p_min.y + m_config.title_bar_height);
            const ImU32 sep_col = ImGui::GetColorU32(ImGuiCol_Border);
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            const float clip_right = ImMax(p_min.x, p_max.x - stroke);
            draw_list->PushClipRect(p_min, ImVec2(clip_right, p_max.y), true);
            draw_list->AddRect(
                p_min,
                p_max,
                sep_col,
                rounding,
                title_rounding_flags,
                stroke);
            draw_list->PopClipRect();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();

        if (side_panel_width <= 0.0f) {
            float menu_height = 0.0f;
            if (menu_below_title) {
                const LayoutRect corner_menu_rect = insetRectByHostFrameAdjacency(
                    0.0f,
                    body_y,
                    body_width,
                    menu_bar_height,
                    true,
                    false,
                    true,
                    false);
                menu_height = drawCornerBelowTitleMenuRegion(
                    corner_menu_rect.x,
                    corner_menu_rect.y,
                    corner_menu_rect.width,
                    corner_menu_rect.height);
            } else if (menu_main_region) {
                const LayoutRect main_menu_rect = insetRectByHostFrameAdjacency(
                    0.0f,
                    body_y,
                    body_width,
                    menu_bar_height,
                    true,
                    false,
                    true,
                    false);
                menu_height = drawClassicMainMenuRegion(
                    main_menu_rect.x,
                    main_menu_rect.y,
                    main_menu_rect.width,
                    main_menu_rect.height);
            }
            const LayoutRect main_region_rect = insetRectByHostFrameAdjacency(
                0.0f,
                body_y + menu_height,
                body_width,
                ImMax(0.0f, body_h - menu_height),
                true,
                false,
                true,
                true);
            const ImVec2 main_region_padding = computeMainRegionPadding(style);
            ImGui::SetCursorPos(ImVec2(main_region_rect.x, main_region_rect.y));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, main_region_padding);
            if (ImGui::BeginChild(
                    u8"##imguix_main_region",
                    ImVec2(main_region_rect.width, main_region_rect.height),
                    ImGuiChildFlags_AlwaysUseWindowPadding,
                    ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_NoDecoration |
                    ImGuiWindowFlags_NoBackground
                )) {
                drawControllersContent();
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
            return;
        }

        ImGui::SetCursorPos(ImVec2(0.0f, body_y + corner_gap));
        if (ImGui::BeginChild(
                u8"##imguix_side_panel",
                ImVec2(side_panel_width, side_h),
                ImGuiChildFlags_AlwaysUseWindowPadding,
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoBackground
            )) {
            const ImVec2 p_min = ImGui::GetWindowPos();
            const ImVec2 p_max = ImVec2(p_min.x + ImGui::GetWindowWidth(), p_min.y + ImGui::GetWindowHeight());
            ImGui::GetWindowDrawList()->AddRectFilled(
                p_min,
                p_max,
                ImGui::GetColorU32(ImGuiCol_TitleBgActive),
                rounding,
                side_rounding_flags);
            if (hasFlag(m_flags, WindowFlags::CornerModeBorder) && stroke > 0.0f) {
                const ImU32 sep_col = ImGui::GetColorU32(ImGuiCol_Border);
                ImGui::GetWindowDrawList()->AddRect(
                    p_min,
                    p_max,
                    sep_col,
                    rounding,
                    side_rounding_flags,
                    stroke);
            }
            const SidePanelContentRegion content_region =
                computeSidePanelContentRegion(style, side_panel_width);
            if (content_region.width > 0.0f) {
                const float content_y = ImGui::GetCursorPosY();
                const float content_h = ImMax(0.0f, ImGui::GetContentRegionAvail().y);
                ImGui::SetCursorPos(ImVec2(content_region.x, content_y));
                if (ImGui::BeginChild(
                        u8"##imguix_side_panel_content",
                        ImVec2(content_region.width, content_h),
                        ImGuiChildFlags_None,
                        ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoDecoration |
                        ImGuiWindowFlags_NoBackground
                    )) {
                    drawSidePanel();
                }
                ImGui::EndChild();
            }
        }
        ImGui::EndChild();

        const float main_region_x = side_panel_width;
        const float corner_menu_height = [&]() {
            if (!menu_below_title) {
                return 0.0f;
            }
            const LayoutRect corner_menu_rect = insetRectByHostFrameAdjacency(
                main_region_x,
                body_y,
                main_region_width,
                menu_bar_height,
                false,
                false,
                true,
                false);
            return drawCornerBelowTitleMenuRegion(
                corner_menu_rect.x,
                corner_menu_rect.y,
                corner_menu_rect.width,
                corner_menu_rect.height);
        }();
        const float main_menu_height = [&]() {
            if (!menu_main_region) {
                return 0.0f;
            }
            const LayoutRect main_menu_rect = insetRectByHostFrameAdjacency(
                main_region_x,
                body_y,
                main_region_width,
                menu_bar_height,
                false,
                false,
                true,
                false);
            return drawClassicMainMenuRegion(
                main_menu_rect.x,
                main_menu_rect.y,
                main_menu_rect.width,
                main_menu_rect.height);
        }();
        const LayoutRect main_region_rect = insetRectByHostFrameAdjacency(
            main_region_x,
            body_y + corner_menu_height + main_menu_height,
            main_region_width,
            ImMax(0.0f, body_h - corner_menu_height - main_menu_height),
            false,
            false,
            true,
            true);
        const ImVec2 main_region_padding = computeMainRegionPadding(style);

        ImGui::SetCursorPos(ImVec2(main_region_rect.x, main_region_rect.y));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, main_region_padding);
        if (ImGui::BeginChild(
                u8"##imguix_main_region",
                ImVec2(main_region_rect.width, main_region_rect.height),
                ImGuiChildFlags_AlwaysUseWindowPadding,
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoBackground
            )) {
            drawControllersContent();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

    // Compact control-buttons geometry is enabled only for corner chrome with explicit border.
    // In this mode the button strip is slightly reduced/inset to align with the custom border.
    bool ImGuiFramedWindow::isCompactControlButtonsMode() const {
        return hasFlag(m_flags, WindowFlags::HasCornerIconArea) &&
            hasFlag(m_flags, WindowFlags::CornerModeBorder);
    }

    // Per-button size reduction (in px) used by all button styles in compact mode.
    float ImGuiFramedWindow::getControlButtonsCompactDelta() const {
        return isCompactControlButtonsMode() ? 2.0f : 0.0f;
    }

    // Additional right-side inset (in px) that keeps the rightmost button off the border stroke.
    float ImGuiFramedWindow::getControlButtonsRightInset() const {
        return isCompactControlButtonsMode() ? 1.0f : 0.0f;
    }

    ImGuiFramedWindow::ControlButtonsBand ImGuiFramedWindow::computeControlButtonsBand() const {
        ControlButtonsBand band{};
        band.separator_context =
            hasFlag(m_flags, WindowFlags::HasCornerIconArea) || m_config.side_panel_width > 0;
        band.top_chrome =
            ImMax(0.0f, m_config.frame_outer_stroke_thickness) +
            ImMax(0.0f, m_config.frame_inner_stroke_thickness);
        band.bottom_chrome = band.separator_context ? ImMax(0.0f, m_config.frame_stroke_thickness) : 0.0f;
        band.usable_h =
            ImMax(0.0f, static_cast<float>(m_config.title_bar_height) - band.top_chrome - band.bottom_chrome);
        return band;
    }

    float ImGuiFramedWindow::computeCenteredButtonY(const ControlButtonsBand& band, float button_h) {
        const float y = band.top_chrome + ImMax(0.0f, (band.usable_h - button_h) * 0.5f);
        return ImFloor(y + 0.5f);
    }

    ImGuiFramedWindow::ControlButtonsStyle ImGuiFramedWindow::resolveControlButtonsStyle() const {
        const bool classic = hasFlag(m_flags, WindowFlags::ClassicStyledControlButtons);
        const bool imgui = hasFlag(m_flags, WindowFlags::ImGuiStyledControlButtons);
        const bool mac = hasFlag(m_flags, WindowFlags::MacStyledControlButtons);
        const int selected_styles = static_cast<int>(classic) + static_cast<int>(imgui) + static_cast<int>(mac);

        assert(selected_styles <= 1 && "ImGuiFramedWindow: control button style flags are mutually exclusive");
        if (selected_styles > 1) {
            return ControlButtonsStyle::ImGui;
        }
        if (mac) {
            return ControlButtonsStyle::Mac;
        }
        if (imgui) {
            return ControlButtonsStyle::ImGui;
        }
        return ControlButtonsStyle::Classic;
    }

    float ImGuiFramedWindow::getControlButtonsReservedWidth() const {
        const ImGuiStyle& style = ImGui::GetStyle();
        const ControlButtonsStyle control_style = resolveControlButtonsStyle();
        // Compact mode slightly shrinks each button to visually fit corner-border chrome.
        const float compact_delta = getControlButtonsCompactDelta();
        const ControlButtonsBand band = computeControlButtonsBand();
        const float separator_stroke = ImMax(0.0f, m_config.frame_stroke_thickness);
        const float outer_frame_stroke = ImMax(0.0f, m_config.frame_outer_stroke_thickness);
        const float inner_frame_stroke = ImMax(0.0f, m_config.frame_inner_stroke_thickness);
        const float right_gap = outer_frame_stroke + inner_frame_stroke;

        if (control_style == ControlButtonsStyle::Mac) {
            const float usable_h = band.usable_h > 0.0f
                ? band.usable_h
                : ImMax(0.0f, static_cast<float>(m_config.title_bar_height) - outer_frame_stroke - inner_frame_stroke);
            float btn_diameter = ImFloor(usable_h * 0.45f + 0.5f);
            btn_diameter = ImClamp(btn_diameter, 12.0f, 16.0f);
            btn_diameter = ImMax(8.0f, btn_diameter - compact_delta * 0.5f);
            if (band.usable_h > 0.0f) {
                btn_diameter = ImMin(btn_diameter, band.usable_h);
            }
            btn_diameter = ImMax(1.0f, btn_diameter);
            const float btn_spacing = ImMax(3.0f, ImFloor(btn_diameter * 0.35f + 0.5f));
            const float right_clearance =
                right_gap + getControlButtonsRightInset() + ImMax(2.0f, ImFloor(btn_spacing * 0.5f));
            return btn_diameter * 3.0f + btn_spacing * 2.0f + right_clearance;
        }

        if (control_style == ControlButtonsStyle::ImGui) {
            float btn_height = m_config.title_bar_height - outer_frame_stroke - inner_frame_stroke;
            if (band.separator_context) {
                // Corner/side-panel layouts add an explicit separator line around title chrome.
                btn_height -= separator_stroke;
            }
            btn_height = ImMax(8.0f, btn_height);
            if (band.usable_h > 0.0f) {
                btn_height = ImMin(btn_height, band.usable_h);
            }
            btn_height = ImMax(1.0f, btn_height);
            return btn_height * 3.0f + right_gap;
        }

        const ImVec2 char_size = ImGui::CalcTextSize(u8"W");
        float btn_width = char_size.x * 3.0f + style.FramePadding.x * 2.0f;
        btn_width = ImMax(char_size.x * 2.0f, btn_width - compact_delta);
        return style.ItemSpacing.x * 2.0f + btn_width * 3.0f + right_gap;
    }

    void ImGuiFramedWindow::drawControlButtons() {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        const ImGuiStyle& style = ImGui::GetStyle();
        const ControlButtonsBand band = computeControlButtonsBand();
        // Apply compact-mode reduction to button width/height when corner-border chrome is active.
        const float compact_delta = getControlButtonsCompactDelta();
        const ImVec2 char_size = ImGui::CalcTextSize(u8"W");
        float btn_width = char_size.x * 3.0f + style.FramePadding.x * 2.0f;
        float btn_height = char_size.y;
        btn_width = ImMax(char_size.x * 2.0f, btn_width - compact_delta);
        btn_height = ImMax(char_size.y * 0.8f, btn_height - compact_delta);
        if (band.usable_h > 0.0f) {
            btn_height = ImMin(btn_height, band.usable_h);
        }
        btn_height = ImMax(1.0f, btn_height);
        const float btn_padding = getControlButtonsReservedWidth();
        ImVec2 btn_size(btn_width, btn_height);

        ImVec2 btn_min, btn_max;
        const float center_y = computeCenteredButtonY(band, btn_size.y);
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - btn_padding, center_y));
        if (ImGui::Button(m_config.minimize_button_text, btn_size)) {
            minimize();
        }

        btn_min = ImGui::GetItemRectMin();
        btn_max = ImGui::GetItemRectMax();
#       ifdef _WIN32
        m_minimize_btn_rect = {
            static_cast<LONG>(btn_min.x),
            static_cast<LONG>(btn_min.y),
            static_cast<LONG>(btn_max.x),
            static_cast<LONG>(btn_max.y)
        };
#       endif
        
        ImGui::SameLine();
        if (ImGui::Button(m_config.maximize_button_text, btn_size)) {
            toggleMaximizeRestore();
        }
        
        btn_min = ImGui::GetItemRectMin();
        btn_max = ImGui::GetItemRectMax();
#       ifdef _WIN32
        m_maximize_btn_rect = {
            static_cast<LONG>(btn_min.x),
            static_cast<LONG>(btn_min.y),
            static_cast<LONG>(btn_max.x),
            static_cast<LONG>(btn_max.y)
        };
#       endif

        ImGui::SameLine();
        if (ImGui::Button(m_config.close_button_text, btn_size)) {
            close();
        }

        btn_min = ImGui::GetItemRectMin();
        btn_max = ImGui::GetItemRectMax();
#       ifdef _WIN32
        m_close_btn_rect = {
            static_cast<LONG>(btn_min.x),
            static_cast<LONG>(btn_min.y),
            static_cast<LONG>(btn_max.x),
            static_cast<LONG>(btn_max.y)
        };
#       endif

        ImGui::PopStyleVar();
    }
    
    void ImGuiFramedWindow::drawMacStyledControlButtons() {
        const ControlButtonsBand band = computeControlButtonsBand();
        const float compact_delta = getControlButtonsCompactDelta();
        const float outer_frame_stroke = ImMax(0.0f, m_config.frame_outer_stroke_thickness);
        const float inner_frame_stroke = ImMax(0.0f, m_config.frame_inner_stroke_thickness);
        const float usable_h = band.usable_h > 0.0f
            ? band.usable_h
            : ImMax(0.0f, static_cast<float>(m_config.title_bar_height) - outer_frame_stroke - inner_frame_stroke);

        float btn_diameter = ImFloor(usable_h * 0.45f + 0.5f);
        btn_diameter = ImClamp(btn_diameter, 12.0f, 16.0f);
        btn_diameter = ImMax(8.0f, btn_diameter - compact_delta * 0.5f);
        if (band.usable_h > 0.0f) {
            btn_diameter = ImMin(btn_diameter, band.usable_h);
        }
        btn_diameter = ImMax(1.0f, btn_diameter);
        const float btn_spacing = ImMax(3.0f, ImFloor(btn_diameter * 0.35f + 0.5f));
        const float btn_padding = getControlButtonsReservedWidth();
        ImVec2 btn_size(btn_diameter, btn_diameter);

        ImVec2 btn_min, btn_max;
        const float center_y = computeCenteredButtonY(band, btn_size.y);
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - btn_padding, center_y));
        if (Widgets::CircleButton(u8"##imguix_btn_minimize", btn_diameter, ImVec4(1.0f, 0.74f, 0.18f, 1.0f))) {
            minimize();
        }

        btn_min = ImGui::GetItemRectMin();
        btn_max = ImGui::GetItemRectMax();
#       ifdef _WIN32
        m_minimize_btn_rect = {
            static_cast<LONG>(btn_min.x),
            static_cast<LONG>(btn_min.y),
            static_cast<LONG>(btn_max.x),
            static_cast<LONG>(btn_max.y)
        };
#       endif
        
        ImGui::SameLine(0.0f, btn_spacing);
        if (Widgets::CircleButton(u8"##imguix_btn_maximize", btn_diameter, ImVec4(0.15f, 0.79f, 0.25f, 1.0f))) {
            toggleMaximizeRestore();
        }

        btn_min = ImGui::GetItemRectMin();
        btn_max = ImGui::GetItemRectMax();
#       ifdef _WIN32
        m_maximize_btn_rect = {
            static_cast<LONG>(btn_min.x),
            static_cast<LONG>(btn_min.y),
            static_cast<LONG>(btn_max.x),
            static_cast<LONG>(btn_max.y)
        };
#       endif

        ImGui::SameLine(0.0f, btn_spacing);
        if (Widgets::CircleButton(u8"##imguix_btn_close", btn_diameter, ImVec4(1.0f, 0.0f, 0.0f, 1.0f))) {
            close();
        }
 
        btn_min = ImGui::GetItemRectMin();
        btn_max = ImGui::GetItemRectMax();
#       ifdef _WIN32
        m_close_btn_rect = {
            static_cast<LONG>(btn_min.x),
            static_cast<LONG>(btn_min.y),
            static_cast<LONG>(btn_max.x),
            static_cast<LONG>(btn_max.y)
        };
#       endif
    }

    void ImGuiFramedWindow::drawImGuiStyledControlButtons() {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        const ControlButtonsBand band = computeControlButtonsBand();
        const float separator_stroke = ImMax(0.0f, m_config.frame_stroke_thickness);
        // Corner/side-panel layouts add separator strokes around title chrome, so buttons keep extra clearance.
        const float separator_compensation = band.separator_context ? separator_stroke : 0.0f;
        const float frame_compensation = band.top_chrome;

        const float btn_height_raw = m_config.title_bar_height - frame_compensation - separator_compensation;
        float btn_height = ImMax(8.0f, btn_height_raw);
        if (band.usable_h > 0.0f) {
            btn_height = ImMin(btn_height, band.usable_h);
        }
        btn_height = ImMax(1.0f, btn_height);
        const float btn_padding = getControlButtonsReservedWidth();

        ImVec2 btn_size(btn_height, btn_height);
        ImVec2 btn_min, btn_max;
        const float center_y = computeCenteredButtonY(band, btn_size.y);
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - btn_padding, center_y));
        if (Widgets::SystemButton(u8"##imguix_btn_minimize", Widgets::SystemButtonType::Minimize, btn_size)) {
            minimize();
        }

        btn_min = ImGui::GetItemRectMin();
        btn_max = ImGui::GetItemRectMax();
#       ifdef _WIN32
        m_minimize_btn_rect = {
            static_cast<LONG>(btn_min.x),
            static_cast<LONG>(btn_min.y),
            static_cast<LONG>(btn_max.x),
            static_cast<LONG>(btn_max.y)
        };
#       endif
        
        ImGui::SameLine(0.0f, 0.0f);
        if (Widgets::SystemButton(u8"##imguix_btn_maximize", Widgets::SystemButtonType::Maximize, btn_size)) {
            toggleMaximizeRestore();
        }

        btn_min = ImGui::GetItemRectMin();
        btn_max = ImGui::GetItemRectMax();
#       ifdef _WIN32
        m_maximize_btn_rect = {
            static_cast<LONG>(btn_min.x),
            static_cast<LONG>(btn_min.y),
            static_cast<LONG>(btn_max.x),
            static_cast<LONG>(btn_max.y)
        };
#       endif

        ImGui::SameLine(0.0f, 0.0f);
        if (Widgets::SystemButton(u8"##imguix_btn_close", Widgets::SystemButtonType::Close, btn_size)) {
            close();
        }
 
        btn_min = ImGui::GetItemRectMin();
        btn_max = ImGui::GetItemRectMax();
#       ifdef _WIN32
        m_close_btn_rect = {
            static_cast<LONG>(btn_min.x),
            static_cast<LONG>(btn_min.y),
            static_cast<LONG>(btn_max.x),
            static_cast<LONG>(btn_max.y)
        };
#       endif

        ImGui::PopStyleVar();
    }

    void ImGuiFramedWindow::renderFrameManually() {
#       ifdef _WIN32
        if (!m_in_manual_sizing) return;
#       endif
        tick();
        drawUi();
        drawContent();
        present();
    }

} // namespace ImGuiX::Windows
