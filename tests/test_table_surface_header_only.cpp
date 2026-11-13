#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/containers/table_surface.hpp>

#include <imgui_internal.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

namespace {

    float cross(const ImVec2 a, const ImVec2 b) {
        return a.x * b.y - a.y * b.x;
    }

    // Deterministic single-pixel rasterization of the final indexed triangles,
    // including command clipping and interpolated AA coverage. All primitives
    // in this fixture use the atlas white texel; no font/GPU dependency is needed.
    ImVec4 rasterPixel(const ImDrawList& list, const ImVec2 pixel, ImVec4 result) {
        for (const ImDrawCmd& command : list.CmdBuffer) {
            if (pixel.x < command.ClipRect.x || pixel.y < command.ClipRect.y ||
                pixel.x >= command.ClipRect.z || pixel.y >= command.ClipRect.w) {
                continue;
            }
            for (unsigned int index = 0; index + 2 < command.ElemCount; index += 3) {
                const ImDrawVert& a = list.VtxBuffer[command.VtxOffset +
                    list.IdxBuffer[command.IdxOffset + index]];
                const ImDrawVert& b = list.VtxBuffer[command.VtxOffset +
                    list.IdxBuffer[command.IdxOffset + index + 1]];
                const ImDrawVert& c = list.VtxBuffer[command.VtxOffset +
                    list.IdxBuffer[command.IdxOffset + index + 2]];
                const float area = cross(b.pos - a.pos, c.pos - a.pos);
                if (std::fabs(area) < 0.0001f) {
                    continue;
                }
                const float wb = cross(pixel - a.pos, c.pos - a.pos) / area;
                const float wc = cross(b.pos - a.pos, pixel - a.pos) / area;
                const float wa = 1.0f - wb - wc;
                    const auto owns_edge = [area](ImVec2 from, ImVec2 to) {
                        if (area < 0) {
                            std::swap(from, to);
                        }
                        return to.y < from.y || (to.y == from.y && to.x > from.x);
                    };
                    if (wa < -0.00001f || wb < -0.00001f || wc < -0.00001f ||
                        (std::fabs(wa) < 0.00001f && !owns_edge(b.pos, c.pos)) ||
                        (std::fabs(wb) < 0.00001f && !owns_edge(c.pos, a.pos)) ||
                        (std::fabs(wc) < 0.00001f && !owns_edge(a.pos, b.pos))) {
                    continue;
                }
                const ImVec4 ca = ImGui::ColorConvertU32ToFloat4(a.col);
                const ImVec4 cb = ImGui::ColorConvertU32ToFloat4(b.col);
                const ImVec4 cc = ImGui::ColorConvertU32ToFloat4(c.col);
                const float alpha = wa * ca.w + wb * cb.w + wc * cc.w;
                result.x = (wa * ca.x + wb * cb.x + wc * cc.x) * alpha + result.x * (1 - alpha);
                result.y = (wa * ca.y + wb * cb.y + wc * cc.y) * alpha + result.y * (1 - alpha);
                result.z = (wa * ca.z + wb * cb.z + wc * cc.z) * alpha + result.z * (1 - alpha);
            }
        }
        return result;
    }

    bool sameColor(const ImVec4 a, const ImVec4 b) {
        return std::fabs(a.x - b.x) < 0.04f && std::fabs(a.y - b.y) < 0.04f &&
               std::fabs(a.z - b.z) < 0.04f;
    }

