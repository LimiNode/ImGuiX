#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/containers/section_card.hpp>
#include <imguix/widgets/containers/table_surface.hpp>

#include <imgui_internal.h>

#include <cmath>
#include <stdexcept>

namespace {

    float cross(const ImVec2 left, const ImVec2 right) {
        return left.x * right.y - left.y * right.x;
    }

    void require(const bool condition, const char* message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    ImVec4 sample_pixel(const ImDrawData& draw_data, const ImVec2 point) {
        ImVec4 result(0.0f, 0.0f, 0.0f, 0.0f);
        for (int list_index = 0; list_index < draw_data.CmdListsCount; ++list_index) {
            const ImDrawList& list = *draw_data.CmdLists[list_index];
            for (const ImDrawCmd& command : list.CmdBuffer) {
                if (point.x < command.ClipRect.x || point.y < command.ClipRect.y ||
                    point.x >= command.ClipRect.z || point.y >= command.ClipRect.w) {
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
                    const float wb = cross(point - a.pos, c.pos - a.pos) / area;
                    const float wc = cross(b.pos - a.pos, point - a.pos) / area;
                    const float wa = 1.0f - wb - wc;
                    if (wa < -0.0001f || wb < -0.0001f || wc < -0.0001f) {
                        continue;
                    }
                    const ImVec4 ca = ImGui::ColorConvertU32ToFloat4(a.col);
                    const ImVec4 cb = ImGui::ColorConvertU32ToFloat4(b.col);
                    const ImVec4 cc = ImGui::ColorConvertU32ToFloat4(c.col);
                    const float alpha = wa * ca.w + wb * cb.w + wc * cc.w;
                    result.x = (wa * ca.x + wb * cb.x + wc * cc.x) * alpha +
                               result.x * (1.0f - alpha);
                    result.y = (wa * ca.y + wb * cb.y + wc * cc.y) * alpha +
                               result.y * (1.0f - alpha);
                    result.z = (wa * ca.z + wb * cb.z + wc * cc.z) * alpha +
                               result.z * (1.0f - alpha);
                }
            }
        }
        return result;
    }

    bool near_color(const ImVec4 actual, const ImVec4 expected) {
        return std::fabs(actual.x - expected.x) < 0.08f &&
               std::fabs(actual.y - expected.y) < 0.08f &&
               std::fabs(actual.z - expected.z) < 0.08f;
    }

    void draw_fixture(const bool hover_last_row, const bool open_popup) {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = hover_last_row ? ImVec2(165.0f, 205.0f) : ImVec2(-FLT_MAX, -FLT_MAX);
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(420.0f, 300.0f));
        ImGui::Begin("##table_surface_integration", nullptr, ImGuiWindowFlags_NoDecoration);

        ImGuiX::Widgets::SectionCardConfig card;
        card.title = "Accounts";
        card.panel.background_color = ImVec4(0.12f, 0.14f, 0.18f, 1.0f);
        require(ImGuiX::Widgets::BeginSectionCard("##card", card),
                "SectionCard must be visible in the real table fixture.");
        const ImVec4 table_color(0.24f, 0.28f, 0.34f, 1.0f);
        const ImVec4 outside_color = card.panel.background_color;
        const ImGuiStyle& style = ImGui::GetStyle();
        const ImGuiID table_id = ImGui::GetID("##real_table");
        require(ImGui::BeginTable("##real_table", 3,
                                  ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                                      ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp,
                                  ImVec2(280.0f, 150.0f)),
                "BeginTable must create the integration fixture.");
        ImGuiTable* table = ImGui::GetCurrentTable();
        require((table->Flags & ImGuiTableFlags_BordersOuter) == 0,
                "Integration table must use the custom rounded outer border.");
        const ImGuiX::Widgets::TableSurfaceGeometry geometry{
            table->OuterRect.Min,
            table->OuterRect.Max,
            12.0f,
            ImGui::GetColorU32(table_color),
            ImGui::GetWindowDrawList()->GetClipRectMin(),
            ImGui::GetWindowDrawList()->GetClipRectMax()};
        ImGuiX::Widgets::DrawTableSurfaceBackground(ImGui::GetWindowDrawList(), geometry);
        ImGui::TableSetupColumn("#");
        ImGui::TableSetupColumn("Account");
        ImGui::TableSetupColumn("Status");
        ImGui::TableHeadersRow();
        for (int row = 0; row < 2; ++row) {
            ImGui::TableNextRow();
            for (int column = 0; column < 3; ++column) {
                ImGui::TableSetColumnIndex(column);
                ImGui::TextUnformatted(column == 0 ? (row == 0 ? "1" : "2")
                                                   : (column == 1 ? "Account" : "Ready"));
            }
        }
        ImDrawList* table_draw_list = table->InnerWindow->DrawList;
        ImGui::EndTable();
        ImGuiX::Widgets::CoverTableSurfaceCorners(table_draw_list, geometry);
        ImGuiX::Widgets::DrawTableSurfaceBorder(
            table_draw_list, geometry, ImGui::GetColorU32(ImGuiCol_Border));
        if (open_popup) {
            ImGui::OpenPopup("##table_popup");
        }
        if (ImGui::BeginPopup("##table_popup")) {
            ImGui::TextUnformatted("Columns");
            ImGui::EndPopup();
        }
        ImGuiX::Widgets::EndSectionCard();
        ImGui::End();
        ImGui::Render();

        require(ImGui::FindWindowByName("##table_surface_integration") != nullptr,
                "Real table window must remain in ImGui context.");
        const ImVec2 min = table->OuterRect.Min;
        const ImVec2 max = table->OuterRect.Max;
        require(near_color(sample_pixel(*ImGui::GetDrawData(), min + ImVec2(1.5f, 1.5f)),
                            outside_color),
                "Real table top-left corner must preserve SectionCard surface.");
        require(near_color(sample_pixel(*ImGui::GetDrawData(),
                                        ImVec2((min.x + max.x) * 0.5f, min.y + 1.0f)),
                            ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Border))) ||
                    near_color(sample_pixel(*ImGui::GetDrawData(),
                                            ImVec2((min.x + max.x) * 0.5f, min.y + 2.0f)),
                              ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Border))),
                "Real table must expose the rounded custom outer border.");
        require(near_color(sample_pixel(*ImGui::GetDrawData(), max - ImVec2(1.5f, 1.5f)),
                            outside_color),
                "Real table bottom-right corner must preserve SectionCard surface.");
        (void)table_id;
        (void)style;
    }

}  // namespace

int main() {
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().DisplaySize = ImVec2(420.0f, 300.0f);
    ImGui::GetIO().Fonts->AddFontDefault();
    ImGui::GetIO().Fonts->Build();
    draw_fixture(false, false);
    draw_fixture(true, false);
    draw_fixture(false, true);
    ImGui::DestroyContext();
    return 0;
}
