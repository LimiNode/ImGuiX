#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/plot/plot_overlay.hpp>

#include <imgui.h>
#include <implot.h>

#include <cmath>
#include <iostream>

namespace {

    constexpr float kEpsilon = 0.01f;

    bool nearly_equal(const ImVec2& left, const ImVec2& right) {
        return std::fabs(left.x - right.x) <= kEpsilon && std::fabs(left.y - right.y) <= kEpsilon;
    }

    bool clip_matches(const ImVec4& clip, const ImVec2& min, const ImVec2& max) {
        return std::fabs(clip.x - min.x) <= kEpsilon && std::fabs(clip.y - min.y) <= kEpsilon &&
               std::fabs(clip.z - max.x) <= kEpsilon && std::fabs(clip.w - max.y) <= kEpsilon;
    }

}  // namespace

int main() {
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(640.0f, 480.0f);
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char* pixels = nullptr;
    int atlas_width = 0;
    int atlas_height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &atlas_width, &atlas_height);

    ImVec2 first_plot_pos{};
    ImVec2 first_plot_size{};
    ImVec2 first_button_min{};
    ImVec2 first_button_max{};
    ImVec2 cursor_before_overlay{};
    ImVec2 cursor_after_overlay{};
    bool first_callback_called = false;
    bool second_callback_called = false;
    bool popup_visible = false;

    const auto render_frame = [&](const ImVec2& mouse_pos, const bool mouse_down) {
        io.AddMousePosEvent(mouse_pos.x, mouse_pos.y);
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, mouse_down);
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(560.0f, 380.0f), ImGuiCond_Always);
        ImGui::Begin("##plot_overlay_test", nullptr, ImGuiWindowFlags_NoDecoration);

        ImGui::SetCursorPos(ImVec2(28.0f, 32.0f));
        if (ImPlot::BeginPlot("##overlay_first_plot", ImVec2(230.0f, 180.0f),
                              ImPlotFlags_NoLegend)) {
            first_plot_pos = ImPlot::GetPlotPos();
            first_plot_size = ImPlot::GetPlotSize();
            ImPlot::SetupAxis(ImAxis_X1, nullptr);
            ImPlot::SetupAxis(ImAxis_Y1, nullptr);
            const double x_values[] = {0.0, 1.0, 2.0};
            const double y_values[] = {1.0, 2.0, 1.5};
            ImPlot::PlotLine("series", x_values, y_values, 3);

            cursor_before_overlay = ImGui::GetCursorScreenPos();
            ImGuiX::Widgets::PlotOverlayConfig config;
            config.padding = ImVec2(6.0f, 6.0f);
            ImGuiX::Widgets::PlotOverlay(config, [&](const auto& overlay) {
                first_callback_called = true;
                if (!nearly_equal(overlay.plot_pos, first_plot_pos) ||
                    !nearly_equal(overlay.plot_size, first_plot_size)) {
                    first_callback_called = false;
                }
                if (ImGui::Button("##overlay_button", ImVec2(54.0f, 0.0f))) {
                    ImGui::OpenPopup("##overlay_popup");
                }
                first_button_min = ImGui::GetItemRectMin();
                first_button_max = ImGui::GetItemRectMax();
                ImGui::SameLine();
                ImGui::SetNextItemWidth(62.0f);
                if (ImGui::BeginCombo("##overlay_combo", "H1")) {
                    ImGui::Selectable("H1", true);
                    ImGui::EndCombo();
                }
                if (ImGui::BeginPopup("##overlay_popup")) {
                    popup_visible = true;
                    ImGui::TextUnformatted("popup");
                    ImGui::EndPopup();
                }
            });
            cursor_after_overlay = ImGui::GetCursorScreenPos();
            ImPlot::EndPlot();
        }

        ImGui::SameLine(0.0f, 18.0f);
        if (ImPlot::BeginPlot("##overlay_second_plot", ImVec2(230.0f, 180.0f),
                              ImPlotFlags_NoLegend)) {
            ImGuiX::Widgets::PlotOverlayConfig config;
            config.padding = ImVec2(4.0f, 4.0f);
            ImGuiX::Widgets::PlotOverlay(config, [&](const auto&) {
                second_callback_called = true;
                ImGui::TextUnformatted("second");
            });
            ImPlot::EndPlot();
        }

        ImGui::End();
        ImGui::Render();
    };

    render_frame(ImVec2(-100.0f, -100.0f), false);
    if (!first_callback_called || !second_callback_called ||
        !nearly_equal(cursor_before_overlay, cursor_after_overlay)) {
        std::cerr << "plot overlay did not preserve callback/layout state\n";
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        return 1;
    }

    bool found_plot_clip = false;
    for (const ImDrawList* draw_list : ImGui::GetDrawData()->CmdLists) {
        for (const ImDrawCmd& command : draw_list->CmdBuffer) {
            if (command.ElemCount > 0 &&
                clip_matches(command.ClipRect, first_plot_pos,
                             ImVec2(first_plot_pos.x + first_plot_size.x,
                                    first_plot_pos.y + first_plot_size.y))) {
                found_plot_clip = true;
            }
        }
    }
    if (!found_plot_clip) {
        std::cerr << "plot overlay draw commands were not clipped to the plot\n";
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        return 1;
    }

    const ImVec2 button_center((first_button_min.x + first_button_max.x) * 0.5f,
                               (first_button_min.y + first_button_max.y) * 0.5f);
    render_frame(button_center, true);
    render_frame(button_center, false);
    if (!popup_visible) {
        std::cerr << "plot overlay button did not open its popup\n";
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        return 1;
    }

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    return 0;
}
