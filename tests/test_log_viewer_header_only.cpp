#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/log/log_viewer.hpp>

#include <cstring>

int main() {
    ImGuiX::Widgets::LogViewerConfig config;
    ImGuiX::Widgets::LogViewerState state;
    const bool zero_panel_padding =
        config.panel.padding.x == 0.0f && config.panel.padding.y == 0.0f;
    const bool auto_column_widths =
        config.timestamp_column_width <= 0.0f && config.level_column_width <= 0.0f;
    const bool distinct_folder_and_paste_icons =
        std::strcmp(IMGUIX_ICON_FOLDER, IMGUIX_ICON_PASTE) != 0;
    return config.show_copy_all && state.selected_ids.empty() && zero_panel_padding &&
               auto_column_widths && distinct_folder_and_paste_icons
        ? 0
        : 1;
}
