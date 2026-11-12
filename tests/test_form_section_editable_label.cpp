#include <imguix/widgets/containers/form_section.hpp>
#include <imguix/widgets/controls/editable_label.hpp>

#include <imgui.h>

#include <stdexcept>
#include <string>

namespace {

    void require(const bool condition, const char* message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

}  // namespace

int main() {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(800.0f, 600.0f);
    io.Fonts->AddFontDefault();
    io.Fonts->Build();

    bool edit_requested = false;
    ImGui::NewFrame();
    ImGui::Begin("##form_section_editable_label", nullptr, ImGuiWindowFlags_NoDecoration);

    ImGuiX::Widgets::FormSectionConfig connection;
    connection.title = "Connection";
    require(ImGuiX::Widgets::BeginFormSection("##connection", connection),
            "FormSection must use its configured default-open state.");
    ImGui::TextUnformatted("Body");
    ImGuiX::Widgets::EndFormSection();

    ImGuiX::Widgets::FormSectionConfig advanced;
    advanced.title = "Advanced";
    advanced.default_open = false;
    require(!ImGuiX::Widgets::BeginFormSection("##advanced", advanced),
            "FormSection instances must keep independent state.");
    ImGuiX::Widgets::EndFormSection();

    ImGuiX::Widgets::EditableLabelConfig label;
    label.id = "##identity";
    label.text = "Demo account";
    label.edit_tooltip = "Edit identity";
    label.on_edit = [&edit_requested]() { edit_requested = true; };
    const ImVec2 before = ImGui::GetCursorScreenPos();
    ImGuiX::Widgets::EditableLabel(label);
    const ImVec2 after = ImGui::GetCursorScreenPos();
    require(after.y > before.y, "EditableLabel must reserve a normal row height.");
    require(!edit_requested, "EditableLabel must not invoke its callback without a click.");

    ImGui::End();
    ImGui::Render();
    ImGui::DestroyContext();
    return 0;
}
