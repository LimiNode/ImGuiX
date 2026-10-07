#pragma once
#ifndef _IMGUIX_WIDGETS_ROUNDED_PANEL_HPP_INCLUDED
#define _IMGUIX_WIDGETS_ROUNDED_PANEL_HPP_INCLUDED

/// \file rounded_panel.hpp
/// \brief Rounded panel container with a visual corner-cover clipping mode.

#include <imgui.h>

namespace ImGuiX::Widgets {

    /// \brief Strategy used to keep content away from rounded panel corners.
    enum class RoundedPanelClipMode {
        /// The caller supplies enough padding so content does not reach the corners.
        PaddedContent,
        /// Restore the rounded corner pixels after the child content is rendered.
        CoverCorners,
    };

    /// \brief Appearance and layout configuration for BeginRoundedPanel().
    struct RoundedPanelConfig {
        float rounding{-1.0f}; ///< Negative value uses ImGuiStyle::ChildRounding.
        float border_thickness{-1.0f}; ///< Negative value uses ImGuiStyle::ChildBorderSize.

        ImVec4 background_color{}; ///< Zero alpha uses ImGuiCol_ChildBg.
        ImVec4 outside_color{}; ///< Zero alpha uses ImGuiCol_ChildBg.
        ImVec4 border_color{}; ///< Zero alpha uses ImGuiCol_Border.

        ImVec2 padding{-1.0f, -1.0f}; ///< Negative component uses ImGuiStyle::WindowPadding.

        RoundedPanelClipMode clip_mode{RoundedPanelClipMode::CoverCorners};
    };

    /// \brief Begin a rounded, borderable child panel.
    ///
    /// The panel deliberately does not rely on Dear ImGui's rectangular child
    /// background. In CoverCorners mode it redraws the four rounded corner
    /// surfaces and the border after EndRoundedPanel(), which prevents flush
    /// tables and scrolling content from showing through the corner arcs.
    /// \param id Child-window identifier.
    /// \param size Requested panel size; zero components consume available space.
    /// \param config Panel appearance and clipping policy.
    /// \return True when the child contents should be submitted.
    bool BeginRoundedPanel(
        const char* id,
        const ImVec2& size,
        const RoundedPanelConfig& config = {});

    /// \brief Finish the most recently opened rounded panel.
    void EndRoundedPanel();

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#   include "rounded_panel.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_ROUNDED_PANEL_HPP_INCLUDED
