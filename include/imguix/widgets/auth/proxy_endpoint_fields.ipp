#include "proxy_endpoint_fields.hpp"

#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE bool ProxyEndpointFields(
            const char* id,
            ProxyEndpointFieldsConfig& cfg,
            ProxyEndpointSettings& st
        ) {
        bool changed = false;

        ImGui::PushID(id);
        const float check_height = cfg.show_check ? ImGui::GetFrameHeightWithSpacing() : 0.0f;
        const float auto_height = cfg.show_header ?
            ImGui::GetTextLineHeightWithSpacing() + ImGui::GetFrameHeightWithSpacing() + check_height :
            ImGui::GetFrameHeightWithSpacing() + check_height;
        const ImVec2 size{
            cfg.panel_size.x <= 0.0f ? ImGui::CalcItemWidth() : cfg.panel_size.x,
            cfg.panel_size.y <= 0.0f ? auto_height : cfg.panel_size.y
        };

        auto calc_w = [](const char* s) -> float {
            ImVec2 ts = ImGui::CalcTextSize(s, nullptr, false);
            return ts.x + 2.0f * ImGui::GetStyle().FramePadding.x;
        };

        ImGui::BeginChild(u8"##proxy_endpoint_fields", size, cfg.border);
        if (cfg.show_header) {
            ImGui::Text(u8"%s", cfg.header ? cfg.header : u8"Proxy");
            ImGui::Separator();
        }

        if (cfg.inputs_fill_width) {
            ImGui::PushItemWidth(-FLT_MIN);
        }

        const ImGuiStyle& style = ImGui::GetStyle();
        const float spacing = style.ItemInnerSpacing.x;

        const float full_w = ImGui::CalcItemWidth();

        const float frame_h = ImGui::GetFrameHeight();
        ImVec2 btn_sz = cfg.kb_cfg.button_size;
        if (btn_sz.x <= 0.0f || btn_sz.y <= 0.0f) {
            btn_sz = ImVec2(frame_h, frame_h);
        }

        const float port_w = cfg.field_width_port > 0.0f ?
            cfg.field_width_port :
            calc_w(u8"65535") + (cfg.vk_port ? (btn_sz.x + spacing) : 0.0f);

        float type_w = cfg.field_width_type > 0.0f ?
            cfg.field_width_type :
            std::max(calc_w(u8"SOCKS 5") + 2.0f * spacing, 0.25f * full_w);

        const float separator_w = calc_w(u8":");
        const float used_w = port_w + type_w + separator_w + 4.0f * style.ItemSpacing.x;
        const float host_w = cfg.field_width_host > 0.0f ?
            cfg.field_width_host :
            std::max(0.0f, full_w - used_w);

        ImGui::PushItemWidth(host_w);
        {
            bool field_changed = false;
            if (cfg.vk_ip) {
                field_changed = InputTextWithVKValidated(
                    u8"##proxy.endpoint.ip",
                    cfg.hint_ip,
                    st.ip,
                    cfg.validate_ip,
                    InputValidatePolicy::OnTouch,
                    cfg.ip_regex,
                    st.ip_valid,
                    cfg.error_color,
                    ImGuiInputTextFlags_AutoSelectAll,
                    nullptr,
                    nullptr,
                    cfg.kb_cfg,
                    cfg.vk_cfg
                );
            } else {
                field_changed = InputTextValidated(
                    u8"##proxy.endpoint.ip",
                    cfg.hint_ip,
                    st.ip,
                    cfg.validate_ip,
                    InputValidatePolicy::OnTouch,
                    cfg.ip_regex,
                    st.ip_valid,
                    cfg.error_color
                );
            }
            changed = changed || field_changed;
        }
        ImGui::PopItemWidth();

        ImGui::SameLine();
        ImGui::TextUnformatted(u8":");

        ImGui::SameLine();
        ImGui::PushItemWidth(port_w);
        {
            std::string pbuf(std::to_string(std::max(0, st.port)));
            bool field_changed = false;
            if (cfg.vk_port) {
                field_changed = InputTextWithVKValidated(
                    u8"##proxy.endpoint.port",
                    cfg.hint_port,
                    pbuf,
                    cfg.validate_port,
                    InputValidatePolicy::OnTouch,
                    cfg.port_regex,
                    st.port_valid,
                    cfg.error_color,
                    ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_CharsDecimal,
                    nullptr,
                    nullptr,
                    cfg.kb_cfg,
                    cfg.vk_cfg
                );
            } else {
                field_changed = InputTextValidated(
                    u8"##proxy.endpoint.port",
                    cfg.hint_port,
                    pbuf,
                    cfg.validate_port,
                    InputValidatePolicy::OnTouch,
                    cfg.port_regex,
                    st.port_valid,
                    cfg.error_color
                );
            }

            if (field_changed) {
                int value = 0;
                if (!pbuf.empty()) {
                    value = std::stoi(pbuf);
                }
                value = std::clamp(value, 0, 65535);
                if (value != st.port) {
                    st.port = value;
                    changed = true;
                }
            }
        }
        ImGui::PopItemWidth();

        type_w = std::max(calc_w(u8"SOCKS 5") + 2.0f * spacing, type_w);
        ImGui::SameLine();
        ImGui::PushItemWidth(type_w);
        {
            int type_index = static_cast<int>(st.type);
            const char* items[] = { u8"HTTP", u8"SOCKS 4", u8"SOCKS 5" };
            if (ImGui::Combo(u8"##proxy.endpoint.type", &type_index, items, IM_ARRAYSIZE(items))) {
                st.type = static_cast<ProxyType>(type_index);
                changed = true;
            }
        }
        ImGui::PopItemWidth();

        // --- Check button + status ---
        if (cfg.show_check) {
            const bool disabled = !st.ip_valid || !st.port_valid;

            if (disabled) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button(cfg.button_check)) {
                if (!disabled && cfg.on_check) {
                    cfg.on_check();
                }
            }
            if (disabled) {
                ImGui::EndDisabled();
            }

            ImGui::SameLine();
            if (cfg.checked) {
                ImGui::PushStyleColor(
                    ImGuiCol_CheckMark,
                    cfg.check_ok ?
                        ImVec4(0.14f, 0.8f, 0.27f, 1.0f) :
                        ImVec4(0.78f, 0.14f, 0.14f, 1.0f)
                );
                ImGui::RadioButton(cfg.label_checked, true);
                ImGui::PopStyleColor();
            } else {
                ImGui::RadioButton(cfg.label_checked, false);
            }
        }

        if (cfg.inputs_fill_width) {
            ImGui::PopItemWidth();
        }

        ImGui::EndChild();
        ImGui::PopID();
        return changed;
    }

    IMGUIX_IMPL_INLINE bool ProxyEndpointFields(
            const char* id,
            ProxyEndpointFieldsConfig& cfg,
            ProxySettings& st
        ) {
        ProxyEndpointSettings endpoint{};
        endpoint.ip = st.ip;
        endpoint.port = st.port;
        endpoint.type = st.type;
        endpoint.ip_valid = st.ip_valid;
        endpoint.port_valid = st.port_valid;

        const bool changed = ProxyEndpointFields(id, cfg, endpoint);

        st.ip = endpoint.ip;
        st.port = endpoint.port;
        st.type = endpoint.type;
        st.ip_valid = endpoint.ip_valid;
        st.port_valid = endpoint.port_valid;

        return changed;
    }

} // namespace ImGuiX::Widgets
