#include <vector>
#include <string>
#include <chrono>
#include <functional>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <utility>
#include <imgui_internal.h>

#include <imguix/widgets/controls/system_button.hpp>

namespace ImGuiX::Notify {

    inline void NotificationManager::render() {
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        const ImVec2 wrap_ref = vp->Size;
        const float wrap_width = wrap_ref.x * m_cfg.wrap_width_frac;
        float height = 0.0f;

        for (std::size_t i = 0; i < m_notifications.size(); ++i) {
            Notification& toast = m_notifications[i];
            
            const char* content = toast.content();
            int wrapped_lines = 1;
            if (content && *content) {
                // Высота текста при wrap и делим на высоту строки
                if (m_fonts.text) ImGui::PushFont(m_fonts.text);
                const ImVec2 sz = ImGui::CalcTextSize(content, nullptr, false, wrap_width);
                if (m_fonts.text) ImGui::PopFont();
                wrapped_lines = std::max(1, static_cast<int>(std::ceil(sz.y / ImGui::GetTextLineHeight())));
            }
            
            const int eff_dismiss = computeEffectiveDismissMs(toast, m_cfg, wrap_width, wrapped_lines);

            if (toast.phase(m_cfg, eff_dismiss) == Phase::Expired) { remove(i); --i; continue; }
            if (m_cfg.render_limit > 0 && i >= static_cast<std::size_t>(m_cfg.render_limit)) continue;

            const float opacity = toast.fadePercent(m_cfg, eff_dismiss);
            ImVec4 icon_color = toast.color();
            if (!toast.hasColorOverride()) {
                switch (toast.type()) {
                    case Type::Success: icon_color = m_style.icon_colors.success; break;
                    case Type::Warning: icon_color = m_style.icon_colors.warning; break;
                    case Type::Error:   icon_color = m_style.icon_colors.error;   break;
                    case Type::Info:    icon_color = m_style.icon_colors.info;    break;
                    default:            icon_color = m_style.icon_colors.none;    break;
                }
            }
            icon_color.w *= opacity;
            ImVec4 text_color = m_style.text;
            text_color.w *= opacity;
            const ImVec4 border_color = m_style.border_use_icon_color
                ? icon_color
                : m_style.border;

            char window_name[48];
            std::snprintf(window_name, sizeof(window_name), "###toast%u", i);
            ImGui::SetNextWindowBgAlpha(opacity);

            const ImVec2 work_pos  = m_cfg.outside_main_window ? vp->WorkPos : vp->Pos;
            const ImVec2 work_size = m_cfg.outside_main_window ? vp->WorkSize : vp->Size;

            ImGui::SetNextWindowPos(
                ImVec2(work_pos.x + work_size.x - m_cfg.padding_x,
                       work_pos.y + work_size.y - m_cfg.padding_y - height),
                ImGuiCond_Always, ImVec2(1.0f, 1.0f));

            ImGuiWindowFlags wflags = toast.windowFlags(m_cfg);
            if (!m_cfg.use_dismiss_button && !toast.onButtonPress())
                wflags |= ImGuiWindowFlags_NoInputs;

            if (m_fonts.text) ImGui::PushFont(m_fonts.text);

            ImGui::PushStyleColor(ImGuiCol_WindowBg, m_style.background);
            ImGui::PushStyleColor(ImGuiCol_PopupBg, m_style.background);
            ImGui::PushStyleColor(ImGuiCol_Border, border_color);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, m_style.border_thickness);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, m_style.rounding);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, m_style.padding);

            ImGui::Begin(window_name, nullptr, wflags);
            ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
            ImGui::PushTextWrapPos(wrap_ref.x * m_cfg.wrap_width_frac);
            
            ImGui::PushID(static_cast<int>(i));
            ImGui::PushStyleColor(ImGuiCol_Text, text_color);
            
            if (m_cfg.pause_on_hover) {
                const ImVec2 p0 = ImGui::GetWindowPos();
                const ImVec2 ws = ImGui::GetWindowSize();
                const ImVec2 p1(p0.x + ws.x, p0.y + ws.y);
                const bool hovered = ImGui::IsMouseHoveringRect(p0, p1, /*clip*/false);
                toast.updateHover(hovered);
            }

            const char* icon = nullptr;
            switch (toast.type()) {
                case Type::Success: icon = m_icons.icon_success; break;
                case Type::Warning: icon = m_icons.icon_warning; break;
                case Type::Error:   icon = m_icons.icon_error;   break;
                case Type::Info:    icon = m_icons.icon_info;    break;
                default: break;
            }

            bool title_rendered = false;
            if (icon && icon[0]) {
                if (m_fonts.icons) ImGui::PushFont(m_fonts.icons);
                ImGui::PushStyleColor(ImGuiCol_Text, icon_color);
                ImGui::TextUnformatted(icon);
                ImGui::PopStyleColor();
                if (m_fonts.icons) ImGui::PopFont();
                title_rendered = true;
            }

            if (const char* t = toast.title(); t && t[0]) {
                if (title_rendered) ImGui::SameLine();
                ImGui::TextUnformatted(t);
                title_rendered = true;
            } else 
            if (const char* dt = toast.defaultTitle()) {
                if (title_rendered) ImGui::SameLine();
                ImGui::TextUnformatted(dt);
                title_rendered = true;
            }

            // Close button (square)
            if (m_cfg.use_dismiss_button && m_icons.icon_close) {
                if (title_rendered || (toast.content() && toast.content()[0])) { 
                    ImGui::SameLine();
                }
                
                const ImGuiStyle& style = ImGui::GetStyle();

                const float btn_w = ImFloor(ImGui::GetFrameHeight() - 2.0f);
                const float right_margin = (m_cfg.close_btn_right_margin >= 0.0f)
                         ? m_cfg.close_btn_right_margin
                         : style.FramePadding.x;
                         
                const float avail = ImGui::GetContentRegionAvail().x;
                const float cur_x = ImGui::GetCursorPosX();

                ImGui::SetCursorPosX(cur_x + std::max(0.0f, avail - btn_w - right_margin));
                ImGui::PushStyleColor(ImGuiCol_Button, m_style.close_button_background);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_style.close_button_hovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, m_style.close_button_active);
                ImGui::PushStyleColor(ImGuiCol_Text, m_style.close_button_text);
                const bool close_clicked = ImGuiX::Widgets::CloseXButton("##close", btn_w);
                ImGui::PopStyleColor(4);

                if (close_clicked) {
                    ImGui::PopTextWrapPos(); 
                    if (m_fonts.text) ImGui::PopFont();
                    ImGui::PopID();
                    ImGui::PopStyleColor();
                    ImGui::End();
                    ImGui::PopStyleVar(3);
                    ImGui::PopStyleColor(3);
                    remove(i); height += m_style.spacing_y; --i; continue;
                }
                /*
                ImGuiX::Widgets::IconButtonConfig ib;
                if (m_fonts.icons) ib.font = m_fonts.icons;
                if (ImGuiX::Widgets::IconButtonCentered("##close", m_icons.icon_close, ib)) {
                    ImGui::PopTextWrapPos(); 
                    if (m_fonts.text) ImGui::PopFont();
                    ImGui::PopID();
                    ImGui::End();
                    remove(i); height += m_cfg.padding_message_y; --i; continue;
                }
                */
            }

            if (title_rendered && toast.content() && toast.content()[0])
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);

            if (toast.content() && toast.content()[0]) {
                if (title_rendered && m_cfg.use_separator) ImGui::Separator();
                ImGui::TextUnformatted(toast.content());
            }

            // Deferred onClick to avoid vector reallocation invalidation
            std::function<void()> deferred;
            if (toast.onButtonPress()) {
                if (const char* lbl = toast.buttonLabel(); lbl && lbl[0]) {
                    ImGui::PushStyleColor(ImGuiCol_Button, m_style.action_button_background);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_style.action_button_hovered);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, m_style.action_button_active);
                    ImGui::PushStyleColor(ImGuiCol_Text, m_style.action_button_text);
                    const bool action_clicked = ImGui::Button(lbl);
                    ImGui::PopStyleColor(4);
                    if (action_clicked) deferred = toast.onButtonPress();
                }
            }

            ImGui::PopTextWrapPos();
            height += ImGui::GetWindowHeight() + m_style.spacing_y;
            if (m_fonts.text) ImGui::PopFont();
            ImGui::PopID();
            ImGui::PopStyleColor();
            ImGui::End();
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(3);

            if (deferred) deferred();
        }
    }


    /// \brief Посчитать эффективный dismiss с учётом политики и wrap.
    int NotificationManager::computeEffectiveDismissMs(
            const Notification& n,
            const Config& cfg,
            float wrap_width,
            int wrapped_lines /*>=1*/
        ) const {
        // Если пользователь задал dismiss явно — уважаем его
        if (n.userDismissMs() > 0)
            return std::clamp(n.userDismissMs(), cfg.min_dismiss_ms, cfg.max_dismiss_ms);

        int total = cfg.default_dismiss_ms;

        if (cfg.auto_mode == AutoDurationMode::PerChar) {
            const int chars = n.contentLength();
            total += static_cast<int>(chars * cfg.ms_per_char);
        } else 
        if (cfg.auto_mode == AutoDurationMode::WPM) {
            const int chars = n.contentLength();
            const int words = std::max(1, static_cast<int>(std::ceil(chars / std::max(1.0f, cfg.avg_chars_per_word))));
            const float minutes = words / std::max(1.0f, cfg.reading_wpm);
            total += static_cast<int>(minutes * 60000.0f);
        }

        if (wrapped_lines > 1)
            total += cfg.extra_per_line_ms * (wrapped_lines - 1);

        return std::clamp(total, cfg.min_dismiss_ms, cfg.max_dismiss_ms);
    }


} // namespace ImGuiX::Notify
