#include <imguix/config/build.hpp>

#include <algorithm>
#include <cfloat>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE bool EditableLabel(const EditableLabelConfig& config) {
        const char* id = config.id != nullptr ? config.id : "##editable_label";
        const char* icon = config.edit_icon != nullptr ? config.edit_icon : "";
        const float gap = std::max(0.0f, config.gap);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        ImFont* text_font = config.text_font != nullptr ? config.text_font : ImGui::GetFont();
        const float text_size_y = ImGui::GetFontSize();
        const ImVec2 text_size = text_font != nullptr
                                     ? text_font->CalcTextSizeA(text_size_y, FLT_MAX, 0.0f,
                                                                config.text.data(),
                                                                config.text.data() + config.text.size())
                                     : ImGui::CalcTextSize(config.text.data(),
                                                           config.text.data() + config.text.size());
        const float frame_height = ImGui::GetFrameHeight();
        const float row_height = std::max(frame_height, text_size.y);
        const float text_y = origin.y + (row_height - text_size.y) * 0.5f;
        const float button_y = origin.y + (row_height - frame_height) * 0.5f;

        ImGui::PushID(id);
        ImGui::BeginGroup();
        ImGui::SetCursorScreenPos(ImVec2(origin.x, text_y));
        if (config.text_font != nullptr) {
            ImGui::PushFont(config.text_font);
        }
        if (config.use_text_color) {
            ImGui::PushStyleColor(ImGuiCol_Text, config.text_color);
        }
        ImGui::TextUnformatted(config.text.data(), config.text.data() + config.text.size());
        if (config.use_text_color) {
            ImGui::PopStyleColor();
        }
        if (config.text_font != nullptr) {
            ImGui::PopFont();
        }

        ImGui::SameLine(0.0f, gap);
        ImGui::SetCursorScreenPos(ImVec2(origin.x + text_size.x + gap, button_y));
        ImGui::BeginDisabled(config.disabled);
        IconButtonConfig button;
        button.size = ImVec2(frame_height, frame_height);
        button.appear_on_hover = true;
        const bool clicked = IconButtonCentered("##edit", icon, button);
        const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
        ImGui::EndDisabled();
        if (hovered && config.edit_tooltip != nullptr) {
            ImGui::SetTooltip("%s", config.edit_tooltip);
        }
        ImGui::EndGroup();
        ImGui::PopID();
        // Invoke application callbacks after restoring the caller's ID scope.
        // Popup IDs opened by the callback must resolve in the same scope as
        // the matching BeginPopup call that follows this widget.
        if (clicked && config.on_edit) {
            config.on_edit();
        }
        return clicked;
    }

}  // namespace ImGuiX::Widgets
