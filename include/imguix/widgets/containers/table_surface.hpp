#pragma once
#ifndef _IMGUIX_WIDGETS_TABLE_SURFACE_HPP_INCLUDED
#define _IMGUIX_WIDGETS_TABLE_SURFACE_HPP_INCLUDED

/// \file table_surface.hpp
/// \brief Rounded draw geometry for rectangular table contents.

#include <imgui.h>

namespace ImGuiX::Widgets {

    /// \brief Geometry and effective clipping for a rounded table surface.
    struct TableSurfaceGeometry final {
        ImVec2 min{}; ///< Screen-space table bounds captured by the caller.
        ImVec2 max{}; ///< Bottom-right table corner.
        float rounding{0.0f}; ///< Corner radius, clamped to the table dimensions.
        ImU32 surface_color{0}; ///< Opaque host surface color used outside the rounded silhouette.
        ImVec2 clip_min{}; ///< Effective host clip, captured before entering the table.
        ImVec2 clip_max{}; ///< Bottom-right effective host clip boundary.
        ImU32 outside_color{0}; ///< Parent surface color restored at rounded corners.
    };

    /// \brief Draw a rounded rectangular surface below table contents.
    /// \param draw_list Table-owned draw list; never a global foreground list.
    /// \param geometry Table bounds, host color, and effective clipping.
    void DrawTableSurfaceBackground(ImDrawList* draw_list,
                                    const TableSurfaceGeometry& geometry);

    /// \brief Draw a rounded outer border for a table surface.
    /// \param draw_list Table-owned draw list; never a global foreground list.
    /// \param geometry Table bounds and effective clipping.
    /// \param border_color Border color in packed ImGui format.
    /// \param thickness Border thickness in pixels.
    void DrawTableSurfaceBorder(ImDrawList* draw_list,
                                const TableSurfaceGeometry& geometry,
                                ImU32 border_color,
                                float thickness = 1.0f);

    /// \brief Restore rounded corner pixels above rectangular table contents.
    ///
    /// The supplied clip rectangle is intersected with the surface bounds so
    /// the helper remains safe inside scrolling child windows.
    /// \param draw_list Table-owned draw list after EndTable merges its channels.
    /// \param geometry Table bounds, host color, and effective clipping.
    void CoverTableSurfaceCorners(ImDrawList* draw_list,
                                  const TableSurfaceGeometry& geometry);

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#include "table_surface.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_TABLE_SURFACE_HPP_INCLUDED
