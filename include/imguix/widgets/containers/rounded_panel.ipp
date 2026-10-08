#include <imguix/config/build.hpp>

#include <algorithm>
#include <vector>

namespace ImGuiX::Widgets {

    namespace {

        struct RoundedPanelState {
            ImVec2 min{};
            ImVec2 max{};

            float rounding{0.0f};
            float border_thickness{0.0f};

            ImU32 background_color{0};
            ImU32 outside_color{0};
            ImU32 border_color{0};

            RoundedPanelClipMode clip_mode{
                RoundedPanelClipMode::CoverCorners};

            bool content_clip_pushed{false};
        };

        std::vector<RoundedPanelState>& rounded_panel_stack() {
            static thread_local std::vector<RoundedPanelState> stack;
            return stack;
        }

        bool isUnsetColor(const ImVec4& color) {
            return color.x == 0.0f &&
                   color.y == 0.0f &&
                   color.z == 0.0f &&
                   color.w == 0.0f;
        }

        float clampRounding(
                float rounding,
                const ImVec2& min,
                const ImVec2& max) {
            const float width =
                std::max(0.0f, max.x - min.x);

            const float height =
                std::max(0.0f, max.y - min.y);

            return std::min(
                std::max(0.0f, rounding),
                std::min(width, height) * 0.5f);
        }

        void drawCornerCover(
                ImDrawList* draw_list,
                const ImVec2& min,
                const ImVec2& max,
                float radius,
                ImU32 color,
                int corner) {
            if (radius <= 0.0f) {
                return;
            }

            draw_list->PathClear();
            constexpr float pi = 3.14159265358979323846f;

            // Filled paths use clockwise winding. Each path describes only
            // the part of a corner square lying outside the rounded panel.
            switch (corner) {
                case 0: { // Top-left.
                    const ImVec2 center(
                        min.x + radius,
                        min.y + radius);

                    draw_list->PathLineTo(
                        ImVec2(min.x, min.y));

                    // Top tangent -> left tangent.
                    draw_list->PathArcTo(
                        center,
                        radius,
                        pi * 1.5f,
                        pi,
                        0);
                    break;
                }

                case 1: { // Top-right.
                    const ImVec2 center(
                        max.x - radius,
                        min.y + radius);

                    draw_list->PathLineTo(
                        ImVec2(max.x, min.y));

                    // Right tangent -> top tangent.
                    draw_list->PathArcTo(
                        center,
                        radius,
                        0.0f,
                        -pi * 0.5f,
                        0);
                    break;
                }

                case 2: { // Bottom-right.
                    const ImVec2 center(
                        max.x - radius,
                        max.y - radius);

                    draw_list->PathLineTo(
                        ImVec2(max.x, max.y));

                    // Bottom tangent -> right tangent.
                    draw_list->PathArcTo(
                        center,
                        radius,
                        pi * 0.5f,
                        0.0f,
                        0);
                    break;
                }

                case 3: { // Bottom-left.
                    const ImVec2 center(
                        min.x + radius,
                        max.y - radius);

                    draw_list->PathLineTo(
                        ImVec2(min.x, max.y));

                    // Left tangent -> bottom tangent.
                    draw_list->PathArcTo(
                        center,
                        radius,
                        pi,
                        pi * 0.5f,
                        0);
                    break;
                }

                default:
                    return;
            }

            draw_list->PathFillConcave(color);
        }

        void drawCornerCovers(
                ImDrawList* draw_list,
                const ImVec2& min,
                const ImVec2& max,
                float radius,
                ImU32 outside_color) {
            drawCornerCover(
                draw_list,
                min,
                max,
                radius,
                outside_color,
                0);

            drawCornerCover(
                draw_list,
                min,
                max,
                radius,
                outside_color,
                1);

            drawCornerCover(
                draw_list,
                min,
                max,
                radius,
                outside_color,
                2);

            drawCornerCover(
                draw_list,
                min,
                max,
                radius,
                outside_color,
                3);
        }

        void drawPanelBorder(
                ImDrawList* draw_list,
                const RoundedPanelState& state) {
            if (state.border_thickness <= 0.0f) {
                return;
            }

            const float inset =
                state.border_thickness * 0.5f;

            const ImVec2 border_min(
                state.min.x + inset,
                state.min.y + inset);

            const ImVec2 border_max(
                state.max.x - inset,
                state.max.y - inset);

            if (border_max.x <= border_min.x ||
                border_max.y <= border_min.y) {
                return;
            }

            draw_list->AddRect(
                border_min,
                border_max,
                state.border_color,
                std::max(
                    0.0f,
                    state.rounding - inset),
                ImDrawFlags_RoundCornersAll,
                state.border_thickness);
        }

        void drawBottomCornerCovers(
                ImDrawList* draw_list,
                const ImVec2& min,
                const ImVec2& max,
                float radius,
                ImU32 color) {
            drawCornerCover(
                draw_list,
                min,
                max,
                radius,
                color,
                2); // Bottom-right.

            drawCornerCover(
                draw_list,
                min,
                max,
                radius,
                color,
                3); // Bottom-left.
        }

    } // namespace

