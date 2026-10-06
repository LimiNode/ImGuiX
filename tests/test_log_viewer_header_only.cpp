#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/log/log_viewer.hpp>

int main() {
    ImGuiX::Widgets::LogViewerConfig config;
    ImGuiX::Widgets::LogViewerState state;
    return config.show_copy_all && state.selected_ids.empty() ? 0 : 1;
}
