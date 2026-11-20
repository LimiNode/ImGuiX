#include <imguix/config/build.hpp>

#include <algorithm>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE void DrawTableSurfaceBackground(
        ImDrawList* draw_list, const TableSurfaceGeometry& geometry) {
        if (draw_list == nullptr || geometry.max.x <= geometry.min.x ||
            geometry.max.y <= geometry.min.y) {
            return;
        }
        draw_list->PushClipRect(geometry.clip_min, geometry.clip_max, true);
        draw_list->AddRectFilled(geometry.min, geometry.max, geometry.surface_color,
                                 geometry.rounding, ImDrawFlags_RoundCornersAll);
        draw_list->PopClipRect();
    }

    IMGUIX_IMPL_INLINE void DrawTableSurfaceBorder(
        ImDrawList* draw_list, const TableSurfaceGeometry& geometry,
        const ImU32 border_color, const float thickness) {
        if (draw_list == nullptr || geometry.max.x <= geometry.min.x ||
            geometry.max.y <= geometry.min.y || thickness <= 0.0f) {
            return;
        }
        const ImVec2 clip_min(std::max(geometry.min.x, geometry.clip_min.x),
                              std::max(geometry.min.y, geometry.clip_min.y));
        const ImVec2 clip_max(std::min(geometry.max.x, geometry.clip_max.x),
                              std::min(geometry.max.y, geometry.clip_max.y));
        if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) {
            return;
        }
        const float half = thickness * 0.5f;
        const float radius = std::max(0.0f, geometry.rounding - half);
        draw_list->PushClipRect(clip_min, clip_max, false);
        // Coordinates already describe the stroke centre. AddRect adds its
        // own half-pixel inset, splitting a 1px edge over two translucent rows.
        // Geometry AA also avoids depending on the backend's line atlas.
        const ImDrawListFlags saved_flags = draw_list->Flags;
        draw_list->Flags &= ~ImDrawListFlags_AntiAliasedLinesUseTex;
        draw_list->PathRect(geometry.min + ImVec2(half, half),
                            geometry.max - ImVec2(half, half), radius,
                            ImDrawFlags_RoundCornersAll);
        draw_list->PathStroke(border_color, ImDrawFlags_Closed, thickness);
        draw_list->Flags = saved_flags;
        draw_list->PopClipRect();
    }

    namespace detail {

        IMGUIX_IMPL_INLINE void draw_table_corner(ImDrawList* draw_list,
                                                  const TableSurfaceGeometry& geometry,
                                                  const int corner) {
            constexpr float pi = 3.14159265358979323846f;
            const float radius = std::max(
                0.0f, std::min(geometry.rounding, std::min(geometry.max.x - geometry.min.x,
                                                           geometry.max.y - geometry.min.y) *
                                                      0.5f));
            if (radius <= 0.0f) {
                return;
            }
            draw_list->PathClear();
            switch (corner) {
                case 0:
                    draw_list->PathLineTo(geometry.min);
                    draw_list->PathArcTo(ImVec2(geometry.min.x + radius, geometry.min.y + radius),
                                         radius, pi * 1.5f, pi, 0);
                    break;
                case 1:
                    draw_list->PathLineTo(ImVec2(geometry.max.x, geometry.min.y));
                    draw_list->PathArcTo(ImVec2(geometry.max.x - radius, geometry.min.y + radius),
                                         radius, 0.0f, -pi * 0.5f, 0);
                    break;
                case 2:
                    draw_list->PathLineTo(geometry.max);
                    draw_list->PathArcTo(ImVec2(geometry.max.x - radius, geometry.max.y - radius),
                                         radius, pi * 0.5f, 0.0f, 0);
                    break;
                case 3:
                    draw_list->PathLineTo(ImVec2(geometry.min.x, geometry.max.y));
                    draw_list->PathArcTo(ImVec2(geometry.min.x + radius, geometry.max.y - radius),
                                         radius, pi, pi * 0.5f, 0);
                    break;
                default:
                    return;
            }
            draw_list->PathFillConcave(geometry.outside_color != 0 ? geometry.outside_color
                                                                    : geometry.surface_color);
        }

    }  // namespace detail

    IMGUIX_IMPL_INLINE void CoverTableSurfaceCorners(
        ImDrawList* draw_list, const TableSurfaceGeometry& geometry) {
        if (draw_list == nullptr || geometry.max.x <= geometry.min.x ||
            geometry.max.y <= geometry.min.y) {
            return;
        }
        const ImVec2 clip_min(std::max(geometry.min.x, geometry.clip_min.x),
                              std::max(geometry.min.y, geometry.clip_min.y));
        const ImVec2 clip_max(std::min(geometry.max.x, geometry.clip_max.x),
                              std::min(geometry.max.y, geometry.clip_max.y));
        if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) {
            return;
        }
        draw_list->PushClipRect(clip_min, clip_max, false);
        for (int corner = 0; corner < 4; ++corner) {
            detail::draw_table_corner(draw_list, geometry, corner);
        }
        draw_list->PopClipRect();
    }

}  // namespace ImGuiX::Widgets
