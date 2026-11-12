#pragma once
#ifndef _IMGUIX_WIDGETS_TABLE_SURFACE_HPP_INCLUDED
#define _IMGUIX_WIDGETS_TABLE_SURFACE_HPP_INCLUDED

/// \file table_surface.hpp
/// \brief Rounded draw geometry for rectangular table contents.

#include <imgui.h>

namespace ImGuiX::Widgets {

    /// \brief Geometry and effective clipping for a rounded table surface.
    struct TableSurfaceGeometry final {
        ImVec2 min{};
        ImVec2 max{};
        float rounding{0.0f};
        ImU32 surface_color{0};
        ImVec2 clip_min{};
        ImVec2 clip_max{};
    };

    /// \brief Draw a rounded rectangular surface below table contents.
    void DrawTableSurfaceBackground(ImDrawList* draw_list,
                                    const TableSurfaceGeometry& geometry);

    /// \brief Restore rounded corner pixels above rectangular table contents.
    ///
    /// The supplied clip rectangle is intersected with the surface bounds so
    /// the helper remains safe inside scrolling child windows.
    void CoverTableSurfaceCorners(ImDrawList* draw_list,
                                  const TableSurfaceGeometry& geometry);

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#include "table_surface.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_TABLE_SURFACE_HPP_INCLUDED