    IMGUIX_IMPL_INLINE bool BeginRoundedPanel(
            const char* id,
            const ImVec2& size,
            const RoundedPanelConfig& config) {
        IM_ASSERT(
            ImGui::GetCurrentContext() != nullptr);

        const ImGuiStyle& style =
            ImGui::GetStyle();

        const ImVec2 available =
            ImGui::GetContentRegionAvail();

        const ImVec2 panel_size(
            size.x > 0.0f
                ? size.x
                : std::max(0.0f, available.x),
            size.y > 0.0f
                ? size.y
                : std::max(0.0f, available.y));

        const ImVec2 panel_min =
            ImGui::GetCursorScreenPos();

        const ImVec2 panel_max(
            panel_min.x + panel_size.x,
            panel_min.y + panel_size.y);

        const float requested_rounding =
            config.rounding >= 0.0f
                ? config.rounding
                : style.ChildRounding;

        const float rounding =
            clampRounding(
                requested_rounding,
                panel_min,
                panel_max);

        const float border_thickness =
            std::max(
                0.0f,
                config.border_thickness >= 0.0f
                    ? config.border_thickness
                    : style.ChildBorderSize);

        const ImVec2 padding(
            config.padding.x >= 0.0f
                ? config.padding.x
                : style.WindowPadding.x,
            config.padding.y >= 0.0f
                ? config.padding.y
                : style.WindowPadding.y);

        const ImVec4 background_color =
            isUnsetColor(config.background_color)
                ? style.Colors[ImGuiCol_ChildBg]
                : config.background_color;

        const ImVec4 outside_color =
            isUnsetColor(config.outside_color)
                ? style.Colors[ImGuiCol_ChildBg]
                : config.outside_color;

        const ImVec4 border_color =
            isUnsetColor(config.border_color)
                ? style.Colors[ImGuiCol_Border]
                : config.border_color;

        const ImU32 background_u32 =
            ImGui::GetColorU32(background_color);

        // The real child background stays disabled. Draw the visual rounded
        // surface in the parent first; the child itself remains rectangular
        // and is visually clipped during EndRoundedPanel().
        ImGui::GetWindowDrawList()->AddRectFilled(
            panel_min,
            panel_max,
            background_u32,
            rounding,
            ImDrawFlags_RoundCornersAll);

        RoundedPanelState state;
        state.min = panel_min;
        state.max = panel_max;
        state.rounding = rounding;
        state.border_thickness =
            border_thickness;
        state.background_color =
            background_u32;
        state.outside_color =
            ImGui::GetColorU32(outside_color);
        state.border_color =
            ImGui::GetColorU32(border_color);
        state.clip_mode =
            config.clip_mode;

        rounded_panel_stack().push_back(state);

        // WindowPadding configures the child being created. Restore the
        // caller's style before rendering content so nested windows and
        // popups keep their own theme metrics.
        ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding,
            padding);

        const bool child_open = ImGui::BeginChild(
            id,
            panel_size,
            ImGuiChildFlags_AlwaysUseWindowPadding,
            ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse);

        ImGui::PopStyleVar();

        if (config.clip_mode ==
                RoundedPanelClipMode::CoverCorners &&
            border_thickness > 0.0f) {
            // The border is drawn inside the panel bounds. Prevent flush
            // descendants (tables, scrolling children, etc.) from painting
            // underneath its straight sections. Rounded corners are restored
            // separately in EndRoundedPanel().
            const float inset =
                std::max(1.0f, border_thickness);

            const ImVec2 clip_min(
                panel_min.x + inset,
                panel_min.y + inset);

            const ImVec2 clip_max(
                panel_max.x - inset,
                panel_max.y - inset);

            if (clip_max.x > clip_min.x &&
                clip_max.y > clip_min.y) {
                ImGui::PushClipRect(
                    clip_min,
                    clip_max,
                    true);

                rounded_panel_stack()
                    .back()
                    .content_clip_pushed = true;
            }
        }

        return child_open;
    }

    IMGUIX_IMPL_INLINE void EndRoundedPanel() {
        auto& stack =
            rounded_panel_stack();

        IM_ASSERT(!stack.empty());

        if (stack.empty()) {
            return;
        }

        const RoundedPanelState state =
            stack.back();

        stack.pop_back();

        if (state.content_clip_pushed) {
            ImGui::PopClipRect();
        }

        ImGui::EndChild();

        ImDrawList* draw_list =
            ImGui::GetForegroundDrawList();

        if (state.clip_mode ==
            RoundedPanelClipMode::CoverCorners) {
            const float content_inset =
                std::max(
                    1.0f,
                    state.border_thickness);

            const ImVec2 inner_min(
                state.min.x + content_inset,
                state.min.y + content_inset);

            const ImVec2 inner_max(
                state.max.x - content_inset,
                state.max.y - content_inset);

            if (inner_max.x > inner_min.x &&
                inner_max.y > inner_min.y) {
                const float inner_rounding =
                    std::max(
                        0.0f,
                        state.rounding -
                            content_inset);

                // Only protect the bottom inner corners.
                // Top inner corners must remain rectangular so the
                // table header / scrollbar area does not get an
                // unintended top-right arc again.
                drawBottomCornerCovers(
                    draw_list,
                    inner_min,
                    inner_max,
                    inner_rounding,
                    state.background_color);
            }

            // Restore pixels physically outside the panel silhouette.
            drawCornerCovers(
                draw_list,
                state.min,
                state.max,
                state.rounding,
                state.outside_color);
        }

        // Final rounded panel border.
        drawPanelBorder(
            draw_list,
            state);
    }

} // namespace ImGuiX::Widgets
