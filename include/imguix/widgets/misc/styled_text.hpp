#pragma once
#ifndef _IMGUIX_WIDGETS_MISC_STYLED_TEXT_HPP_INCLUDED
#define _IMGUIX_WIDGETS_MISC_STYLED_TEXT_HPP_INCLUDED

/// \file styled_text.hpp
/// \brief Draw one text item with an explicit native font and color.

#include <imgui.h>

#include <string_view>

namespace ImGuiX::Widgets {

    /// \brief Configuration for a single styled text item.
    struct StyledTextConfig final {
        std::string_view text{};
        ImFont* font = nullptr;  ///< Native registered font; null uses the active font.
        ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    };

    /// \brief Draw one UTF-8 text item without interpreting its contents as a format string.
    /// \param config Text, native font, and color to use.
    /// \return The size reserved in the current ImGui layout.
    /// \note A null font falls back to the active ImGui font. Font roles are resolved by the
    ///       caller's FontManager and passed here as an ImFont pointer.
    ImVec2 StyledText(const StyledTextConfig& config);

    /// \brief Draw one UTF-8 text item using an explicit native font and color.
    /// \param text Text to draw.
    /// \param font Native registered font, or null for the active font.
    /// \param color Text color.
    /// \return The size reserved in the current ImGui layout.
    ImVec2 StyledText(std::string_view text, ImFont* font, const ImVec4& color);

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#include "styled_text.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_MISC_STYLED_TEXT_HPP_INCLUDED
