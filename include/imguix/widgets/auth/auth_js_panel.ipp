#include "auth_js_panel.hpp"
#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE bool AuthJsPanel(const char* id, AuthJsPanelConfig& cfg, AuthJsSettings& st) {
        bool changed = false;
        ImGui::PushID(id);

        // Auto size similar to other panels
        float height = ImGui::GetTextLineHeightWithSpacing() + 4.0f * ImGui::GetFrameHeightWithSpacing();
        ImVec2 size = cfg.panel_size;
        if (size.x <= 0.0f) size.x = ImGui::CalcItemWidth();
        if (size.y <= 0.0f) size.y = std::max(height, ImGui::GetFrameHeightWithSpacing());

        ImGui::BeginChild(u8"##auth_js_panel", size, cfg.border);
        ImGui::TextUnformatted(cfg.header.c_str());
        if (cfg.show_separator) {
            ImGui::Separator();
        }

        if (cfg.inputs_fill_width) ImGui::PushItemWidth(-FLT_MIN);

        // User-Agent
        {
            bool ok = true;
            if (cfg.vk_user_agent) {
                if (InputTextWithVKValidated(u8"##authjs.user_agent",
                        cfg.hint_user_agent.c_str(),
                        st.user_agent,
                        cfg.validate_user_agent,
                        cfg.policy,
                        cfg.user_agent_regex,
                        ok,
                        cfg.error_color,
                        cfg.input_flags,
                        /*callback*/nullptr, /*user_data*/nullptr,
                        cfg.kb_cfg, cfg.vk_cfg)) {
                    changed = true;
                }
            } else {
                if (InputTextValidated(u8"##authjs.user_agent",
                        cfg.hint_user_agent.c_str(),
                        st.user_agent,
                        cfg.validate_user_agent,
                        cfg.policy,
                        cfg.user_agent_regex,
                        ok,
                        cfg.error_color,
                        cfg.input_flags)) {
                    changed = true;
                }
            }
        }

        // Accept-Language
        {
            bool ok = true;
            if (cfg.vk_accept_language) {
                if (InputTextWithVKValidated(u8"##authjs.accept_lang",
                        cfg.hint_accept_language.c_str(),
                        st.accept_language,
                        cfg.validate_accept_language,
                        cfg.policy,
                        cfg.accept_language_regex,
                        ok,
                        cfg.error_color,
                        cfg.input_flags,
                        /*callback*/nullptr, /*user_data*/nullptr,
                        cfg.kb_cfg, cfg.vk_cfg)) {
                    changed = true;
                }
            } else {
                if (InputTextValidated(u8"##authjs.accept_lang",
                        cfg.hint_accept_language.c_str(),
                        st.accept_language,
                        cfg.validate_accept_language,
                        cfg.policy,
                        cfg.accept_language_regex,
                        ok,
                        cfg.error_color,
                        cfg.input_flags)) {
                    changed = true;
                }
            }
        }

        // DNT
        if (ImGui::Checkbox(cfg.label_dnt.c_str(), &st.dnt)) {
            changed = true;
        }

        if (cfg.inputs_fill_width) ImGui::PopItemWidth();

        ImGui::EndChild();
        ImGui::PopID();
        return changed;
    }

} // namespace ImGuiX::Widgets
