#pragma once
#ifndef _IMGUIX_WIDGETS_TEXT_CENTER_HPP_INCLUDED
#define _IMGUIX_WIDGETS_TEXT_CENTER_HPP_INCLUDED

/// \file text_center.hpp
/// \brief Helpers to center text within the current content region (single-line, unformatted, wrapped).
///
/// ImGui 1.92.* compatible: uses only public API + <cstdio> for formatting.

#include <imgui.h>
#include <string>

namespace ImGuiX::Widgets {

    /// \brief Center a single-line formatted text using ImGui::Text.
    /// \param fmt Format string for std::vsnprintf.
    /// \param ... printf-style arguments.
    /// \note If text width exceeds available width, falls back to left alignment.
    void TextCenteredFmt(const char* fmt, ...);

    /// \brief Center an unformatted string (no printf parsing).
    /// \param text UTF-8 string to render.
    void TextUnformattedCentered(const char* text);

    /// \brief Center a wrapped text block by placing it into a centered child.
    /// \param text Text to render.
    /// \param wrap_width Wrap width in pixels; if <= 0 uses available width.
    /// \note Creates centered child of the target width and calls TextWrapped inside.
    void TextWrappedCentered(const char* text, float wrap_width = 0.0f);

    /// \brief Center each line of a wrapped/multiline text block.
    /// \param text Text to render.
    /// \param wrap_width Wrap width in pixels; if <= 0 uses available width.
    /// \note Explicit newlines are preserved; long words wider than the wrap width are left aligned.
    void TextWrappedLinesCentered(const char* text, float wrap_width = 0.0f);
    
    /// \brief Overload for std::string (unformatted).
    /// \param s UTF-8 string to render.
    inline void TextUnformattedCentered(const std::string& s) {
        TextUnformattedCentered(s.c_str());
    }

    /// \brief Overload for std::string (wrapped).
    /// \param s Text to render.
    /// \param wrap_width Wrap width in pixels; if <= 0 uses available width.
    inline void TextWrappedCentered(const std::string& s, float wrap_width = 0.0f) {
        TextWrappedCentered(s.c_str(), wrap_width);
    }

    /// \brief Overload for std::string (wrapped line-by-line centered).
    /// \param s Text to render.
    /// \param wrap_width Wrap width in pixels; if <= 0 uses available width.
    inline void TextWrappedLinesCentered(const std::string& s, float wrap_width = 0.0f) {
        TextWrappedLinesCentered(s.c_str(), wrap_width);
    }

#ifdef IMGUIX_DEMO
    /// \brief Render demo for centered text helpers.
    inline void DemoTextCenter() {
        static std::string email;
        TextCenteredFmt(
            "Welcome, %s!", email.empty() ? "guest" : email.c_str());
        TextUnformattedCentered("This line is centered.");
        TextWrappedCentered(
            "This is a long message that demonstrates how wrapped text can be visually centered "
            "by placing it inside a centered child region.",
            420.0f
        );
        TextWrappedLinesCentered(
            "This helper wraps text and centers every produced line independently.",
            420.0f
        );
    }
#endif

} // namespace ImGuiX::Widgets
#ifdef IMGUIX_HEADER_ONLY
#   include "text_center.ipp"
#endif

#endif // _IMGUIX_WIDGETS_TEXT_CENTER_HPP_INCLUDED
