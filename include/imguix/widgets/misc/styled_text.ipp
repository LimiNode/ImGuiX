#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE ImVec2 StyledText(const StyledTextConfig& config) {
        if (config.text.empty()) {
            return ImVec2{};
        }

        const bool pushed_font = config.font != nullptr;
        if (pushed_font) {
            ImGui::PushFont(config.font);
        }
        ImGui::PushStyleColor(ImGuiCol_Text, config.color);
        const char* begin = config.text.data();
        ImGui::TextUnformatted(begin, begin + config.text.size());
        const ImVec2 size = ImGui::GetItemRectSize();
        ImGui::PopStyleColor();
        if (pushed_font) {
            ImGui::PopFont();
        }
        return size;
    }

    IMGUIX_IMPL_INLINE ImVec2 StyledText(const std::string_view text, ImFont* font,
                                         const ImVec4& color) {
        return StyledText(StyledTextConfig{text, font, color});
    }

}  // namespace ImGuiX::Widgets
