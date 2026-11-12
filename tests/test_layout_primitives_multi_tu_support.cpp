#include <imguix/widgets/layout_primitives.hpp>

#include <imgui.h>

extern "C" void layout_primitives_multi_tu_probe() {
    ImGuiX::Widgets::ResponsiveColumnsConfig config;
    config.min_column_width = 100.0f;
    config.gap = 4.0f;
    if (!ImGuiX::Widgets::BeginResponsiveColumns("##multi_tu_columns", 2, config)) {
        return;
    }
    ImGuiX::Widgets::BeginResponsiveColumn(0);
    ImGui::TextUnformatted("multi-tu-left");
    ImGuiX::Widgets::EndResponsiveColumn();
    ImGuiX::Widgets::BeginResponsiveColumn(1);
    ImGui::TextUnformatted("multi-tu-right");
    ImGuiX::Widgets::EndResponsiveColumn();
    ImGuiX::Widgets::EndResponsiveColumns();
}

// Finish a group begun in the other TU. Separate private stacks would be
// empty here even if a complete Begin/End pair in each TU worked on its own.
extern "C" void layout_primitives_multi_tu_finish() {
    ImGuiX::Widgets::BeginResponsiveColumn(1);
    ImGui::TextUnformatted("cross-tu-right");
    ImGuiX::Widgets::EndResponsiveColumn();
    ImGuiX::Widgets::EndResponsiveColumns();
}

// Explicit identity check also detects private stacks when assertions are off.
extern "C" const void* layout_primitives_multi_tu_stack() {
    return &ImGuiX::Widgets::detail::responsiveColumnsStack();
}
