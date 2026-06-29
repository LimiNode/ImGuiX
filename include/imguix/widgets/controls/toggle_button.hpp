#pragma once
#ifndef _IMGUIX_WIDGETS_TOGGLE_BUTTON_HPP_INCLUDED
#define _IMGUIX_WIDGETS_TOGGLE_BUTTON_HPP_INCLUDED

/// \file toggle_button.hpp
/// \brief Animated toggle switch for ImGui.

#include <imgui.h>

namespace ImGuiX::Widgets {

    /// \brief Configuration for ToggleButton geometry and colors.
    struct ToggleButtonConfig {
        ImVec2 size = ImVec2(0.0f, 0.0f);   ///< Toggle size in pixels; <= 0 dimensions are derived.
        ImVec2 offset = ImVec2(0.0f, 0.0f); ///< Visual and hitbox offset from current cursor position.
        bool restore_cursor_after_offset = true; ///< Restore cursor after applying offset.
        float height = 0.0f;                ///< Legacy height override; used when size.y <= 0.

        ImVec4 off_color = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);         ///< Track color when disabled.
        ImVec4 off_hovered_color = ImVec4(0.78f, 0.78f, 0.78f, 1.0f); ///< Hovered track color when disabled.
        ImVec4 on_color = ImVec4(0.56f, 0.83f, 0.26f, 1.0f);          ///< Track color when enabled.
        ImVec4 on_hovered_color = ImVec4(0.64f, 0.83f, 0.34f, 1.0f);  ///< Hovered track color when enabled.
        ImVec4 knob_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);           ///< Knob fill color.
    };

    /// \brief Draw animated toggle switch.
    /// \param id Unique string identifier.
    /// \param state Pointer to boolean storing current state.
    /// \param config Geometry and color configuration.
    /// \code{.cpp}
    /// bool enabled = false;
    /// ToggleButton("enabled", &enabled);
    /// \endcode
    void ToggleButton(const char* id, bool* state, const ToggleButtonConfig& config = {});

} // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#   include "toggle_button.ipp"
#endif

#endif // _IMGUIX_WIDGETS_TOGGLE_BUTTON_HPP_INCLUDED
