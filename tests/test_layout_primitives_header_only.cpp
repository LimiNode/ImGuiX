#ifndef IMGUIX_HEADER_ONLY
#   define IMGUIX_HEADER_ONLY
#endif

#include <imguix/widgets/layout_primitives.hpp>
#include <imguix/widgets/containers/section_card.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <cstdlib>
#include <iostream>
#include <string_view>

extern "C" void layout_primitives_multi_tu_probe();
extern "C" void layout_primitives_multi_tu_finish();
extern "C" const void* layout_primitives_multi_tu_stack();

namespace {

    void draw_frame(const ImVec2 size, const bool long_message) {
        ImGui::GetIO().DisplaySize = size;
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(size);
        ImGui::Begin("##layout_primitives", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);

        ImGuiX::Widgets::SectionCardConfig card;
        card.title = "Card";
        card.panel.border_thickness = 1.0f;
        if (!ImGuiX::Widgets::BeginSectionCard("##card", card)) {
            std::cerr << "content card unexpectedly clipped\n";
            std::exit(1);
        }
        ImGui::TextUnformatted(long_message ? "A long localized error message that changes card content."
                                            : "body");
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

        // A full pair in another TU is useful, but only a split Begin/End
        // proves both TUs actually share the same responsive-state stack.
        if (layout_primitives_multi_tu_stack() != &ImGuiX::Widgets::detail::responsiveColumnsStack()) {
            std::cerr << "header-only translation units do not share responsive state\n";
            std::exit(1);
        }
        layout_primitives_multi_tu_probe();
        if (ImGuiX::Widgets::BeginResponsiveColumns("##cross_tu_columns", 2, columns)) {
            ImGuiX::Widgets::BeginResponsiveColumn(0);
            ImGui::TextUnformatted("cross-tu-left");
            ImGuiX::Widgets::EndResponsiveColumn();
            layout_primitives_multi_tu_finish();
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
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(800.0f, 600.0f);
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    draw_frame(ImVec2(800.0f, 600.0f), false);
    draw_frame(ImVec2(1650.0f, 930.0f), true);
    draw_frame(ImVec2(800.0f, 600.0f), false);

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

    // A second context must start with an empty layout stack and the same
    // primitives must remain usable after a full context teardown.
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().DeltaTime = 1.0f / 60.0f;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    draw_frame(ImVec2(1650.0f, 930.0f), true);
    ImGui::DestroyContext();
    return 0;
}
