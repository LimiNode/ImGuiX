#include <cstdint>

#include <imguix/widgets/containers/form_section.hpp>
#include <imguix/widgets/controls/editable_label.hpp>
#include <imguix/widgets/auth/domain_selector.hpp>
#include <imguix/widgets/input/validated_password_input.hpp>
#include <imguix/widgets/auth/proxy_panel.hpp>
#include <imguix/widgets/auth/auth_js_panel.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <stdexcept>

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

    int edit_count = 0;
    bool has_popup = false;
    ImVec2 button_center;
    for (int frame = 0; frame < 5; ++frame) {
        if (frame > 0) {
            io.AddMousePosEvent(button_center.x, button_center.y);
        }
        if (frame == 2 || frame == 3) {
            io.AddMouseButtonEvent(ImGuiMouseButton_Left, frame == 2);
        }
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(600.0f, 400.0f));
        ImGui::Begin("##form_section_editable_label", nullptr, ImGuiWindowFlags_NoDecoration);
        ImGui::PushID("caller");

        ImGuiX::Widgets::FormSectionConfig connection;
        connection.title = "Connection";
        require(ImGuiX::Widgets::BeginFormSection("##connection", connection),
                "FormSection must retain its default-open state.");
        ImGui::TextUnformatted("Body");
        ImGuiX::Widgets::EndFormSection();

        ImGuiX::Widgets::FormSectionConfig advanced;
        advanced.title = "Advanced";
        advanced.default_open = false;
        require(!ImGuiX::Widgets::BeginFormSection("##advanced", advanced),
                "FormSection instances must keep independent state.");
        ImGuiX::Widgets::EndFormSection();

        const ImGuiID popup_id = ImGui::GetID("##identity_popup");
        ImGuiX::Widgets::EditableLabelConfig label;
        label.id = "##identity";
        label.text = "Demo account";
        label.edit_tooltip = "Edit identity";
        label.on_edit = [&]() {
            require(ImGui::GetID("##identity_popup") == popup_id,
                    "Edit callback must run in the caller ID scope.");
            ++edit_count;
            ImGui::OpenPopup("##identity_popup");
        };
        const ImVec2 before = ImGui::GetCursorScreenPos();
        ImGuiX::Widgets::EditableLabel(label);
        require(ImGui::GetCursorScreenPos().y > before.y,
                "EditableLabel must reserve a normal row height.");
        // EndGroup exposes the full row, not the button. Target its rightmost
        // frame-height square so this is an actual mouse interaction test.
        button_center = ImVec2(ImGui::GetItemRectMax().x - ImGui::GetFrameHeight() * 0.5f,
                               before.y + ImGui::GetFrameHeight() * 0.5f);
        if (ImGui::BeginPopup("##identity_popup")) {
            has_popup = true;
            ImGui::TextUnformatted("Edit name and description");
            ImGui::EndPopup();
        }
        if (frame < 3) {
            require(edit_count == 0, "Edit callback must not fire before mouse release.");
        }
        if (frame == 0) {
            const ImVec4 child_color = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];
            ImGuiX::Widgets::DomainSelectorConfig domain;
            domain.embedded = true;
            domain.border = false;
            domain.vk_enabled = false;
            domain.panel_size = ImVec2(300.0f, 80.0f);
            std::string host = "example.test";
            ImGuiX::Widgets::DomainSelector("domain", domain, host);
            ImGuiX::Widgets::ProxyPanelConfig proxy;
            proxy.embedded = true;
            proxy.border = false;
            proxy.panel_size = ImVec2(300.0f, 150.0f);
            ImGuiX::Widgets::ProxySettings proxy_settings;
            ImGuiX::Widgets::ProxyPanel("proxy", proxy, proxy_settings);
            ImGuiX::Widgets::AuthJsPanelConfig browser;
            browser.embedded = true;
            browser.border = false;
            browser.panel_size = ImVec2(300.0f, 150.0f);
            ImGuiX::Widgets::AuthJsSettings browser_settings;
            ImGuiX::Widgets::AuthJsPanel("browser", browser, browser_settings);
            int embedded_count = 0;
            for (const ImGuiWindow* window : ImGui::GetCurrentContext()->Windows) {
                if (window->ParentWindow == ImGui::GetCurrentWindow()) {
                    require((window->Flags & ImGuiWindowFlags_NoBackground) != 0,
                            "Embedded auth panels must not draw a separate background.");
                    ++embedded_count;
                }
            }
            require(embedded_count == 3, "Embedded test must inspect all three auth panels.");
            const ImVec4 after_color = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];
            require(child_color.x == after_color.x && child_color.y == after_color.y &&
                        child_color.z == after_color.z && child_color.w == after_color.w,
                    "Embedded panels must preserve the host style.");
        }
        ImGui::PopID();
        ImGui::End();
        ImGui::Render();
    }
    require(edit_count == 1 && has_popup,
            "Clicking EditableLabel must open the caller-owned popup exactly once.");
    ImGui::DestroyContext();
    return 0;
}
