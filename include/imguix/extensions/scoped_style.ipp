namespace ImGuiX::Extensions {

    inline ScopedStyleVar::ScopedStyleVar(ImGuiStyleVar index, float value) {
        ImGui::PushStyleVar(index, value);
    }

    inline ScopedStyleVar::ScopedStyleVar(ImGuiStyleVar index, const ImVec2& value) {
        ImGui::PushStyleVar(index, value);
    }

    inline ScopedStyleVar::~ScopedStyleVar() {
        ImGui::PopStyleVar();
    }

    inline ScopedStyleColor::ScopedStyleColor(ImGuiCol index, ImU32 color) {
        ImGui::PushStyleColor(index, color);
    }

    inline ScopedStyleColor::ScopedStyleColor(ImGuiCol index, const ImVec4& color) {
        ImGui::PushStyleColor(index, color);
    }

    inline ScopedStyleColor::~ScopedStyleColor() {
        ImGui::PopStyleColor();
    }

}  // namespace ImGuiX::Extensions
