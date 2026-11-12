#include <imguix/config/build.hpp>
#include <imguix/widgets/layout_primitives.hpp>

#include <algorithm>
#include <cfloat>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE bool BeginSectionCard(const char* id, const SectionCardConfig& config) {
        IM_ASSERT(config.height_policy != SectionCardHeightPolicy::Fixed || config.size.y > 0.0f);
        IM_ASSERT(config.height_policy != SectionCardHeightPolicy::Constrained ||
                  config.max_height > 0.0f);
        RoundedPanelConfig panel = config.panel;
        // Decorations stay in this child's draw list, never above a popup.
        panel.draw_decorations_in_child = true;
        ImVec2 size = config.size;
        if (config.height_policy != SectionCardHeightPolicy::Fixed) {
            size.y = 0.0f;
            panel.child_flags |= ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize;
            const float maximum = config.height_policy == SectionCardHeightPolicy::Constrained
                                      ? std::max(1.0f, config.max_height)
                                      : FLT_MAX;
            ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(FLT_MAX, maximum));
        }

        SectionCardScrollPolicy scroll_policy = config.scroll_policy;
        if (config.height_policy == SectionCardHeightPolicy::Constrained &&
            scroll_policy == SectionCardScrollPolicy::Never) {
            // A constrained card must not silently clip content when the
            // caller leaves the overflow policy at the content-card default.
            scroll_policy = SectionCardScrollPolicy::Vertical;
        }
        if (config.height_policy == SectionCardHeightPolicy::Content ||
            scroll_policy == SectionCardScrollPolicy::Never) {
            panel.window_flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        } else if (scroll_policy == SectionCardScrollPolicy::Both) {
            panel.window_flags |= ImGuiWindowFlags_HorizontalScrollbar;
        }

        const bool is_open = BeginRoundedPanel(id, size, panel);
        if (is_open && config.title != nullptr && config.title[0] != '\0') {
            PageHeaderConfig header;
            header.title = config.title;
            header.title_font = config.title_font;
            header.actions = config.header_actions;
            header.actions_width = config.header_actions_width;
            // Set only the heading-to-body gap; no extra Dummy/line-height.
            const ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
            ImGui::PushStyleVar(
                ImGuiStyleVar_ItemSpacing,
                ImVec2(spacing.x, config.title_gap < 0.0f ? spacing.y : config.title_gap));
            PageHeader(header);
            ImGui::PopStyleVar();
        }
        return is_open;
    }

    IMGUIX_IMPL_INLINE void EndSectionCard() {
        EndRoundedPanel();
    }

}  // namespace ImGuiX::Widgets
