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
        const ImVec2 text_size = ImGui::CalcTextSize(label);
        const float label_height = std::max(line_height, text_size.y);
        const float required_dot_height =
            2.0f * (radius + std::abs(config.optical_offset_y));
        const float row_height = std::max(label_height, required_dot_height);
        const float label_offset_y = (row_height - label_height) * 0.5f;
        const float center_y = std::floor(
            origin.y + row_height * 0.5f + config.optical_offset_y + 0.5f);

        ImGui::GetWindowDrawList()->AddCircleFilled(
            ImVec2(origin.x + radius, center_y), radius,
            ImGui::GetColorU32(indicator_color));

        ImGui::SetCursorScreenPos(
            ImVec2(
                origin.x + radius * 2.0f + spacing,
                origin.y + label_offset_y));
        ImGui::TextUnformatted(label);

        // Extend the group bounds to include the lower half of the reserved
        // row without relying on SetCursorPos() to grow a parent boundary.
        if (label_offset_y > 0.0f)
            ImGui::Dummy(ImVec2(0.0f, label_offset_y));

        ImGui::EndGroup();
    }

} // namespace ImGuiX::Widgets
