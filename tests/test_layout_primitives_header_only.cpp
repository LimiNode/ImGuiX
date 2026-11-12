#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/layout_primitives.hpp>
#include <imguix/widgets/containers/section_card.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {

    void draw_frame() {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("##layout_primitives", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);

        ImGuiX::Widgets::SectionCardConfig card;
        card.title = "Card";
        card.panel.border_thickness = 1.0f;
        if (!ImGuiX::Widgets::BeginSectionCard("##card", card)) {
            std::cerr << "content card unexpectedly clipped\n";
            std::exit(1);
        }
        ImGui::TextUnformatted("body");
        ImGuiX::Widgets::EndSectionCard();

        ImGuiX::Widgets::ResponsiveColumnsConfig columns;
        columns.min_column_width = 200.0f;
        if (!ImGuiX::Widgets::BeginResponsiveColumns("##columns", 2, columns)) {
            std::cerr << "responsive columns did not open\n";
            std::exit(1);
        }
        ImGuiX::Widgets::BeginResponsiveColumn(0);
        const ImGuiID left_id = ImGui::GetID("same-control");
        ImGui::Button("same-control");
        ImGuiX::Widgets::EndResponsiveColumn();
        ImGuiX::Widgets::BeginResponsiveColumn(1);
        const ImGuiID right_id = ImGui::GetID("same-control");
        ImGui::Button("same-control");
        ImGuiX::Widgets::EndResponsiveColumn();
        ImGuiX::Widgets::EndResponsiveColumns();
        if (left_id == right_id) {
            std::cerr << "responsive columns reused an ImGui ID\n";
            std::exit(1);
        }

        ImGuiX::Widgets::PageHeaderConfig header;
        header.title = "Header";
        header.back_action = [] {};
        ImGuiX::Widgets::PageHeader(header);

        ImGuiX::Widgets::SectionCardConfig constrained;
        constrained.title = "Constrained";
        constrained.height_policy = ImGuiX::Widgets::SectionCardHeightPolicy::Constrained;
        constrained.max_height = 100.0f;
        if (!ImGuiX::Widgets::BeginSectionCard("##constrained", constrained)) {
            std::cerr << "constrained card unexpectedly clipped\n";
            std::exit(1);
        }
        for (int index = 0; index < 32; ++index) {
            ImGui::Text("line %d", index);
        }
        ImGuiX::Widgets::EndSectionCard();

        ImGui::OpenPopup("##card_popup");
        if (ImGui::BeginPopup("##card_popup")) {
            ImGui::TextUnformatted("popup");
            ImGui::EndPopup();
        }
        ImGui::End();
        ImGui::Render();
    }
}  // namespace

int main() {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(900.0f, 600.0f);
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    draw_frame();
    draw_frame();

    const ImGuiWindow* window = ImGui::FindWindowByName("##layout_primitives");
    const ImGuiWindow* constrained = nullptr;
    for (const ImGuiWindow* candidate : ImGui::GetCurrentContext()->Windows) {
        if (std::string_view(candidate->Name).find("##layout_primitives/##constrained") == 0) {
            constrained = candidate;
            break;
        }
    }
    if (window == nullptr || window->ScrollMax.y >= 1.0f || constrained == nullptr ||
        constrained->ScrollMax.y < 1.0f) {
        std::cerr << "expected constrained-only scrolling was not established\n";
        return 1;
    }
    ImGui::DestroyContext();
    return 0;
}
