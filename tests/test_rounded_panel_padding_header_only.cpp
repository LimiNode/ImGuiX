#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/containers/rounded_panel.hpp>

#include <imgui.h>

#include <cmath>
#include <iostream>

namespace {

    bool nearly_equal(const float lhs, const float rhs) {
        return std::fabs(lhs - rhs) < 0.01f;
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
    io.Fonts->GetTexDataAsRGBA32(&pixels, &atlas_width, &atlas_height);

    ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 original_padding = style.WindowPadding;
    const ImVec2 panel_padding(18.0f, 16.0f);

    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f));
    ImGui::SetNextWindowSize(ImVec2(400.0f, 300.0f));
    ImGui::Begin("##rounded_panel_padding_test", nullptr,
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

    ImGuiX::Widgets::RoundedPanelConfig config;
    config.padding = panel_padding;
    config.border_thickness = 0.0f;
    config.clip_mode = ImGuiX::Widgets::RoundedPanelClipMode::PaddedContent;

    const bool panel_open =
        ImGuiX::Widgets::BeginRoundedPanel("##panel", ImVec2(240.0f, 160.0f), config);
    const ImVec2 child_position = ImGui::GetWindowPos();
    const ImVec2 content_position = ImGui::GetCursorScreenPos();
    const ImVec2 active_style_padding = style.WindowPadding;

    ImGuiX::Widgets::EndRoundedPanel();
    ImGui::End();
    ImGui::Render();

    const bool content_uses_requested_padding =
        nearly_equal(content_position.x - child_position.x, panel_padding.x) &&
        nearly_equal(content_position.y - child_position.y, panel_padding.y);
    const bool caller_style_restored = nearly_equal(active_style_padding.x, original_padding.x) &&
                                       nearly_equal(active_style_padding.y, original_padding.y);

    ImGui::DestroyContext();

    if (!panel_open) {
        std::cerr << "rounded panel child was unexpectedly clipped\n";
        return 1;
    }
    if (!content_uses_requested_padding) {
        std::cerr << "rounded panel did not apply its requested child padding\n";
        return 1;
    }
    if (!caller_style_restored) {
        std::cerr << "rounded panel leaked its child padding into caller content\n";
        return 1;
    }
    return 0;
}
