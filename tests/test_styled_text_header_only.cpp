#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/misc/styled_text.hpp>

#include <imgui.h>

#include <cmath>
#include <iostream>

namespace {

    bool nearly_equal(const float lhs, const float rhs) {
        return std::fabs(lhs - rhs) < 0.01f;
    }

    bool has_color(const ImU32 color) {
        const ImDrawData* data = ImGui::GetDrawData();
        if (data == nullptr) {
            return false;
        }
        for (int list_index = 0; list_index < data->CmdListsCount; ++list_index) {
            for (const ImDrawVert& vertex : data->CmdLists[list_index]->VtxBuffer) {
                if (vertex.col == color) {
                    return true;
                }
            }
        }
        return false;
    }

}  // namespace

int main() {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(640.0f, 480.0f);
    io.DeltaTime = 1.0f / 60.0f;
    ImFontConfig large_font_config;
    large_font_config.SizePixels = 22.0f;
    ImFont* large_font = io.Fonts->AddFontDefault(&large_font_config);
    unsigned char* pixels = nullptr;
    int atlas_width = 0;
    int atlas_height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &atlas_width, &atlas_height);

    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f));
    ImGui::SetNextWindowSize(ImVec2(560.0f, 240.0f));
    ImGui::Begin("##styled_text_test", nullptr,
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

    ImFont* saved_font = ImGui::GetFont();
    const ImVec4 saved_color = ImGui::GetStyle().Colors[ImGuiCol_Text];
    const ImVec4 color{0.1f, 0.8f, 0.2f, 1.0f};
    const ImVec2 expected = ImGui::CalcTextSize("Balance");
    const ImVec2 size = ImGuiX::Widgets::StyledText(
        ImGuiX::Widgets::StyledTextConfig{"Balance", saved_font, color});

    if (!nearly_equal(size.x, expected.x) || !nearly_equal(size.y, expected.y) ||
        ImGui::GetFont() != saved_font ||
        ImGui::GetStyle().Colors[ImGuiCol_Text].x != saved_color.x ||
        ImGui::GetStyle().Colors[ImGuiCol_Text].y != saved_color.y ||
        ImGui::GetStyle().Colors[ImGuiCol_Text].z != saved_color.z ||
        ImGui::GetStyle().Colors[ImGuiCol_Text].w != saved_color.w) {
        ImGui::End();
        ImGui::Render();
        ImGui::DestroyContext();
        std::cerr << "StyledText did not preserve geometry or ImGui state\n";
        return 1;
    }

    const ImVec2 overload_size = ImGuiX::Widgets::StyledText("Equity", nullptr, color);
    if (overload_size.x <= 0.0f) {
        ImGui::End();
        ImGui::Render();
        ImGui::DestroyContext();
        std::cerr << "StyledText overload did not reserve geometry\n";
        return 1;
    }

    const ImVec2 native_size = ImGuiX::Widgets::StyledText(
        ImGuiX::Widgets::StyledTextConfig{"Native UTF-8: Прибыль", large_font, color});
    if (native_size.y <= expected.y || ImGui::GetFont() != saved_font) {
        ImGui::End();
        ImGui::Render();
        ImGui::DestroyContext();
        std::cerr << "StyledText did not use the explicitly registered native font\n";
        return 1;
    }

    ImGui::End();
    ImGui::Render();
    const bool color_ok = has_color(ImGui::GetColorU32(color));
    ImGui::DestroyContext();
    if (!color_ok) {
        std::cerr << "StyledText did not render the explicit color\n";
        return 1;
    }
    return 0;
}
