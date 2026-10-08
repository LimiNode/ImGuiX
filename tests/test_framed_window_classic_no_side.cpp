#include <exception>
#include <iostream>
#include <string>
#include <utility>

#include <imguix/core.hpp>
#include <imguix/windows/ImGuiFramedWindow.hpp>

#if defined(IMGUIX_USE_SFML_BACKEND)

namespace {

    class ClassicNoSideWindow final : public ImGuiX::Windows::ImGuiFramedWindow {
       public:
        ClassicNoSideWindow(int id, ImGuiX::ApplicationContext& app, std::string name)
            : ImGuiFramedWindow(
                  id,
                  app,
                  std::move(name),
                  "Classic no-side-panel regression",
                  ImGuiX::Windows::WindowFlags::NoFlags,
                  make_config()) {}

        int main_region_calls() const { return m_main_region_calls; }
        int side_panel_calls() const { return m_side_panel_calls; }

        void onInit() override { create(640, 480); }

       protected:
        void drawMainRegionContent() override {
            ++m_main_region_calls;
            close();
        }

        void drawSidePanel() override { ++m_side_panel_calls; }

       private:
        static ImGuiX::Windows::ImGuiFramedWindowConfig make_config() {
            ImGuiX::Windows::ImGuiFramedWindowConfig config{};
            config.side_panel_width = 0;
            return config;
        }

        int m_main_region_calls = 0;
        int m_side_panel_calls = 0;
    };

}  // namespace

int main() {
    try {
        ImGuiX::Application app;
        auto& window = app.createWindow<ClassicNoSideWindow>("ClassicNoSideRegression");
        app.run();

        if (window.main_region_calls() != 1) {
            std::cerr << "classic no-side layout rendered main region "
                      << window.main_region_calls() << " times\n";
            return 1;
        }
        if (window.side_panel_calls() != 0) {
            std::cerr << "classic no-side layout rendered side panel "
                      << window.side_panel_calls() << " times\n";
            return 1;
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    } catch (...) {
        return 1;
    }
}

#else

int main() {
    return 0;
}

#endif