    void verifyPixels(const float radius, const bool is_pearl, const bool is_scrolled) {
        const ImU32 outside = is_pearl ? IM_COL32(230, 235, 242, 255) : IM_COL32(55, 55, 55, 255);
        const ImU32 content = IM_COL32(190, 40, 90, 255);
        const ImU32 popup = IM_COL32(40, 130, 220, 255);
        const ImGuiX::Widgets::TableSurfaceGeometry geometry{
            ImVec2(20, 20), ImVec2(220, 160), radius, outside,
            ImVec2(10, is_scrolled ? 24.0f : 10.0f),
            ImVec2(230, is_scrolled ? 156.0f : 180.0f)};
        ImDrawList list(ImGui::GetDrawListSharedData());
        list._ResetForNewFrame();
        list.PushClipRect(geometry.clip_min, geometry.clip_max);
        list.PushTextureID(ImGui::GetIO().Fonts->TexRef);
        ImGuiX::Widgets::DrawTableSurfaceBackground(&list, geometry);
        list.AddRectFilled(geometry.min, geometry.max, content);
        ImGuiX::Widgets::CoverTableSurfaceCorners(&list, geometry);

        const ImVec4 outside_color = ImGui::ColorConvertU32ToFloat4(outside);
        const ImVec4 content_color = ImGui::ColorConvertU32ToFloat4(content);
        for (int corner = 0; corner < 4; ++corner) {
            const bool is_right = corner == 1 || corner == 2;
            const bool is_bottom = corner >= 2;
            for (float x = 0.5f; x < radius; x += 1.0f) {
                for (float y = 0.5f; y < radius; y += 1.0f) {
                    const ImVec2 pixel(is_right ? geometry.max.x - x : geometry.min.x + x,
                                       is_bottom ? geometry.max.y - y : geometry.min.y + y);
                    const bool is_clipped = pixel.y < geometry.clip_min.y ||
                                            pixel.y >= geometry.clip_max.y;
                    const float distance = std::hypot(radius - x, radius - y);
                    if (std::min(x, y) < 1.0f && std::fabs(distance - radius) < 1.0f) {
                        continue;  // Intersecting edge fringes are not a single circular AA ramp.
                    }
                    // Inspect erased pixels, preserved content, and the AA ramp.
                    const float coverage = std::clamp(distance - radius + 0.5f, 0.0f, 1.0f);
                    const ImVec4 expected = is_clipped ? outside_color : ImVec4(
                        outside_color.x * coverage + content_color.x * (1 - coverage),
                        outside_color.y * coverage + content_color.y * (1 - coverage),
                        outside_color.z * coverage + content_color.z * (1 - coverage), 1.0f);
                    const ImVec4 actual = rasterPixel(list, pixel, outside_color);
                    const float tolerance = !is_clipped && std::fabs(distance - radius) < 1.0f
                                                ? 0.12f : 0.04f;
                    if (std::fabs(actual.x - expected.x) > tolerance ||
                        std::fabs(actual.y - expected.y) > tolerance ||
                        std::fabs(actual.z - expected.z) > tolerance) {
                        throw std::runtime_error("Final rounded-table pixels do not match the circular corner: " +
                            std::to_string(corner) + " radius=" + std::to_string(radius) +
                            " x=" + std::to_string(x) + " y=" + std::to_string(y) +
                            " actual=" + std::to_string(actual.x) + " expected=" + std::to_string(expected.x));
                    }
                }
            }
        }
        // A later popup must not be covered by a foreground corner decoration.
        list.AddRectFilled(ImVec2(20, 24), ImVec2(40, 44), popup);
        if (!sameColor(rasterPixel(list, ImVec2(25.5f, 25.5f), outside_color),
                       ImGui::ColorConvertU32ToFloat4(popup))) {
            throw std::runtime_error("Rounded-table decorations must remain below later popup content.");
        }
        bool has_clip = false;
        for (const ImDrawCmd& command : list.CmdBuffer) {
            has_clip = has_clip || (command.ElemCount > 0 &&
                command.ClipRect.x == 20 && command.ClipRect.z == 220 &&
                command.ClipRect.y == std::max(20.0f, geometry.clip_min.y) &&
                command.ClipRect.w == std::min(160.0f, geometry.clip_max.y));
        }
        if (!has_clip) {
            throw std::runtime_error("Corner draw commands must use the surface/host clip intersection.");
        }
        list.PopTextureID();
        list.PopClipRect();
    }

}  // namespace

int main() {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(320, 240);
    io.Fonts->AddFontDefault();
    io.Fonts->Build();
    ImGui::NewFrame();
    for (const float radius : {9.0f, 16.0f, 24.0f}) {
        for (const bool is_pearl : {false, true}) {
            for (const bool is_scrolled : {false, true}) {
                verifyPixels(radius, is_pearl, is_scrolled);
            }
        }
    }
    ImGui::EndFrame();
    ImGui::DestroyContext();
    return 0;
}
