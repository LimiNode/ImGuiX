#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/misc/text_pair.hpp>

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <iostream>

namespace {

    constexpr float kEpsilon = 0.01f;

    bool nearly_equal(const float lhs, const float rhs) {
        return std::fabs(lhs - rhs) <= kEpsilon;
    }

    ImVec2 measure(const char* text, const float size) {
        ImGui::PushFont(ImGui::GetFont(), size);
        const ImVec2 result = ImGui::CalcTextSize(text);
        ImGui::PopFont();
        return result;
    }

    bool has_vertex_color(const ImU32 color) {
        const ImDrawData* draw_data = ImGui::GetDrawData();
        if (draw_data == nullptr) {
            return false;
        }
        for (int list_index = 0; list_index < draw_data->CmdListsCount; ++list_index) {
            const ImDrawList* draw_list = draw_data->CmdLists[list_index];
            for (const ImDrawVert& vertex : draw_list->VtxBuffer) {
                if (vertex.col == color) {
                    return true;
                }
            }
        }
        return false;
    }

    bool run_test() {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(640.0f, 480.0f);
        io.DeltaTime = 1.0f / 60.0f;
        unsigned char* pixels = nullptr;
        int atlas_width = 0;
        int atlas_height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &atlas_width, &atlas_height);

        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f));
        ImGui::SetNextWindowSize(ImVec2(560.0f, 300.0f));
        ImGui::Begin("##text_pair_test", nullptr,
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

        const ImVec2 first_origin = ImGui::GetCursorScreenPos();
        const ImVec2 left_size = measure("17 500.00 USD", 24.0f);
        const ImVec2 right_size = measure("▲ +500.00", 12.0f);

        ImGuiX::Widgets::TextPairConfig first_config;
        first_config.left = ImGuiX::Widgets::TextPart{"17 500.00 USD", ImGui::GetFont(),
                                                      ImVec4(0.95f, 0.95f, 0.95f, 1.0f), 24.0f};
        first_config.right = ImGuiX::Widgets::TextPart{u8"▲ +500.00", ImGui::GetFont(),
                                                       ImVec4(0.1f, 0.8f, 0.2f, 1.0f), 12.0f};
        first_config.spacing = 5.0f;
        const ImVec2 first_size = ImGuiX::Widgets::TextPair(first_config);

        const ImVec2 expected_first(left_size.x + 5.0f + right_size.x,
                                    std::max(left_size.y, right_size.y));
        if (!nearly_equal(first_size.x, expected_first.x) ||
            !nearly_equal(first_size.y, expected_first.y) ||
            !nearly_equal(ImGui::GetItemRectSize().x, expected_first.x) ||
            !nearly_equal(ImGui::GetItemRectSize().y, expected_first.y)) {
            ImGui::End();
            ImGui::Render();
            ImGui::DestroyContext();
            std::cerr << "TextPair did not reserve the measured first row\n";
            return false;
        }

        const ImVec2 second_origin = ImGui::GetCursorScreenPos();
        ImGuiX::Widgets::TextPairConfig second_config;
        second_config.left = ImGuiX::Widgets::TextPart{"short", ImGui::GetFont(),
                                                       ImVec4(0.2f, 0.4f, 1.0f, 1.0f), 12.0f};
        second_config.right = ImGuiX::Widgets::TextPart{"offset", ImGui::GetFont(),
                                                        ImVec4(1.0f, 0.5f, 0.1f, 1.0f), 20.0f};
        second_config.spacing = 3.0f;
        second_config.right_offset_y = 2.0f;
        const ImVec2 second_size = ImGuiX::Widgets::TextPair(second_config);
        const ImVec2 second_left_size = measure("short", 12.0f);
        const ImVec2 second_right_size = measure("offset", 20.0f);
        const ImVec2 expected_second(second_left_size.x + 3.0f + second_right_size.x,
                                     std::max(second_left_size.y, 2.0f + second_right_size.y));

        const bool geometry_ok = nearly_equal(second_size.x, expected_second.x) &&
                                 nearly_equal(second_size.y, expected_second.y) &&
                                 second_origin.y >= first_origin.y + expected_first.y;

        const ImVec2 third_origin = ImGui::GetCursorScreenPos();
        ImGuiX::Widgets::TextPairConfig third_config;
        third_config.left = ImGuiX::Widgets::TextPart{"left", ImGui::GetFont(),
                                                      ImVec4(0.8f, 0.8f, 0.8f, 1.0f), 14.0f};
        third_config.right = ImGuiX::Widgets::TextPart{"above", ImGui::GetFont(),
                                                       ImVec4(0.9f, 0.6f, 0.1f, 1.0f), 10.0f};
        third_config.spacing = 2.0f;
        third_config.right_offset_y = -4.0f;
        const ImVec2 third_size = ImGuiX::Widgets::TextPair(third_config);
        const ImVec2 third_left_size = measure("left", 14.0f);
        const ImVec2 third_right_size = measure("above", 10.0f);
        const float third_bottom = std::max(third_left_size.y, -4.0f + third_right_size.y);
        const ImVec2 expected_third(third_left_size.x + 2.0f + third_right_size.x,
                                    third_bottom + 4.0f);
        const ImVec2 third_item_min = ImGui::GetItemRectMin();
        const ImVec2 third_item_max = ImGui::GetItemRectMax();
        const bool negative_offset_ok =
            nearly_equal(third_size.x, expected_third.x) &&
            nearly_equal(third_size.y, expected_third.y) &&
            nearly_equal(third_item_min.y, third_origin.y - 4.0f) &&
            nearly_equal(third_item_max.y, third_origin.y + third_bottom);
        ImGui::End();
        ImGui::Render();

        const bool colors_ok = has_vertex_color(ImGui::GetColorU32(first_config.left.color)) &&
                               has_vertex_color(ImGui::GetColorU32(first_config.right.color)) &&
                               has_vertex_color(ImGui::GetColorU32(second_config.left.color)) &&
                               has_vertex_color(ImGui::GetColorU32(second_config.right.color));

        ImGui::DestroyContext();
        if (!geometry_ok || !negative_offset_ok || !colors_ok) {
            std::cerr << "TextPair did not preserve geometry or independent styling\n";
            return false;
        }
        return true;
    }

}  // namespace

int main() {
    return run_test() ? 0 : 1;
}
