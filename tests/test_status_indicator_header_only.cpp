#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/misc/status_indicator.hpp>

int main() {
    const ImGuiX::Widgets::StatusIndicatorConfig config;
    return config.radius == 4.0f && config.spacing == 6.0f &&
                   config.optical_offset_y == 0.0f
               ? 0
               : 1;
}
