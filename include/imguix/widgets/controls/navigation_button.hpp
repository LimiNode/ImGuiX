#pragma once
#ifndef _IMGUIX_WIDGETS_NAVIGATION_BUTTON_HPP_INCLUDED
#define _IMGUIX_WIDGETS_NAVIGATION_BUTTON_HPP_INCLUDED

/// \file navigation_button.hpp
/// \brief Rounded navigation control with theme-derived state surfaces.

#include <imgui.h>

namespace ImGuiX::Widgets {

    /// \brief Appearance configuration for NavigationButton.
    struct NavigationButtonConfig {
        ImVec2 size{0.0f, 0.0f}; ///< Zero component uses natural ImGui size.
        ImVec2 frame_padding{-1.0f, -1.0f}; ///< Negative component uses current style.
        float rounding{-1.0f}; ///< Negative value uses style.FrameRounding.
    };

    /// \brief Rounded navigation button with persistent selected state.
    /// \param label Visible UTF-8 label and optional hidden ImGui ID suffix.
    /// \param selected Whether the navigation destination is currently selected.
    /// \param config Appearance configuration.
    /// \return True when activated.
    bool NavigationButton(
        const char* label,
        bool selected,
        const NavigationButtonConfig& config = {});

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#   include "navigation_button.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_NAVIGATION_BUTTON_HPP_INCLUDED
