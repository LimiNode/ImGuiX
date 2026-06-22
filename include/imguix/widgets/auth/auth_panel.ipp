#include "auth_panel.hpp"
#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE AuthPanelResult AuthPanel(
            const char* id,
            AuthPanelConfig& cfg,
            AuthData& data
        ) {
        AuthPanelResult res = AuthPanelResult::None;
        ImGui::PushID(id);

		// Compute a conservative height to keep the child stable.
		float height = 0.0f;
        if (cfg.show_header) {
            height += ImGui::GetTextLineHeightWithSpacing();
        }
		height = std::max(height, ImGui::GetFrameHeightWithSpacing());

        if (cfg.show_host)      height += ImGui::GetFrameHeightWithSpacing();
        if (cfg.show_email)     height += ImGui::GetFrameHeightWithSpacing();
        if (cfg.show_password)  height += ImGui::GetFrameHeightWithSpacing();
        if (cfg.show_token)     height += ImGui::GetFrameHeightWithSpacing();
        if (cfg.show_api_keys)  height += 2.0f * ImGui::GetFrameHeightWithSpacing();
        if (cfg.show_connect_button) height += ImGui::GetFrameHeightWithSpacing();
        height += ImGui::GetFrameHeightWithSpacing();

        ImVec2 size = cfg.panel_size;
        if (size.x <= 0.0f) size.x = ImGui::CalcItemWidth();
        if (size.y <= 0.0f) size.y = height;

        ImGui::BeginChild(u8"##AuthPanel", size, true);

        if (cfg.show_header) {
            ImGui::TextUnformatted(cfg.header ? cfg.header : "");
            if (cfg.show_separator) {
                ImGui::Separator();
            }
        }

        if (cfg.inputs_fill_width) ImGui::PushItemWidth(-FLT_MIN);

        // Host
        if (cfg.show_host) {
            bool changed = false;
            if (cfg.vk_host) {
                changed = InputTextWithVKValidated(
                    u8"##host", cfg.hint_host,
                    data.host, cfg.validate_host, InputValidatePolicy::OnTouch,
                    cfg.host_regex, data.host_valid,
                    cfg.error_color,
                    /*extra_flags*/0, /*cb*/nullptr, /*ud*/nullptr,
                    cfg.kb_cfg, cfg.vk_cfg
                );
            } else {
                changed = InputTextValidated(
                    u8"##host", cfg.hint_host,
                    data.host, cfg.validate_host, InputValidatePolicy::OnTouch,
                    cfg.host_regex, data.host_valid, cfg.error_color
                );
            }
            if (changed) res |= AuthPanelResult::HostChanged;
        }

        // Token
        if (cfg.show_token) {
            bool changed = false;
            if (cfg.vk_token) {
                changed = InputPasswordWithToggleVK(
                    u8"##token", cfg.hint_token,
                    data.token, cfg.validate_token, InputValidatePolicy::OnTouch,
                    cfg.token_regex, data.token_valid,
                    cfg.toggle_cfg, cfg.kb_cfg, cfg.vk_cfg,
                    cfg.error_color
                );
            } else {
                changed = InputPasswordWithToggle(
                    u8"##token", cfg.hint_token,
                    data.token, cfg.validate_token, InputValidatePolicy::OnTouch,
                    cfg.token_regex, data.token_valid,
                    cfg.toggle_cfg, cfg.error_color
                );
            }
            if (changed) res |= AuthPanelResult::TokenChanged;
        }

        // API keys
        if (cfg.show_api_keys) {
            // public (usually not masked, but we keep the same UI for symmetry)
            {
                bool changed = false;
                if (cfg.vk_api_key) {
                    changed = InputPasswordWithToggleVK(
                        u8"##api_key", cfg.hint_api_key,
                        data.api_key, cfg.validate_api_key, InputValidatePolicy::OnTouch,
                        cfg.api_key_regex, data.api_key_valid,
                        cfg.toggle_cfg, cfg.kb_cfg, cfg.vk_cfg,
                        cfg.error_color
                    );
                } else {
                    changed = InputPasswordWithToggle(
                        u8"##api_key", cfg.hint_api_key,
                        data.api_key, cfg.validate_api_key, InputValidatePolicy::OnTouch,
                        cfg.api_key_regex, data.api_key_valid,
                        cfg.toggle_cfg, cfg.error_color
                    );
                }
                if (changed) res |= AuthPanelResult::ApiKeyChanged;
            }
            // secret
            {
                bool changed = false;
                if (cfg.vk_api_secret) {
                    changed = InputPasswordWithToggleVK(
                        u8"##api_secret", cfg.hint_api_secret,
                        data.api_secret, cfg.validate_api_secret, InputValidatePolicy::OnTouch,
                        cfg.api_secret_regex, data.api_secret_valid,
                        cfg.toggle_cfg, cfg.kb_cfg, cfg.vk_cfg,
                        cfg.error_color
                    );
                } else {
                    changed = InputPasswordWithToggle(
                        u8"##api_secret", cfg.hint_api_secret,
                        data.api_secret, cfg.validate_api_secret, InputValidatePolicy::OnTouch,
                        cfg.api_secret_regex, data.api_secret_valid,
                        cfg.toggle_cfg, cfg.error_color
                    );
                }
                if (changed) res |= AuthPanelResult::ApiSecretChanged;
            }
        }

        // Email
        if (cfg.show_email) {
            bool changed = false;
            if (cfg.vk_email) {
                changed = InputTextWithVKValidated(
                    u8"##email", cfg.hint_email,
                    data.email, cfg.validate_email, InputValidatePolicy::OnTouch,
                    cfg.email_regex, data.email_valid,
                    cfg.error_color,
                    /*extra_flags*/0, /*cb*/nullptr, /*ud*/nullptr,
                    cfg.kb_cfg, cfg.vk_cfg
                );
            } else {
                changed = InputTextValidated(
                    u8"##email", cfg.hint_email,
                    data.email, cfg.validate_email, InputValidatePolicy::OnTouch,
                    cfg.email_regex, data.email_valid, cfg.error_color
                );
            }
            if (changed) res |= AuthPanelResult::EmailChanged;

            // Optional email state pill
            if (cfg.use_email_state) {
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_CheckMark, cfg.email_state_color);
                ImGui::RadioButton(cfg.email_state_label.empty() ? u8"state" : cfg.email_state_label.c_str(),
                                   cfg.email_state);
                ImGui::PopStyleColor();
            }
        }

        // Password
        if (cfg.show_password) {
            bool changed = false;
            if (cfg.vk_password) {
                changed = InputPasswordWithToggleVK(
                    u8"##password", cfg.hint_password,
                    data.password, cfg.validate_password, InputValidatePolicy::OnTouch,
                    cfg.password_regex, data.password_valid,
                    cfg.toggle_cfg, cfg.kb_cfg, cfg.vk_cfg,
                    cfg.error_color
                );
            } else {
                changed = InputPasswordWithToggle(
                    u8"##password", cfg.hint_password,
                    data.password, cfg.validate_password, InputValidatePolicy::OnTouch,
                    cfg.password_regex, data.password_valid,
                    cfg.toggle_cfg, cfg.error_color
                );
            }
            if (changed) res |= AuthPanelResult::PasswordChanged;
        }

        // Connection button
        if (cfg.show_connect_button) {
            if (cfg.show_connect_separator) {
                ImGui::Separator();
            }
            // Connection state + action
            if (cfg.show_connection_state) {
                if (cfg.init) {
                    ImGui::PushStyleColor(ImGuiCol_CheckMark,
                        cfg.connected ? ImVec4(0.14f, 0.8f, 0.27f, 1.0f)
                                      : ImVec4(0.78f, 0.14f, 0.14f, 1.0f));
                    ImGui::RadioButton(cfg.connected_label, true);
                    ImGui::PopStyleColor();
                } else {
                    ImGui::RadioButton(cfg.connected_label, false);
                }
                ImGui::SameLine();
            }
            if (ImGui::Button(cfg.connect_label)) {
                if (cfg.on_connect) cfg.on_connect();
                res |= AuthPanelResult::ConnectClicked;
            }
        }

        if (cfg.inputs_fill_width) ImGui::PopItemWidth();

        ImGui::EndChild();
        ImGui::PopID();
        return res;
    }

} // namespace ImGuiX::Widgets
