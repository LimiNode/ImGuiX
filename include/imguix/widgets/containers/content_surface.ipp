#include <algorithm>

#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE bool BeginContentSurface(
        const char* id,
        const ContentSurfaceConfig& config) {
        IM_ASSERT(ImGui::GetCurrentContext() != nullptr);

        const ImGuiStyle& style = ImGui::GetStyle();
        const float rounding = std::max(
            0.0f,
            config.rounding >= 0.0f ? config.rounding : style.ChildRounding);
        const ImVec2 padding(
            config.padding.x >= 0.0f ? config.padding.x : style.WindowPadding.x,
            config.padding.y >= 0.0f ? config.padding.y : style.WindowPadding.y);

        // BeginChild() snapshots these metrics into the child window. Restore
        // the parent style immediately so popups and nested children inherit
        // the normal theme instead of the page surface's padding/rounding.
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
        const bool open =
            ImGui::BeginChild(id, config.size, config.child_flags, config.window_flags);
        ImGui::PopStyleVar(2);
        return open;
    }

    IMGUIX_IMPL_INLINE void EndContentSurface() {
        ImGui::EndChild();
    }

}  // namespace ImGuiX::Widgets
