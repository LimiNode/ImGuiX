#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>
#include <imgui_internal.h>
#include <imguix/widgets/containers/rounded_panel.hpp>
#include <imguix/widgets/containers/table_surface.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

    constexpr ImU32 kOutside = IM_COL32(35, 48, 65, 255);
    constexpr ImU32 kSurface = IM_COL32(210, 215, 220, 255);
    constexpr ImU32 kBorder = IM_COL32(255, 80, 20, 255);

    float cross(ImVec2 a, ImVec2 b) { return a.x * b.y - a.y * b.x; }

    // Sample final triangles and atlas alpha, including textured line AA. The
    // oracle deliberately consumes Render() output, not pre-merge channels.
    ImVec4 sample(const ImDrawData& data, ImVec2 p) {
        ImVec4 result = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
        const ImTextureData* texture_data = ImGui::GetIO().Fonts->TexData;
        const unsigned char* atlas = texture_data->Pixels;
        const int width = texture_data->Width, height = texture_data->Height;
        for (const ImDrawList* list : data.CmdLists) {
            for (const ImDrawCmd& cmd : list->CmdBuffer) {
                if (cmd.UserCallback || p.x < cmd.ClipRect.x || p.y < cmd.ClipRect.y ||
                    p.x >= cmd.ClipRect.z || p.y >= cmd.ClipRect.w) continue;
                for (unsigned i = 0; i < cmd.ElemCount; i += 3) {
                    const auto& a = list->VtxBuffer[cmd.VtxOffset + list->IdxBuffer[cmd.IdxOffset + i]];
                    const auto& b = list->VtxBuffer[cmd.VtxOffset + list->IdxBuffer[cmd.IdxOffset + i + 1]];
                    const auto& c = list->VtxBuffer[cmd.VtxOffset + list->IdxBuffer[cmd.IdxOffset + i + 2]];
                    const float area = cross(b.pos - a.pos, c.pos - a.pos);
                    if (std::fabs(area) < 1e-5f) continue;
                    const float wb = cross(p - a.pos, c.pos - a.pos) / area;
                    const float wc = cross(b.pos - a.pos, p - a.pos) / area;
                    const float wa = 1 - wb - wc;
                    const auto owns = [area](ImVec2 from, ImVec2 to) {
                        if (area < 0) std::swap(from, to);
                        return to.y < from.y || (to.y == from.y && to.x > from.x);
                    };
                    if (wa < -1e-5f || wb < -1e-5f || wc < -1e-5f ||
                        (std::fabs(wa) < 1e-5f && !owns(b.pos, c.pos)) ||
                        (std::fabs(wb) < 1e-5f && !owns(c.pos, a.pos)) ||
                        (std::fabs(wc) < 1e-5f && !owns(a.pos, b.pos))) continue;
                    const auto ca = ImGui::ColorConvertU32ToFloat4(a.col);
                    const auto cb = ImGui::ColorConvertU32ToFloat4(b.col);
                    const auto cc = ImGui::ColorConvertU32ToFloat4(c.col);
                    const ImVec2 uv = a.uv * wa + b.uv * wb + c.uv * wc;
                    const float tx = uv.x * width - 0.5f, ty = uv.y * height - 0.5f;
                    const int x = static_cast<int>(std::floor(tx)), y = static_cast<int>(std::floor(ty));
                    const auto texel = [&](int xx, int yy) {
                        const int index = std::clamp(yy, 0, height - 1) * width +
                                          std::clamp(xx, 0, width - 1);
                        return atlas[index * texture_data->BytesPerPixel +
                                     (texture_data->BytesPerPixel == 4 ? 3 : 0)] / 255.0f;
                    };
                    const float fx = tx - x, fy = ty - y;
                    const float texture = (texel(x, y) * (1-fx) + texel(x+1, y) * fx) * (1-fy) +
                                          (texel(x, y+1) * (1-fx) + texel(x+1, y+1) * fx) * fy;
                    const float alpha = (ca.w * wa + cb.w * wb + cc.w * wc) * texture;
                    result.x = (ca.x * wa + cb.x * wb + cc.x * wc) * alpha + result.x * (1-alpha);
                    result.y = (ca.y * wa + cb.y * wb + cc.y * wc) * alpha + result.y * (1-alpha);
                    result.z = (ca.z * wa + cb.z * wb + cc.z * wc) * alpha + result.z * (1-alpha);
                }
            }
        }
        return result;
    }

    void writeImage(const std::string& path, const ImDrawData& data) {
        // PPM keeps this optional evidence export independent of image libraries.
        std::ofstream file(path, std::ios::binary);
        file << "P6\n640 420\n255\n";
        for (int y = 0; y < 420; ++y) for (int x = 0; x < 640; ++x) {
            const ImVec4 color = sample(data, ImVec2(x + 0.5f, y + 0.5f));
            for (float component : {color.x, color.y, color.z}) file.put(static_cast<char>(component * 255 + 0.5f));
        }
    }

    std::vector<ImVec4> verify(float scale, bool scroll, bool clipped, bool pearl,
                              const char* output, bool is_grid_visible = true,
                              bool has_translucent_border = false) {
        const ImU32 outside_color = pearl ? IM_COL32(235, 240, 248, 255) : kOutside;
        ImGui::CreateContext();
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = ImVec2(640, 420);
        io.Fonts->AddFontDefault();
        io.Fonts->Build();
        ImGui::GetStyle().ScaleAllSizes(scale);
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(outside_color);
        ImGui::GetStyle().Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1f, 0.5f, 0.85f, 1);
        ImGui::GetStyle().Colors[ImGuiCol_TableRowBg] = ImVec4(0.8f, 0.1f, 0.7f, 1);
        ImGui::GetStyle().Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.1f, 0.7f, 0.3f, 1);
        ImGui::GetStyle().Colors[ImGuiCol_TableBorderLight] =
            is_grid_visible ? ImVec4(0.9f, 0.15f, 0.1f, 0.8f) : ImVec4();
        ImGui::GetStyle().Colors[ImGuiCol_TableBorderStrong] =
            ImGui::GetStyle().Colors[ImGuiCol_TableBorderLight];
        const ImU32 border_color = has_translucent_border ? IM_COL32(255, 80, 20, 128) : kBorder;
        ImGuiX::Widgets::TableSurfaceGeometry geometry;
        for (int frame = 0; frame < 3; ++frame) {
            ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(io.DisplaySize);
            ImGui::Begin("host", nullptr, ImGuiWindowFlags_NoDecoration);
            ImGuiX::Widgets::RoundedPanelConfig card;
            card.background_color = ImGui::ColorConvertU32ToFloat4(outside_color);
            card.outside_color = card.background_color;
            card.draw_decorations_in_child = true;
            card.border_thickness = 0;
            card.padding = ImVec2(18, 18);
            ImGuiX::Widgets::BeginRoundedPanel("card", ImVec2(600, 350), card);
            if (clipped) {
                const ImVec2 position = ImGui::GetCursorScreenPos();
                ImGui::PushClipRect(position + ImVec2(0, 9), position + ImVec2(500, 145), true);
            }
            const ImVec2 clip_min = ImGui::GetWindowDrawList()->GetClipRectMin();
            const ImVec2 clip_max = ImGui::GetWindowDrawList()->GetClipRectMax();
            ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner |
                                    ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
                                    ImGuiTableFlags_Hideable;
            if (scroll) flags |= ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
            if (ImGui::BeginTable("table", 3, flags, ImVec2(530, scroll ? 190 : 0))) {
                ImGuiTable* table = ImGui::GetCurrentTable();
                geometry = {table->OuterRect.Min, table->OuterRect.Max, 12 * scale,
                            kSurface, clip_min, clip_max, outside_color};
                ImDrawList* draw_list = table->InnerWindow->DrawList;
                ImGuiX::Widgets::DrawTableSurfaceBackground(draw_list, geometry);
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 24);
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("State");
                ImGui::TableHeadersRow();
                for (int row = 0; row < (scroll ? 15 : 4); ++row) {
                    ImGui::TableNextRow(0, 28 * scale);
                    for (int col = 0; col < 3; ++col) {
                        ImGui::TableSetColumnIndex(col);
                        ImGui::Text("%d", row);
                    }
                }
                ImGui::EndTable();
                // The public geometry must follow EndTable's final content height.
                geometry.max = table->OuterRect.Max;
                ImGuiX::Widgets::CoverTableSurfaceCorners(draw_list, geometry);
                ImGuiX::Widgets::DrawTableSurfaceBorder(draw_list, geometry, border_color);
            }
            if (clipped) ImGui::PopClipRect();
            ImGuiX::Widgets::EndRoundedPanel();
            ImGui::End();
            ImGui::Render();
        }
        if (output) writeImage(output, *ImGui::GetDrawData());
        // All four straight edges must have opaque centre coverage. Merely
        // finding border-coloured vertices misses double-inset AA regressions.
        const ImVec4 expected_border = ImGui::ColorConvertU32ToFloat4(kBorder);
        const ImVec2 centre = (geometry.min + geometry.max) * 0.5f;
        for (ImVec2 pixel : {ImVec2(centre.x, geometry.min.y + 0.5f),
                             ImVec2(centre.x, geometry.max.y - 0.5f),
                             ImVec2(geometry.min.x + 0.5f, centre.y),
                             ImVec2(geometry.max.x - 0.5f, centre.y)}) {
            if (pixel.x < geometry.clip_min.x || pixel.x >= geometry.clip_max.x ||
                pixel.y < geometry.clip_min.y || pixel.y >= geometry.clip_max.y) continue;
            const ImVec4 actual = sample(*ImGui::GetDrawData(), pixel);
            if (!has_translucent_border && (std::fabs(actual.x - expected_border.x) > 0.1f ||
                std::fabs(actual.y - expected_border.y) > 0.1f ||
                std::fabs(actual.z - expected_border.z) > 0.1f)) {
                throw std::runtime_error("Table border has incomplete centre coverage");
            }
        }
        const ImVec4 outside = ImGui::ColorConvertU32ToFloat4(outside_color);
        int leaks = 0;
        for (int corner = 0; corner < 4; ++corner) {
            for (float x = 0.5f; x < geometry.rounding; x += 1) {
                for (float y = 0.5f; y < geometry.rounding; y += 1) {
                    if (std::hypot(geometry.rounding - x, geometry.rounding - y) < geometry.rounding + 1.25f) continue;
                    const ImVec2 pixel(corner == 1 || corner == 2 ? geometry.max.x - x : geometry.min.x + x,
                                       corner >= 2 ? geometry.max.y - y : geometry.min.y + y);
                    const ImVec4 actual = sample(*ImGui::GetDrawData(), pixel);
                    if (std::fabs(actual.x-outside.x) > 0.04f || std::fabs(actual.y-outside.y) > 0.04f ||
                        std::fabs(actual.z-outside.z) > 0.04f) ++leaks;
                }
            }
        }
        std::vector<ImVec4> edge_pixels;
        // Compare the final outer stroke with and without the native grid.
        // An opaque test border would conceal an overlapping final-column line.
        for (float inset : {-0.5f, 0.5f, 1.5f}) {
            for (ImVec2 pixel : {ImVec2(geometry.min.x + inset, centre.y + 3.0f),
                                 ImVec2(geometry.max.x - inset, centre.y + 3.0f),
                                 ImVec2(centre.x + 3.0f, geometry.min.y + inset),
                                 ImVec2(centre.x + 3.0f, geometry.max.y - inset)}) {
                edge_pixels.push_back(sample(*ImGui::GetDrawData(), pixel));
            }
        }
        ImGui::DestroyContext();
        if (leaks) throw std::runtime_error("Table corner pixels leaked: " + std::to_string(leaks));
        return edge_pixels;
    }
}

int main(int argc, char** argv) {
    try {
        for (float scale : {1.0f, 1.5f, 2.0f}) {
            for (bool scroll : {false, true}) {
                const auto with_grid = verify(scale, scroll, false, false, nullptr, true, true);
                const auto without_grid = verify(scale, scroll, false, false, nullptr, false, true);
                for (std::size_t index = 0; index < with_grid.size(); ++index) {
                    const ImVec4 actual = with_grid[index];
                    const ImVec4 expected = without_grid[index];
                    if (std::fabs(actual.x - expected.x) > 0.04f ||
                        std::fabs(actual.y - expected.y) > 0.04f ||
                        std::fabs(actual.z - expected.z) > 0.04f) {
                        throw std::runtime_error("Native grid overlaps rounded outer border: scale=" +
                                                 std::to_string(scale) + " pixel=" +
                                                 std::to_string(index));
                    }
                }
            }
        }
        for (float scale : {1.0f, 1.25f, 1.5f, 2.0f}) for (bool scroll : {false, true})
            for (bool clipped : {false, true})
                for (bool pearl : {false, true})
                    verify(scale, scroll, clipped, pearl,
                           argc > 1 && scale == 1 && !scroll && !clipped && !pearl ? argv[1] : nullptr);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
