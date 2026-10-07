#define IMGUIX_HEADER_ONLY

#include <imguix/widgets/containers/content_surface.hpp>

int main() {
    const ImGuiX::Widgets::ContentSurfaceConfig config;
    return config.size.x == 0.0f &&
                   config.size.y == 0.0f &&
                   config.rounding < 0.0f &&
                   config.padding.x < 0.0f &&
                   config.padding.y < 0.0f &&
                   config.window_flags == ImGuiWindowFlags_None
               ? 0
               : 1;
}
