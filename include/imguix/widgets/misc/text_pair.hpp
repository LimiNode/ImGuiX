#pragma once
#ifndef _IMGUIX_WIDGETS_MISC_TEXT_PAIR_HPP_INCLUDED
#define _IMGUIX_WIDGETS_MISC_TEXT_PAIR_HPP_INCLUDED

/// \file text_pair.hpp
/// \brief Draw two independently styled text parts on one top-aligned row.

#include <imgui.h>

#include <string_view>

namespace ImGuiX::Widgets {

    /// \brief Text and presentation attributes for one part of a TextPair.
    struct TextPart final {
        std::string_view text{};
        ImFont* font = nullptr;
        ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        float font_size = 0.0f;
    };

    /// \brief Configuration for two independently styled text parts.
    struct TextPairConfig final {
        TextPart left{};
        TextPart right{};
        float spacing = 6.0f;
        float right_offset_y = 0.0f;
    };

    /// \brief Draw two text parts with their top edges aligned.
    /// \param config Text, fonts, colors, spacing, and optional right offset.
    /// \return The total occupied size reserved in the current ImGui layout.
    /// \note A zero font size uses the part font's native size, or the active
    ///       ImGui font size when the part font is null.
    ImVec2 TextPair(const TextPairConfig& config);

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#include "text_pair.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_MISC_TEXT_PAIR_HPP_INCLUDED
