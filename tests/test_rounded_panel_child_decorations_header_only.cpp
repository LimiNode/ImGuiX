#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/containers/rounded_panel.hpp>

#include <imgui.h>

#include <cmath>
#include <iostream>

namespace {

    constexpr float kEpsilon = 0.01f;

    bool nearly_equal(const float lhs, const float rhs) {
        return std::fabs(lhs - rhs) <= kEpsilon;
    }

    bool clip_matches(
            const ImVec4& clip,
            const ImVec2& expected_min,
            const ImVec2& expected_max) {
        return nearly_equal(clip.x, expected_min.x) &&
               nearly_equal(clip.y, expected_min.y) &&
               nearly_equal(clip.z, expected_max.x) &&
               nearly_equal(clip.w, expected_max.y);
    }

    bool check_height(const float parent_height) {
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

        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f));
        ImGui::SetNextWindowSize(ImVec2(420.0f, parent_height));
        ImGui::Begin(
            "##rounded_panel_child_decorations_test",
            nullptr,
            ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoResize);

        ImGui::BeginChild(
            "##scroll_surface",
            ImVec2(360.0f, parent_height - 40.0f),
            false,
            ImGuiWindowFlags_NoBackground);
        ImGui::SetScrollY(24.0f);

        const ImVec2 surface_clip_min =
            ImGui::GetWindowDrawList()->GetClipRectMin();
        const ImVec2 surface_clip_max =
            ImGui::GetWindowDrawList()->GetClipRectMax();

        // Make the effective parent clip deliberately wider than the child
        // viewport. The border must use this clip, not the child clip.
        const ImVec2 parent_clip_min(
            surface_clip_min.x - 12.0f,
            surface_clip_min.y);
        const ImVec2 parent_clip_max(
            surface_clip_max.x + 12.0f,
            surface_clip_max.y);
        ImGui::PushClipRect(parent_clip_min, parent_clip_max, false);

        ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
        const ImVec2 panel_min = ImGui::GetCursorScreenPos();
        const ImVec2 panel_size(360.0f, parent_height - 80.0f);

        ImGuiX::Widgets::RoundedPanelConfig config;
        config.padding = ImVec2(0.0f, 0.0f);
        config.rounding = 8.0f;
        config.border_thickness = 1.0f;
        config.border_color = ImVec4(0.91f, 0.17f, 0.73f, 1.0f);
        config.draw_decorations_in_child = true;

        const ImU32 border_color =
            ImGui::GetColorU32(config.border_color);
        const bool panel_open =
            ImGuiX::Widgets::BeginRoundedPanel(
                "##full_width_panel",
                panel_size,
                config);
        const ImVec2 child_clip_min =
            ImGui::GetWindowDrawList()->GetClipRectMin();
        const ImVec2 child_clip_max =
            ImGui::GetWindowDrawList()->GetClipRectMax();
        ImDrawList* child_draw_list = ImGui::GetWindowDrawList();
        child_draw_list->Flags |= ImDrawListFlags_AntiAliasedLinesUseTex;
        const ImDrawListFlags child_flags = child_draw_list->Flags;

        if (panel_open) {
            ImGui::Dummy(panel_size);
        }
        ImGuiX::Widgets::EndRoundedPanel();
        const bool child_flags_restored =
            child_draw_list->Flags == child_flags;

        ImGui::PopClipRect();
        ImGui::EndChild();
        ImGui::End();
        ImGui::Render();

        bool found_border = false;
        bool border_used_parent_clip = false;
        bool found_geometry_fringe = false;
        const ImU32 transparent_border_color =
            border_color & ~IM_COL32_A_MASK;
        const ImDrawData* draw_data = ImGui::GetDrawData();
        for (int list_index = 0;
             list_index < draw_data->CmdListsCount;
             ++list_index) {
            const ImDrawList* draw_list = draw_data->CmdLists[list_index];
            for (const ImDrawCmd& command : draw_list->CmdBuffer) {
                bool has_border_vertex = false;
                bool has_geometry_fringe = false;
                for (unsigned int index = 0;
                     index < command.ElemCount;
                     ++index) {
                    const ImDrawIdx draw_index =
                        draw_list->IdxBuffer[command.IdxOffset + index];
                    const ImDrawVert& vertex =
                        draw_list->VtxBuffer[
                            command.VtxOffset + draw_index];
                    if (vertex.col == border_color) {
                        has_border_vertex = true;
                    }
                    if (vertex.col == transparent_border_color) {
                        has_geometry_fringe = true;
                    }
                }
                if (!has_border_vertex) {
                    continue;
                }

                found_border = true;
                found_geometry_fringe |= has_geometry_fringe;
                border_used_parent_clip = clip_matches(
                    command.ClipRect,
                    parent_clip_min,
                    parent_clip_max);
            }
        }

        const bool child_clip_is_narrower =
            child_clip_min.x > parent_clip_min.x + kEpsilon ||
            child_clip_max.x < parent_clip_max.x - kEpsilon;

        ImGui::DestroyContext();

        if (!panel_open || !found_border) {
            std::cerr <<
                "child-local rounded panel border was not rendered\n";
            return false;
        }
        if (!child_clip_is_narrower) {
            std::cerr <<
                "regression fixture did not create a narrower child clip\n";
            return false;
        }
        if (!border_used_parent_clip) {
            std::cerr <<
                "child-local border did not use the saved parent clip\n";
            return false;
        }
        if (!found_geometry_fringe || !child_flags_restored) {
            std::cerr <<
                "panel border did not use geometry AA or restore draw-list flags\n";
            return false;
        }
        return true;
    }

}  // namespace

int main() {
    if (!check_height(180.0f) || !check_height(220.0f) ||
        !check_height(300.0f) || !check_height(400.0f)) {
        return 1;
    }
    return 0;
}
