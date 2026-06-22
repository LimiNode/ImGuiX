#include "proxy_panel.hpp"
#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE bool ProxyPanel(const char* id, ProxyPanelConfig& cfg, ProxySettings& st) {
        bool changed = false;

        ImGui::PushID(id);
        ImVec2 size = cfg.panel_size;
        if (size.x <= 0.0f) {
            size.x = ImGui::CalcItemWidth();
        }
        if (size.y <= 0.0f) {
            size.y = ImGui::GetTextLineHeightWithSpacing() + 4.0f * ImGui::GetFrameHeightWithSpacing();
            if (cfg.show_check_results) {
                size.y += ImGui::GetTextLineHeightWithSpacing();
                if (!cfg.check_results.empty()) {
                    size.y += ImGui::GetTextLineHeightWithSpacing();
                    size.y += static_cast<float>(cfg.check_results.size()) * ImGui::GetTextLineHeightWithSpacing();
                }
            }
        }

        auto calc_w = [&](const char* s) -> float {
            // ширина текста + паддинги
            ImVec2 ts = ImGui::CalcTextSize(s, nullptr, false);
            return ts.x + 2.0f * ImGui::GetStyle().FramePadding.x;
        };

        ImGui::BeginChild(u8"##proxy_panel", size, cfg.border);
        ImGui::Text(u8"%s", cfg.header ? cfg.header : u8"Proxy");
        if (cfg.show_separator) {
            ImGui::Separator();
        }

        if (cfg.inputs_fill_width) ImGui::PushItemWidth(-FLT_MIN);

        const ImGuiStyle& style = ImGui::GetStyle();
        float item_w = cfg.field_width_host_user > 0.0f ?
            cfg.field_width_host_user :
            0.5f * (ImGui::CalcItemWidth() - calc_w(u8":") - 2.0f * style.ItemSpacing.x);
        const float spacing = style.ItemInnerSpacing.x;

        // --- Host + port ---
        ImGui::BeginDisabled(!st.use_proxy);

        // --- IP ---
        ImGui::PushItemWidth(item_w);
        {
            if (cfg.vk_ip) {
                changed = InputTextWithVKValidated(
                    u8"##proxy.ip", cfg.hint_ip,
                    st.ip, cfg.validate_ip, InputValidatePolicy::OnTouch,
                    cfg.ip_regex, st.ip_valid,
                    cfg.error_color,
                    ImGuiInputTextFlags_AutoSelectAll, /*cb*/nullptr, /*ud*/nullptr,
                    cfg.kb_cfg, cfg.vk_cfg
                );
            } else {
                changed = InputTextValidated(
                    u8"##proxy.ip", cfg.hint_ip,
                    st.ip, cfg.validate_ip, InputValidatePolicy::OnTouch,
                    cfg.ip_regex, st.ip_valid, cfg.error_color
                );
            }
        }
        ImGui::PopItemWidth();

        ImGui::SameLine();
        ImGui::TextUnformatted(u8":");

        // --- Port ---
        const float frame_h = ImGui::GetFrameHeight();
        ImVec2 btn_sz = cfg.kb_cfg.button_size;
        if (btn_sz.x <= 0.0f || btn_sz.y <= 0.0f) {
            btn_sz = ImVec2(frame_h, frame_h);
        }
        const float port_w = cfg.field_width_port > 0.0f ?
            cfg.field_width_port :
            calc_w(u8"65535") + (cfg.vk_port ? (btn_sz.x + spacing) : 0.0f);

        ImGui::SameLine();
        ImGui::PushItemWidth(port_w);
        {
            std::string pbuf(std::to_string(std::max(0, st.port)));
            if (cfg.vk_port) {
                changed = InputTextWithVKValidated(
                    u8"##proxy.port", cfg.hint_port,
                    pbuf, cfg.validate_port, InputValidatePolicy::OnTouch,
                    cfg.port_regex, st.port_valid,
                    cfg.error_color,
                    ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsDecimal,
                    /*cb*/nullptr, /*ud*/nullptr,
                    cfg.kb_cfg, cfg.vk_cfg
                );
            } else {
                changed = InputTextValidated(
                    u8"##proxy.port", cfg.hint_port,
                    pbuf, cfg.validate_port, InputValidatePolicy::OnTouch,
                    cfg.port_regex, st.port_valid, cfg.error_color
                );
            }
            if (changed) {
                int v = std::stoi(pbuf);
                v = std::clamp(v, 0, 65535);
                if (v != st.port) { st.port = v; changed = true; }
                else changed = false;
            }
        }
        ImGui::PopItemWidth();

        // --- Type ---
        if (cfg.show_type) {
            float type_w = cfg.field_width_type > 0.0f ?
                cfg.field_width_type :
                item_w - port_w - 2.0f * spacing;
            //calc_w(u8"SOCKS 5")
            type_w = std::max(calc_w(u8"SOCKS 5") + 2.0f * spacing, type_w);
            ImGui::SameLine();
            ImGui::PushItemWidth(type_w);
            int t = static_cast<int>(st.type);
            const char* items[] = { u8"HTTP", u8"SOCKS 4", u8"SOCKS 5" };
            if (ImGui::Combo(u8"##proxy.type", &t, items, IM_ARRAYSIZE(items))) {
                st.type = static_cast<ProxyType>(t);
                changed = true;
            }
            ImGui::PopItemWidth();
        }

        // --- User ---
        ImGui::PushItemWidth(item_w);
        {
            if (cfg.vk_username) {
                changed = InputTextWithVKValidated(
                    u8"##proxy.user", cfg.hint_username,
                    st.username, cfg.validate_username, InputValidatePolicy::OnTouch,
                    cfg.username_regex, st.username_valid,
                    cfg.error_color,
                    ImGuiInputTextFlags_AutoSelectAll,
                    /*cb*/nullptr, /*ud*/nullptr,
                    cfg.kb_cfg, cfg.vk_cfg
                );
            } else {
                changed = InputTextValidated(
                    u8"##proxy.user", cfg.hint_username,
                    st.username, cfg.validate_username, InputValidatePolicy::OnTouch,
                    cfg.username_regex, st.username_valid, cfg.error_color
                );
            }
        }
        ImGui::PopItemWidth();

        ImGui::SameLine();
        ImGui::TextUnformatted(u8":");

        // --- Pass ---
        ImGui::SameLine();
        ImGui::PushItemWidth(item_w);
        {
            if (cfg.vk_password) {
                changed = InputPasswordWithToggleVK(
                    u8"##proxy.pass", cfg.hint_password,
                    st.password, cfg.validate_password, InputValidatePolicy::OnTouch,
                    cfg.password_regex, st.password_valid,
                    cfg.toggle_cfg, cfg.kb_cfg, cfg.vk_cfg,
                    cfg.error_color
                );
            } else {
                changed = InputPasswordWithToggle(
                    u8"##proxy.pass", cfg.hint_password,
                    st.password, cfg.validate_password, InputValidatePolicy::OnTouch,
                    cfg.password_regex, st.password_valid,
                    cfg.toggle_cfg, cfg.error_color
                );
            }
        }
        ImGui::PopItemWidth();

        ImGui::EndDisabled();

        // --- Use proxy checkbox ---
        if (ImGui::Checkbox(cfg.label_use_proxy, &st.use_proxy)) {
            changed = true;
        }

        // --- Check button + status ---
        if (cfg.show_check && st.use_proxy) {
            ImGui::SameLine();
            // Hide button if IP is invalid
            bool disabled = !st.ip_valid ||
                            !st.port_valid ||
                            !st.password_valid ||
                            !st.username_valid;

            if (disabled) ImGui::BeginDisabled();
            if (ImGui::Button(cfg.button_check)) {
                if (!disabled && cfg.on_check) cfg.on_check();
            }
            if (disabled) ImGui::EndDisabled();

            ImGui::SameLine();
            if (cfg.checked) {
                ImGui::PushStyleColor(ImGuiCol_CheckMark,
                    cfg.check_ok ? ImVec4(0.14f, 0.8f, 0.27f, 1.0f)
                                 : ImVec4(0.78f, 0.14f, 0.14f, 1.0f));
                ImGui::RadioButton(cfg.label_checked, true);
                ImGui::PopStyleColor();
            } else {
                ImGui::RadioButton(cfg.label_checked, false);
            }
        }

        if (cfg.show_check_results) {
            if (ImGui::TreeNodeEx(
                    cfg.label_check_results,
                    ImGuiTreeNodeFlags_SpanAvailWidth)) {
                if (ImGui::BeginTable(
                        "##proxy_check_results",
                        3,
                        ImGuiTableFlags_SizingStretchProp |
                            ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_BordersInnerV)) {
                    ImGui::TableSetupColumn(cfg.label_geo);
                    ImGui::TableSetupColumn(cfg.label_ip);
                    ImGui::TableSetupColumn(cfg.label_ping);
                    ImGui::TableHeadersRow();

                    for (const auto& row : cfg.check_results) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(row.geo.c_str());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextUnformatted(row.ip.c_str());
                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextUnformatted(row.ping.c_str());
                    }

                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }
        }

        if (cfg.inputs_fill_width) ImGui::PopItemWidth();

        ImGui::EndChild();
        ImGui::PopID();
        return changed;
    }

} // namespace ImGuiX::Widgets
