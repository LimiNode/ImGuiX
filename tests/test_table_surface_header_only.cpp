#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/containers/table_surface.hpp>

#include <cmath>
#include <stdexcept>

int main() {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(320.0f, 240.0f);
    io.Fonts->AddFontDefault();
    io.Fonts->Build();

    ImGui::NewFrame();
    ImGui::Begin("##table_surface_test", nullptr, ImGuiWindowFlags_NoDecoration);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImGuiX::Widgets::TableSurfaceGeometry geometry{
        ImVec2(10.0f, 20.0f), ImVec2(210.0f, 140.0f), 12.0f, IM_COL32(31, 47, 63, 255),
        ImVec2(0.0f, 0.0f), ImVec2(180.0f, 100.0f)};
    const int vertex_start = draw_list->VtxBuffer.Size;
    const int command_start = draw_list->CmdBuffer.Size;
    ImGuiX::Widgets::DrawTableSurfaceBackground(draw_list, geometry);
    ImGuiX::Widgets::CoverTableSurfaceCorners(draw_list, geometry);
    if (draw_list->VtxBuffer.Size <= vertex_start) {
        throw std::runtime_error("Table surface must submit corner geometry.");
    }
    bool has_intersection_clip = false;
    for (int index = command_start; index < draw_list->CmdBuffer.Size; ++index) {
        const ImDrawCmd& command = draw_list->CmdBuffer[index];
        has_intersection_clip =
            has_intersection_clip || (std::fabs(command.ClipRect.x - 10.0f) < 0.1f &&
                                      std::fabs(command.ClipRect.y - 20.0f) < 0.1f &&
                                      std::fabs(command.ClipRect.z - 180.0f) < 0.1f &&
                                      std::fabs(command.ClipRect.w - 100.0f) < 0.1f);
    }
    if (!has_intersection_clip) {
        throw std::runtime_error("Table surface must intersect host and surface clips.");
    }
    ImGui::End();
    ImGui::Render();
    ImGui::DestroyContext();
    return 0;
}
