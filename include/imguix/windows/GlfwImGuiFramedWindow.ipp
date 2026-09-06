#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <imguix/extensions/blend_colors.hpp>

namespace ImGuiX::Windows {

    bool ImGuiFramedWindow::create() {
        if (m_window || m_is_open) return true;
        if (!glfwInit()) return false;
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        m_window = glfwCreateWindow(width(), height(), name().c_str(), nullptr, nullptr);
        if (!m_window) return false;
        glfwMakeContextCurrent(m_window);

        IMGUI_CHECKVERSION();
        if (!ImGui::GetCurrentContext()) {
            ImGui::CreateContext();
#           ifdef IMGUIX_ENABLE_IMPLOT
            m_implot_ctx = ImPlot::CreateContext();
            ImPlot::SetCurrentContext(m_implot_ctx);
#           endif
#           ifdef IMGUIX_ENABLE_IMPLOT3D
            m_implot3d_ctx = ImPlot3D::CreateContext();
            ImPlot3D::SetCurrentContext(m_implot3d_ctx);
#           endif
        }
        
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init();
        m_is_open = true;
        return true;
    }

    void ImGuiFramedWindow::tick() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (hasFlag(m_flags, WindowFlags::EnableTransparency)) {
            glClearColor(0.f, 0.f, 0.f, 0.f);
        } else {
            glClearColor(m_config.clear_color.x, m_config.clear_color.y,
                         m_config.clear_color.z, m_config.clear_color.w);
        }
        glClear(GL_COLOR_BUFFER_BIT);
        updateCurrentTheme();
        Pubsub::SyncNotifier notifier{eventBus()};
        for (auto& ctrl : m_controllers) {
            ctrl->processFeatures(notifier);
        }
    }

    void ImGuiFramedWindow::drawUi() {
        m_title_bar_interactive_rect = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
#       ifdef IMGUIX_ENABLE_IMPLOT
        ImPlot::SetCurrentContext(m_implot_ctx);
#       endif
#       ifdef IMGUIX_ENABLE_IMPLOT3D
        ImPlot3D::SetCurrentContext(m_implot3d_ctx);
#       endif
        ImGui::PushID(id());
        const ImGuiStyle& style = ImGui::GetStyle();

        // --- Root host window
        int fb_w = 0, fb_h = 0;
        glfwGetFramebufferSize(m_window, &fb_w, &fb_h);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)fb_w, (float)fb_h));

        const float menu_bar_height = ImGui::GetFrameHeight();

        const ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        // --- Optional transparent host background
        // Blend border and window colors so frame border remains visible when root background is transparent.
        if (hasFlag(m_flags, WindowFlags::DisableBackground) || m_disable_background) {
            ImVec4 border_color = style.Colors[ImGuiCol_Border];
            ImVec4 background_color = style.Colors[ImGuiCol_WindowBg];
            ImVec4 new_color = Extensions::BlendColors(border_color, background_color);
            ImGui::PushStyleColor(ImGuiCol_Border, new_color);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
        }

        ImGui::Begin(u8"##imguix_framed_window", nullptr, flags);

        if (hasFlag(m_flags, WindowFlags::DisableBackground) || m_disable_background) {
            ImGui::PopStyleColor(2);
        }

        if (hasFlag(m_flags, WindowFlags::HasCornerIconArea)) {
            drawCornerLayout(menu_bar_height);
        } else {
            drawClassicLayout(menu_bar_height);
        }

        ImGui::End();
        ImGui::PopID();
        notifications().render();
    }

} // namespace ImGuiX::Windows
