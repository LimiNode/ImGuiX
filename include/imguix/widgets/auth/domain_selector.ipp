#include "domain_selector.hpp"
#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE bool DomainSelector(
            const char* label,
            DomainSelectorConfig& cfg,
            std::string& host
        ) {
        ImGui::PushID(label);
        ImGuiStorage* st = ImGui::GetStateStorage();
        const ImGuiID key_combo_index = ImGui::GetID("##combo_index");

        float height = ImGui::GetTextLineHeightWithSpacing() + 3.0f * ImGui::GetFrameHeightWithSpacing();
        ImVec2 size = cfg.panel_size;
        if (size.x <= 0.0f) size.x = ImGui::CalcItemWidth();
        if (size.y <= 0.0f) size.y = std::max(height, ImGui::GetFrameHeightWithSpacing());

        ImGui::BeginChild(u8"##DomainSelector", size, cfg.border);
        ImGui::TextUnformatted(cfg.header.c_str());
        if (cfg.show_separator) {
            ImGui::Separator();
        }

        const auto effective_help_style = [&](bool has_domain_combo) {
            if (cfg.help_style != DomainSelectorHelpStyle::Auto) {
                return cfg.help_style;
            }
            return has_domain_combo
                ? DomainSelectorHelpStyle::TextQuestion
                : DomainSelectorHelpStyle::HelpIcon;
        };

        const auto help_marker_width = [&](bool has_domain_combo) {
            if (!cfg.show_help) {
                return 0.0f;
            }
            const DomainSelectorHelpStyle style = effective_help_style(has_domain_combo);
            const char* marker_text = style == DomainSelectorHelpStyle::HelpIcon
                ? (cfg.help_icon ? cfg.help_icon : IMGUIX_ICON_HELP)
                : u8"(?)";
            return ImGui::CalcTextSize(marker_text).x + ImGui::GetStyle().ItemSpacing.x;
        };

        const auto set_fill_width = [&](float reserved_width = 0.0f) {
            if (cfg.inputs_fill_width) {
                ImGui::SetNextItemWidth(std::max(1.0f, ImGui::GetContentRegionAvail().x - reserved_width));
            }
        };

        const auto draw_help_marker = [&](bool has_domain_combo) {
            const DomainSelectorHelpStyle style = effective_help_style(has_domain_combo);
            if (style == DomainSelectorHelpStyle::HelpIcon) {
                ImGuiX::Widgets::HelpMarker(
                    cfg.help_text.c_str(),
                    MarkerMode::TooltipOnly,
                    cfg.help_icon ? cfg.help_icon : IMGUIX_ICON_HELP
                );
            } else {
                ImGui::TextDisabled(u8"(?)");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(u8"%s", cfg.help_text.c_str());
                }
            }
        };

        bool updated = false;
        bool use_input = false;
        if (!cfg.domains.empty()) {
            // Build items + detect current
            std::vector<const char*> items;
            items.reserve(cfg.domains.size() + 1);
            for (size_t i = 0; i < cfg.domains.size(); ++i) {
                items.push_back(cfg.domains[i].c_str());
            }
            items.push_back(cfg.custom_text.c_str());

            int combo_index = st->GetInt(key_combo_index, (static_cast<int>(items.size()) - 1));
            if (combo_index < 0) combo_index = static_cast<int>(items.size()) - 1;
            if (combo_index >= static_cast<int>(items.size())) combo_index = static_cast<int>(items.size()) - 1;

            set_fill_width(help_marker_width(true));
            if (ImGui::Combo(
                    u8"##domain.combo",
                    &combo_index,
                    items.data(),
                    static_cast<int>(items.size()))) {
                if (combo_index == static_cast<int>(cfg.domains.size())) {
                    host = cfg.default_domain;
                } else {
                    host = cfg.domains[combo_index];
                }
                st->SetInt(key_combo_index, combo_index);
                updated = true;
            }

            if (combo_index == static_cast<int>(cfg.domains.size())) {
                use_input = true;
            } else {
                use_input = false;
            }

            if (cfg.show_help) {
                ImGui::SameLine();
                draw_help_marker(true);
            }
        } else {
            use_input = true;
        }

        if (use_input) {
            bool ok = true;
            if (updated) {
                ImGui::SetKeyboardFocusHere(0);
            }
            set_fill_width(cfg.domains.empty() ? help_marker_width(false) : 0.0f);
            if (cfg.vk_enabled) {
                if (InputTextWithVKValidated(
                        u8"##domain.input",
                        cfg.hint_domain.c_str(),
                        host,
                        cfg.validate_domain,
                        cfg.policy,
                        cfg.domain_regex,
                        ok,
                        cfg.error_color,
                        cfg.input_flags,
                        /*callback*/nullptr, /*user_data*/nullptr,
                        cfg.kb_cfg, cfg.vk_cfg)) {
                    updated = true;
                }
            } else {
                if (InputTextValidated(
                        u8"##domain.input",
                        cfg.hint_domain.c_str(),
                        host,
                        cfg.validate_domain,
                        cfg.policy,
                        cfg.domain_regex,
                        ok,
                        cfg.error_color,
                        cfg.input_flags)) {
                    updated = true;
                }
            }
        }

        if (cfg.domains.empty() && cfg.show_help) {
            ImGui::SameLine();
            draw_help_marker(false);
        }

        ImGui::EndChild();
        ImGui::PopID();
        return updated;
    }

} // namespace ImGuiX::Widgets
