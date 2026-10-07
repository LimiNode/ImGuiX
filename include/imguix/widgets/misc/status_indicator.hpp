#pragma once
#ifndef _IMGUIX_WIDGETS_STATUS_INDICATOR_HPP_INCLUDED
#define _IMGUIX_WIDGETS_STATUS_INDICATOR_HPP_INCLUDED

/// \file status_indicator.hpp
/// \brief Non-interactive colored status dot with an adjacent text label.

#include <imgui.h>

namespace ImGuiX::Widgets {

    /// \brief Layout and visual settings for StatusIndicator.
    struct StatusIndicatorConfig {
        float radius{4.0f};           ///< Indicator radius in pixels.
        float spacing{6.0f};          ///< Gap between the dot and the label in pixels.
        float optical_offset_y{0.0f}; ///< Vertical correction relative to the text line box.
    };

    /// \brief Draw a colored status indicator and its unformatted label.
    /// \param text UTF-8 label rendered next to the indicator.
    /// \param indicator_color Indicator color in ImGui float format.
    /// \param config Indicator geometry and optical correction.
    /// \note Uses one ImGui group so callers can compose it with SameLine(), hover, and tooltips.
    void StatusIndicator(
        const char* text,
        const ImVec4& indicator_color,
        const StatusIndicatorConfig& config = {});

#ifdef IMGUIX_DEMO
    /// \brief Render representative status indicator variants.
    inline void DemoStatusIndicator() {
        const ImVec4 neutral(0.55f, 0.55f, 0.58f, 1.0f);
        const ImVec4 warning(0.95f, 0.70f, 0.05f, 1.0f);
        const ImVec4 success(0.00f, 0.78f, 0.43f, 1.0f);
        const ImVec4 danger(0.90f, 0.02f, 0.20f, 1.0f);

        StatusIndicator("Offline", neutral);
        ImGui::SameLine();
        StatusIndicator("Starting", warning);
        ImGui::SameLine();
        StatusIndicator("Running", success);
        ImGui::SameLine();
        StatusIndicator("Error", danger);

        StatusIndicator(
            "A long status label demonstrates group bounds and wrapping behavior", success);

        StatusIndicatorConfig compact;
        compact.radius = 3.0f;
        compact.spacing = 4.0f;
        StatusIndicator("Compact", neutral, compact);
        ImGui::SameLine();

        StatusIndicatorConfig large;
        large.radius = 7.0f;
        large.spacing = 10.0f;
        large.optical_offset_y = 1.0f;
        StatusIndicator("Large", success, large);

        ImGui::PushFont(nullptr, 14.0f);
        StatusIndicator("Small font", warning);
        ImGui::PopFont();
        ImGui::PushFont(nullptr, 24.0f);
        StatusIndicator("Large font", success);
        ImGui::PopFont();
    }
#endif

} // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#   include "status_indicator.ipp"
#endif

#endif // _IMGUIX_WIDGETS_STATUS_INDICATOR_HPP_INCLUDED
