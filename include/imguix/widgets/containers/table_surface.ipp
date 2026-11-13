#include <imguix/config/build.hpp>

#include <algorithm>
#include <cmath>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE void DrawTableSurfaceBackground(
        ImDrawList* draw_list, const TableSurfaceGeometry& geometry) {
        if (draw_list == nullptr || geometry.max.x <= geometry.min.x ||
            geometry.max.y <= geometry.min.y) {
            return;
        }
        draw_list->AddRectFilled(geometry.min, geometry.max, geometry.surface_color,
                                 geometry.rounding, ImDrawFlags_RoundCornersAll);
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
        draw_list->AddRect(geometry.min + ImVec2(half, half),
                           geometry.max - ImVec2(half, half), border_color, radius,
                           ImDrawFlags_RoundCornersAll, thickness);
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
            constexpr int arc_segments = 16;
            constexpr float coverage = 1.0f;
            ImVec2 corner_point;
            ImVec2 center;
            float start_angle = 0.0f;
            float end_angle = 0.0f;
            switch (corner) {
                case 0:
                    corner_point = geometry.min - ImVec2(coverage, coverage);
                    center = ImVec2(geometry.min.x + radius, geometry.min.y + radius);
                    start_angle = pi * 1.5f;
                    end_angle = pi;
                    break;
                case 1:
                    corner_point = ImVec2(geometry.max.x + coverage, geometry.min.y - coverage);
                    center = ImVec2(geometry.max.x - radius, geometry.min.y + radius);
                    start_angle = 0.0f;
                    end_angle = -pi * 0.5f;
                    break;
                case 2:
                    corner_point = geometry.max + ImVec2(coverage, coverage);
                    center = ImVec2(geometry.max.x - radius, geometry.max.y - radius);
                    start_angle = pi * 0.5f;
                    end_angle = 0.0f;
                    break;
                case 3:
                    corner_point = ImVec2(geometry.min.x - coverage, geometry.max.y + coverage);
                    center = ImVec2(geometry.min.x + radius, geometry.max.y - radius);
                    start_angle = pi;
                    end_angle = pi * 0.5f;
                    break;
                default:
                    return;
            }

            // Convex pieces avoid winding-sensitive concave triangulation and keep
            // the outermost corner pixels fully covered after table RowBg rendering.
            const ImDrawListFlags original_flags = draw_list->Flags;
            draw_list->Flags &= ~ImDrawListFlags_AntiAliasedFill;
            ImVec2 previous(center.x + std::cos(start_angle) * radius,
                             center.y + std::sin(start_angle) * radius);
            for (int segment = 1; segment <= arc_segments; ++segment) {
                const float t = static_cast<float>(segment) / static_cast<float>(arc_segments);
                const float angle = start_angle + (end_angle - start_angle) * t;
                const ImVec2 current(center.x + std::cos(angle) * radius,
                                     center.y + std::sin(angle) * radius);
                draw_list->AddTriangleFilled(
                    corner_point, previous, current,
                    geometry.outside_color != 0 ? geometry.outside_color : geometry.surface_color);
                previous = current;
            }
            draw_list->Flags = original_flags;
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
