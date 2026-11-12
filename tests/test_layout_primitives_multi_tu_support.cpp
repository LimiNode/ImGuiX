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
