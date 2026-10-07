#pragma once
#ifndef _IMGUIX_WIDGETS_CONTENT_SURFACE_HPP_INCLUDED
#define _IMGUIX_WIDGETS_CONTENT_SURFACE_HPP_INCLUDED

/// \file content_surface.hpp
/// \brief Generic rounded page-surface container.

#include <imgui.h>

namespace ImGuiX::Widgets {

    /// \brief Layout settings for BeginContentSurface().
    ///
    /// The widget owns only the child surface.  Outer margins belong to the
    /// shell that places the surface, while page-specific content remains the
    /// caller's responsibility.
    struct ContentSurfaceConfig {
        ImVec2 size{0.0f, 0.0f};

        float rounding{-1.0f};
        ImVec2 padding{-1.0f, -1.0f};

        ImGuiChildFlags child_flags{ImGuiChildFlags_AlwaysUseWindowPadding};
        ImGuiWindowFlags window_flags{ImGuiWindowFlags_NoDecoration};
    };

    /// \brief Begin a reusable rounded content surface.
    ///
    /// Negative rounding and padding components inherit the active ImGui
    /// style.  The call must be paired with EndContentSurface() even when it
    /// returns false, following the normal BeginChild()/EndChild() contract.
    /// \param id Child-window identifier.
    /// \param config Surface size, metrics, and child flags.
    /// \return True when the surface contents should be submitted.
    bool BeginContentSurface(
        const char* id,
        const ContentSurfaceConfig& config = {});

    /// \brief Finish the most recently opened content surface.
    void EndContentSurface();

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#   include "content_surface.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_CONTENT_SURFACE_HPP_INCLUDED
