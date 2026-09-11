#pragma once
#ifndef _IMGUIX_EXTENSIONS_SCOPED_STYLE_HPP_INCLUDED
#define _IMGUIX_EXTENSIONS_SCOPED_STYLE_HPP_INCLUDED

/// \file scoped_style.hpp
/// \brief RAII guards for Dear ImGui style stack entries.

#include <imgui.h>

namespace ImGuiX::Extensions {

    /// \brief Keep one Dear ImGui style-variable override active for a lexical scope.
    class ScopedStyleVar {
       public:
        /// \brief Push a scalar style-variable override.
        /// \param index Style-variable index accepting a scalar value.
        /// \param value Override value.
        ScopedStyleVar(ImGuiStyleVar index, float value);

        /// \brief Push a two-component style-variable override.
        /// \param index Style-variable index accepting an `ImVec2` value.
        /// \param value Override value.
        ScopedStyleVar(ImGuiStyleVar index, const ImVec2& value);

        /// \brief Pop the style-variable override.
        ~ScopedStyleVar();

        ScopedStyleVar(const ScopedStyleVar&) = delete;
        ScopedStyleVar& operator=(const ScopedStyleVar&) = delete;
        ScopedStyleVar(ScopedStyleVar&&) = delete;
        ScopedStyleVar& operator=(ScopedStyleVar&&) = delete;
    };

    /// \brief Keep one Dear ImGui style-color override active for a lexical scope.
    class ScopedStyleColor {
       public:
        /// \brief Push a packed style-color override.
        /// \param index Style-color index.
        /// \param color Packed RGBA override.
        ScopedStyleColor(ImGuiCol index, ImU32 color);

        /// \brief Push a floating-point style-color override.
        /// \param index Style-color index.
        /// \param color Floating-point RGBA override.
        ScopedStyleColor(ImGuiCol index, const ImVec4& color);

        /// \brief Pop the style-color override.
        ~ScopedStyleColor();

        ScopedStyleColor(const ScopedStyleColor&) = delete;
        ScopedStyleColor& operator=(const ScopedStyleColor&) = delete;
        ScopedStyleColor(ScopedStyleColor&&) = delete;
        ScopedStyleColor& operator=(ScopedStyleColor&&) = delete;
    };

}  // namespace ImGuiX::Extensions

#include "scoped_style.ipp"

#endif  // _IMGUIX_EXTENSIONS_SCOPED_STYLE_HPP_INCLUDED
