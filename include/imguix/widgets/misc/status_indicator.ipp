#include <algorithm>
#include <cmath>

#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE void StatusIndicator(
        const char* text,
        const ImVec4& indicator_color,
        const StatusIndicatorConfig& config) {
        const float radius = std::max(0.0f, config.radius);
        const float spacing = std::max(0.0f, config.spacing);
        const char* label = text != nullptr ? text : "";

        ImGui::BeginGroup();

        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float line_height = ImGui::GetTextLineHeight();
        const float center_y = std::floor(
            origin.y + line_height * 0.5f + config.optical_offset_y + 0.5f);

        ImGui::GetWindowDrawList()->AddCircleFilled(
            ImVec2(origin.x + radius, center_y), radius,
            ImGui::GetColorU32(indicator_color));

        ImGui::SetCursorScreenPos(
            ImVec2(origin.x + radius * 2.0f + spacing, origin.y));
        ImGui::TextUnformatted(label);

        ImGui::EndGroup();
    }

} // namespace ImGuiX::Widgets
