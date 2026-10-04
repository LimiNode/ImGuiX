#include <imguix/config/build.hpp>
#include <imguix/extensions/scoped_style.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE bool NavigationButton(
        const char* label,
        bool selected,
        const NavigationButtonConfig& config) {
        const ImGuiStyle& style = ImGui::GetStyle();

        const ImVec2 frame_padding(
            config.frame_padding.x >= 0.0f
                ? config.frame_padding.x
                : style.FramePadding.x,
            config.frame_padding.y >= 0.0f
                ? config.frame_padding.y
                : style.FramePadding.y);

        const float rounding =
            config.rounding >= 0.0f
                ? config.rounding
                : style.FrameRounding;

        const ImGuiX::Extensions::ScopedStyleVar frame_padding_scope(
            ImGuiStyleVar_FramePadding,
            frame_padding);

        const ImGuiX::Extensions::ScopedStyleVar rounding_scope(
            ImGuiStyleVar_FrameRounding,
            rounding);

        const ImGuiX::Extensions::ScopedStyleColor button_scope(
            ImGuiCol_Button,
            selected
                ? style.Colors[ImGuiCol_NavHighlight]
                : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        const ImGuiX::Extensions::ScopedStyleColor hovered_scope(
            ImGuiCol_ButtonHovered,
            style.Colors[ImGuiCol_HeaderHovered]);

        const ImGuiX::Extensions::ScopedStyleColor active_scope(
            ImGuiCol_ButtonActive,
            style.Colors[ImGuiCol_HeaderActive]);

        return ImGui::Button(label, config.size);
    }

}  // namespace ImGuiX::Widgets
