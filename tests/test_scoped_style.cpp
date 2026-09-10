#include <imguix/extensions.hpp>

#include <imgui.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {

    bool nearlyEqual(float lhs, float rhs) {
        return std::fabs(lhs - rhs) < 0.0001f;
    }

    bool equalVec2(const ImVec2& lhs, const ImVec2& rhs) {
        return nearlyEqual(lhs.x, rhs.x) && nearlyEqual(lhs.y, rhs.y);
    }

    bool equalVec4(const ImVec4& lhs, const ImVec4& rhs) {
        return nearlyEqual(lhs.x, rhs.x) && nearlyEqual(lhs.y, rhs.y) &&
               nearlyEqual(lhs.z, rhs.z) && nearlyEqual(lhs.w, rhs.w);
    }

    void require(bool condition, const char* message) {
        if (!condition) {
            std::cerr << message << '\n';
            std::exit(1);
        }
    }

}  // namespace

int main() {
    static_assert(!std::is_copy_constructible_v<ImGuiX::Extensions::ScopedStyleVar>);
    static_assert(!std::is_copy_assignable_v<ImGuiX::Extensions::ScopedStyleVar>);
    static_assert(!std::is_move_constructible_v<ImGuiX::Extensions::ScopedStyleVar>);
    static_assert(!std::is_move_assignable_v<ImGuiX::Extensions::ScopedStyleVar>);
    static_assert(!std::is_copy_constructible_v<ImGuiX::Extensions::ScopedStyleColor>);
    static_assert(!std::is_copy_assignable_v<ImGuiX::Extensions::ScopedStyleColor>);
    static_assert(!std::is_move_constructible_v<ImGuiX::Extensions::ScopedStyleColor>);
    static_assert(!std::is_move_assignable_v<ImGuiX::Extensions::ScopedStyleColor>);

    ImGuiContext* context = ImGui::CreateContext();
    ImGui::SetCurrentContext(context);

    ImGuiStyle& style = ImGui::GetStyle();
    const float initial_alpha = style.Alpha;
    const ImVec2 initial_padding = style.FramePadding;
    const ImVec4 initial_text_color = style.Colors[ImGuiCol_Text];
    const ImVec4 initial_button_color = style.Colors[ImGuiCol_Button];

    {
        const ImGuiX::Extensions::ScopedStyleVar alpha(ImGuiStyleVar_Alpha, 0.7f);
        require(nearlyEqual(style.Alpha, 0.7f), "scalar style override was not applied");

        {
            const ImGuiX::Extensions::ScopedStyleVar nested_alpha(ImGuiStyleVar_Alpha, 0.3f);
            require(nearlyEqual(style.Alpha, 0.3f), "nested scalar style override was not applied");
        }

        require(nearlyEqual(style.Alpha, 0.7f), "nested scalar style override was not restored");
    }
    require(nearlyEqual(style.Alpha, initial_alpha), "scalar style override was not restored");

    {
        const ImVec2 override_padding(11.0f, 2.0f);
        const ImGuiX::Extensions::ScopedStyleVar frame_padding(ImGuiStyleVar_FramePadding,
                                                               override_padding);
        require(equalVec2(style.FramePadding, override_padding),
                "ImVec2 style override was not applied");
    }
    require(equalVec2(style.FramePadding, initial_padding),
            "ImVec2 style override was not restored");

    {
        const ImU32 packed_color = IM_COL32(10, 20, 30, 40);
        const ImGuiX::Extensions::ScopedStyleColor text_color(ImGuiCol_Text, packed_color);
        require(
            equalVec4(style.Colors[ImGuiCol_Text], ImGui::ColorConvertU32ToFloat4(packed_color)),
            "packed style-color override was not applied");
    }
    require(equalVec4(style.Colors[ImGuiCol_Text], initial_text_color),
            "packed style-color override was not restored");

    {
        const ImVec4 override_color(0.1f, 0.2f, 0.3f, 0.4f);
        const ImGuiX::Extensions::ScopedStyleColor button_color(ImGuiCol_Button, override_color);
        require(equalVec4(style.Colors[ImGuiCol_Button], override_color),
                "ImVec4 style-color override was not applied");
    }
    require(equalVec4(style.Colors[ImGuiCol_Button], initial_button_color),
            "ImVec4 style-color override was not restored");

    ImGui::DestroyContext(context);
    return 0;
}
