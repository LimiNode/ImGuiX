#include <imguix/config/build.hpp>

#include <algorithm>
#include <vector>

namespace ImGuiX::Widgets {

    namespace detail {

        inline std::vector<bool>& formSectionStack() {
            static thread_local std::vector<bool> stack;
            return stack;
        }

    }  // namespace detail

    IMGUIX_IMPL_INLINE bool BeginFormSection(const char* id, const FormSectionConfig& config) {
        const char* stable_id = id != nullptr ? id : "##form_section";
        const ImGuiID state_id = ImGui::GetID(stable_id);
        bool is_open = ImGui::GetStateStorage()->GetBool(state_id, config.default_open);
        const float header_height =
            std::max(ImGui::GetFrameHeight(), config.header_height > 0.0f
                                                  ? config.header_height
                                                  : ImGui::GetFrameHeight());
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float width = std::max(0.0f, ImGui::GetContentRegionAvail().x);

        ImGui::PushID(stable_id);
        const bool pressed = ImGui::InvisibleButton("##toggle", ImVec2(width, header_height));
        const bool hovered = ImGui::IsItemHovered();
        if (pressed) {
            is_open = !is_open;
            ImGui::GetStateStorage()->SetBool(state_id, is_open);
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        if (hovered) {
            ImVec4 hover_color = ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered];
            hover_color.w *= 0.55f;
            draw_list->AddRectFilled(origin, origin + ImVec2(width, header_height),
                                     ImGui::GetColorU32(hover_color),
                                     ImGui::GetStyle().FrameRounding);
        }
        const char* arrow = is_open ? u8"\uE5CE" : u8"\uE5CF";
        const ImVec2 arrow_size = ImGui::CalcTextSize(arrow);
        const float text_y = origin.y + (header_height - ImGui::GetTextLineHeight()) * 0.5f;
        draw_list->AddText(ImVec2(origin.x, text_y), ImGui::GetColorU32(ImGuiCol_Text), arrow);
        draw_list->AddText(ImVec2(origin.x + arrow_size.x + ImGui::GetStyle().ItemSpacing.x,
                                  text_y),
                           ImGui::GetColorU32(ImGuiCol_Text),
                           config.title != nullptr ? config.title : "");
        if (config.draw_separator) {
            const float separator_y = origin.y + header_height - 1.0f;
            draw_list->AddLine(ImVec2(origin.x, separator_y),
                               ImVec2(origin.x + width, separator_y),
                               ImGui::GetColorU32(ImGuiCol_Border));
        }

        detail::formSectionStack().push_back(is_open);
        if (is_open) {
            const float indent = config.indent > 0.0f
                                     ? config.indent
                                     : ImGui::GetStyle().ItemSpacing.x;
            ImGui::Indent(indent);
        }
        ImGui::PopID();
        return is_open;
    }

    IMGUIX_IMPL_INLINE void EndFormSection() {
        auto& stack = detail::formSectionStack();
        IM_ASSERT(!stack.empty());
        if (stack.empty()) {
            return;
        }
        const bool is_open = stack.back();
        stack.pop_back();
        if (is_open) {
            ImGui::Unindent();
        }
    }

}  // namespace ImGuiX::Widgets
