#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/containers/rounded_panel.hpp>

#include <imgui.h>

#include <cmath>
#include <iostream>

namespace {

    constexpr float kEpsilon = 0.01f;

    bool clip_is_inside(
            const ImVec4& clip,
            const ImVec2& outer_min,
            const ImVec2& outer_max) {
        return clip.x + kEpsilon >= outer_min.x &&
               clip.y + kEpsilon >= outer_min.y &&
               clip.z - kEpsilon <= outer_max.x &&
               clip.w - kEpsilon <= outer_max.y;
    }

}  // namespace

int main() {
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(640.0f, 480.0f);
    io.DeltaTime = 1.0f / 60.0f;

    unsigned char* pixels = nullptr;
    int atlas_width = 0;
    int atlas_height = 0;
    io.Fonts->GetTexDataAsRGBA32(
        &pixels,
        &atlas_width,
        &atlas_height);

    bool cover_open = false;
    float observed_scroll_y = -1.0f;
    int decoration_commands = 0;

    for (int frame = 0; frame < 2; ++frame) {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f));
        ImGui::SetNextWindowSize(ImVec2(420.0f, 240.0f));
        ImGui::Begin(
            "##rounded_panel_clipping_test",
            nullptr,
            ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoResize);

        ImGui::BeginChild(
            "##scrolling_surface",
            ImVec2(360.0f, 140.0f),
            false,
            ImGuiWindowFlags_NoBackground);

        const ImVec2 surface_clip_min =
            ImGui::GetWindowDrawList()->GetClipRectMin();
        const ImVec2 surface_clip_max =
            ImGui::GetWindowDrawList()->GetClipRectMax();

        // Set the scroll target only in the first frame.  The second frame
        // proves that Dear ImGui applied and retained it before we inspect
        // the clipped foreground decorations.
        if (frame == 0) {
            ImGui::SetScrollY(48.0f);
        }

        ImGuiX::Widgets::RoundedPanelConfig cover_config;
        cover_config.border_thickness = 2.0f;
        cover_config.rounding = 8.0f;
        cover_config.clip_mode =
            ImGuiX::Widgets::RoundedPanelClipMode::CoverCorners;

        ImGui::SetCursorPosY(0.0f);
        cover_open =
            ImGuiX::Widgets::BeginRoundedPanel(
                "##cover_panel",
                ImVec2(300.0f, 120.0f),
                cover_config);
        if (cover_open) {
            ImGui::TextUnformatted("cover panel");

            ImGuiX::Widgets::RoundedPanelConfig padded_config;
            padded_config.border_thickness = 1.0f;
            padded_config.rounding = 5.0f;
            padded_config.clip_mode =
                ImGuiX::Widgets::RoundedPanelClipMode::PaddedContent;

            ImGuiX::Widgets::BeginRoundedPanel(
                "##nested_padded_panel",
                ImVec2(220.0f, 64.0f),
                padded_config);
            ImGui::TextUnformatted("nested padded panel");
            ImGuiX::Widgets::EndRoundedPanel();
        }
        ImGuiX::Widgets::EndRoundedPanel();

        ImGui::SetCursorPosY(150.0f);
        ImGuiX::Widgets::BeginRoundedPanel(
            "##second_cover_panel",
            ImVec2(300.0f, 120.0f),
            cover_config);
        ImGui::TextUnformatted("second cover panel");
        ImGuiX::Widgets::EndRoundedPanel();

        observed_scroll_y = ImGui::GetScrollY();
        ImGui::EndChild();
        ImGui::End();
        ImGui::Render();

        if (frame == 1) {
            const ImDrawList* foreground =
                ImGui::GetForegroundDrawList();
            for (const ImDrawCmd& command : foreground->CmdBuffer) {
                if (command.ElemCount == 0) {
                    continue;
                }

                ++decoration_commands;
                if (!clip_is_inside(
                        command.ClipRect,
                        surface_clip_min,
                        surface_clip_max)) {
                    std::cerr <<
                        "rounded panel decoration escaped the parent clip rect\n";
                    ImGui::DestroyContext();
                    return 1;
                }
            }
        }
    }

    ImGui::DestroyContext();

    if (!cover_open) {
        std::cerr << "rounded panel child was unexpectedly clipped\n";
        return 1;
    }
    if (observed_scroll_y < 47.99f) {
        std::cerr << "rounded panel clipping test did not retain scrolling\n";
        return 1;
    }
    if (decoration_commands < 2) {
        std::cerr << "rounded panel decorations were not rendered\n";
        return 1;
    }
    return 0;
}
