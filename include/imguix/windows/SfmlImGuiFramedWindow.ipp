#ifdef _WIN32
#include <windowsx.h>
#include <commctrl.h>
#include <Dwmapi.h>     // For window transparency: https://gist.github.com/Alia5/5d8c48941d1f73c1ef14967a5ffe33d5
#endif
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/Color.hpp>
#include <imguix/extensions/blend_colors.hpp>
#include <imguix/extensions/color_utils.hpp>

#ifdef _WIN32
#define IMGUIX_WM_USER_ROUNDED_REGION (WM_USER + 201)
#endif

namespace ImGuiX::Windows {

    // Windows Unicode clipboard for Dear ImGui (UTF-8 <-> UTF-16)
    // Link with user32.lib
#   ifdef _WIN32
    namespace {

        static void SetClipboardText_Win(void*, const char* utf8) {
            if (!utf8) return;
            int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
            if (wlen <= 0) return;

            HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, wlen * sizeof(wchar_t));
            if (!hmem) return;
            wchar_t* wbuf = static_cast<wchar_t*>(GlobalLock(hmem));
            MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wbuf, wlen);
            GlobalUnlock(hmem);

            if (OpenClipboard(nullptr)) {
                EmptyClipboard();
                SetClipboardData(CF_UNICODETEXT, hmem); // ownership -> system
                CloseClipboard();
            } else {
                GlobalFree(hmem);
            }
        }

        static const char* GetClipboardText_Win(void*) {
            static std::string out; out.clear();
            if (!OpenClipboard(nullptr)) return "";

            HANDLE h = GetClipboardData(CF_UNICODETEXT);
            if (h) {
                const wchar_t* w = static_cast<const wchar_t*>(GlobalLock(h));
                if (w) {
                    int len = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
                    out.resize(len ? len - 1 : 0);
                    if (len) WideCharToMultiByte(CP_UTF8, 0, w, -1, out.data(), len, nullptr, nullptr);
                    GlobalUnlock(h);
                }
            } else {
                // Fallback для старых/ANSI-приложений (если Unicode отсутствует)
                HANDLE ht = GetClipboardData(CF_TEXT);
                if (ht) {
                    const char* a = static_cast<const char*>(GlobalLock(ht));
                    if (a) { out.assign(a); GlobalUnlock(ht); }
                }
            }
            CloseClipboard();
            return out.c_str();
        }

        static void Platform_SetClipboardText_Win(ImGuiContext*, const char* text) { 
            SetClipboardText_Win(nullptr, text); 
        }

        static const char* Platform_GetClipboardText_Win(ImGuiContext*) { 
            return GetClipboardText_Win(nullptr); 
        }

    }
#   endif

    bool ImGuiFramedWindow::create() {
#       ifdef _WIN32
        if (applyCommonWindowSetup()) {
            HWND hwnd = m_window.getNativeHandle();
            SetWindowSubclass(hwnd, SubclassProc, 1, reinterpret_cast<DWORD_PTR>(this));
            setupWindowEffects(hwnd);
            if (GetForegroundWindow() != hwnd) {
                SetForegroundWindow(hwnd);
                SetFocus(hwnd);
                SetActiveWindow(hwnd);
            }
        }
#       else
        applyCommonWindowSetup();
#       endif
        return m_is_open;
    }

    void ImGuiFramedWindow::tick() {
        setCurrentWindow();
        auto& res = registry().getResource<DeltaClockSfml>();
        ImGui::SFML::Update(m_window, res.delta());
        if (hasFlag(m_flags, WindowFlags::EnableTransparency)) {
            m_window.clear(sf::Color::Transparent);
        } else {
            m_window.clear(Extensions::ColorToSfml(m_config.clear_color));
        }
        updateCurrentTheme();
        Pubsub::SyncNotifier notifier{eventBus()};
        for (auto& ctrl : m_controllers) {
            ctrl->processFeatures(notifier);
        }
    }

    void ImGuiFramedWindow::drawUi() {
        setCurrentWindow();
        ImGui::PushID(id());
        const ImGuiStyle& style = ImGui::GetStyle();

        // --- Root host window
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize(ImVec2((float)m_window.getSize().x, (float)m_window.getSize().y));

        const float menu_bar_height = ImGui::GetFrameHeight();

        const ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        const bool transparent_host_background =
            hasFlag(m_flags, WindowFlags::DisableBackground) || m_disable_background;
        ImU32 frame_border_color = ImGui::GetColorU32(style.Colors[ImGuiCol_Border]);

        // --- Optional transparent host background
        // Blend border and window colors so frame border remains visible when root background is transparent.
        if (transparent_host_background) {
            const ImVec4 border_color = style.Colors[ImGuiCol_Border];
            const ImVec4 background_color = style.Colors[ImGuiCol_WindowBg];
            const ImVec4 new_color = Extensions::BlendColors(border_color, background_color);
            frame_border_color = ImGui::GetColorU32(new_color);
            ImGui::PushStyleColor(ImGuiCol_Border, new_color);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
        }

        const float host_rounding = m_config.frame_corner_radius > 0
            ? static_cast<float>(m_config.frame_corner_radius)
            : 0.0f;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, host_rounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin(u8"##imguix_framed_window", nullptr, flags);
        ImGui::PopStyleVar(2);

        if (transparent_host_background) {
            ImGui::PopStyleColor(2);
        }

        if (hasFlag(m_flags, WindowFlags::HasCornerIconArea)) {
            drawCornerLayout(menu_bar_height);
        } else {
            drawClassicLayout(menu_bar_height);
        }

        const ImVec2 window_pos = ImGui::GetWindowPos();
        const ImVec2 window_size = ImGui::GetWindowSize();
        const float outer_stroke = ImMax(0.0f, m_config.frame_outer_stroke_thickness);
        const float inner_stroke = ImMax(0.0f, m_config.frame_inner_stroke_thickness);
        const float total_stroke = outer_stroke + inner_stroke;
        if ((outer_stroke > 0.0f || inner_stroke > 0.0f) &&
            window_size.x > total_stroke * 2.0f &&
            window_size.y > total_stroke * 2.0f) {
            const ImVec2 outer_min = window_pos;
            const ImVec2 outer_max = window_pos + window_size;
            const ImU32 background_color = ImGui::GetColorU32(style.Colors[ImGuiCol_WindowBg]);
            const ImU32 border_color = frame_border_color;

            ImDrawList* draw_list = ImGui::GetForegroundDrawList();
            draw_list->PushClipRect(window_pos, window_pos + window_size, true);

            if (outer_stroke > 0.0f) {
                const float outer_rounding = ImMax(0.0f, host_rounding - outer_stroke);
                draw_list->AddRect(
                    outer_min,
                    outer_max,
                    background_color,
                    outer_rounding,
                    ImDrawFlags_None,
                    outer_stroke * 2.0f);
            }

            if (inner_stroke > 0.0f && window_size.x > outer_stroke * 2.0f && window_size.y > outer_stroke * 2.0f) {
                const ImVec2 inner_min = outer_min + ImVec2(outer_stroke, outer_stroke);
                const ImVec2 inner_max = outer_max - ImVec2(outer_stroke, outer_stroke);
                const float inner_rounding = ImMax(0.0f, host_rounding - outer_stroke - inner_stroke);
                draw_list->AddRect(
                    inner_min,
                    inner_max,
                    border_color,
                    inner_rounding,
                    ImDrawFlags_None,
                    inner_stroke);
            }

            draw_list->PopClipRect();
        }

        ImGui::End();
        
        ImGui::PopID();
        notifications().render();
    }

#   ifdef _WIN32

    void ImGuiFramedWindow::setupWindowEffects(HWND hwnd) {
        if (hasFlag(m_flags, WindowFlags::EnableTransparency)) {
            // Enable window transparency: https://gist.github.com/Alia5/5d8c48941d1f73c1ef14967a5ffe33d5
            MARGINS margins;
            margins.cxLeftWidth = -1;
            SetWindowLong(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
            DwmExtendFrameIntoClientArea(hwnd, &margins);
        }

        int w = static_cast<int>(m_window.getSize().x);
        int h = static_cast<int>(m_window.getSize().y);
        applyRoundedRegion(hwnd, w, h, m_config.frame_corner_radius);
        PostMessage(hwnd, IMGUIX_WM_USER_ROUNDED_REGION, 0, 0);
    }
    
    bool ImGuiFramedWindow::applyCommonWindowSetup() {
        if (m_window.isOpen() || m_is_open) return true;
        m_window.create(sf::VideoMode({static_cast<unsigned int>(width()),  static_cast<unsigned int>(height())}), name(), sf::Style::None);
        m_window.setFramerateLimit(60);
        m_is_open = ImGui::SFML::Init(m_window);

#       ifdef IMGUIX_ENABLE_IMPLOT
        m_implot_ctx = ImPlot::CreateContext();
        ImPlot::SetCurrentContext(m_implot_ctx);
#       endif

#       ifdef IMGUIX_ENABLE_IMPLOT3D
        m_implot3d_ctx = ImPlot3D::CreateContext();
        ImPlot3D::SetCurrentContext(m_implot3d_ctx);
#       endif

#       ifdef _WIN32
        HWND hwnd = m_window.getNativeHandle();
        if (GetForegroundWindow() != hwnd) {
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
            SetActiveWindow(hwnd);
        }
        
        // Put this where you init ImGui (after creating io)
        ImGuiIO& io = ImGui::GetIO();
        ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
        pio.Platform_SetClipboardTextFn = Platform_SetClipboardText_Win;
        pio.Platform_GetClipboardTextFn = Platform_GetClipboardText_Win;
        io.SetClipboardTextFn = nullptr;
        io.GetClipboardTextFn = nullptr;
#       endif

        return m_is_open;
    }
    
    void ImGuiFramedWindow::applyRoundedRegion(HWND hwnd, int width, int height, int radius) {
        const int safe_radius = radius > 0 ? radius : 0;
        HRGN region = CreateRoundRectRgn(
            0,
            0,
            width + 1,
            height + 1,
            safe_radius * 2,
            safe_radius * 2);
        SetWindowRgn(hwnd, region, TRUE);
    }
    
    LRESULT CALLBACK ImGuiFramedWindow::SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                     UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
        ImGuiFramedWindow* self = reinterpret_cast<ImGuiFramedWindow*>(dwRefData);
        if (self) {
            return self->WndProc(hwnd, msg, wParam, lParam);
        }
        return DefSubclassProc(hwnd, msg, wParam, lParam);
    }
    
    LRESULT ImGuiFramedWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (!hasFlag(m_flags, WindowFlags::ShowControlButtons)) {
            return DefSubclassProc(hwnd, msg, wParam, lParam);
        }
        switch (msg) {
            case WM_ENTERSIZEMOVE:
                m_in_manual_sizing = true;
                return DefSubclassProc(hwnd, msg, wParam, lParam);
            case WM_EXITSIZEMOVE:
                m_in_manual_sizing = false;
                return DefSubclassProc(hwnd, msg, wParam, lParam);
            case WM_GETMINMAXINFO: {
                MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
                mmi->ptMinTrackSize.x = m_config.min_width;
                mmi->ptMinTrackSize.y = m_config.min_height;
                return 0;
            }
            case IMGUIX_WM_USER_ROUNDED_REGION: {
                RECT r{};
                if (GetClientRect(hwnd, &r)) {
                    const int new_width = r.right - r.left;
                    const int new_height = r.bottom - r.top;
                    if (new_width != m_prev_width || new_height != m_prev_height) {
                        m_prev_width = new_width;
                        m_prev_height = new_height;
                        applyRoundedRegion(hwnd, new_width, new_height, m_config.frame_corner_radius);
                    }
                }
                return 0;
            }
            case WM_SIZE:
            case WM_SIZING: {
                RECT rect;
                GetClientRect(hwnd, &rect);
                int w = rect.right - rect.left;
                int h = rect.bottom - rect.top;
                m_window.setSize(sf::Vector2u(w, h));
                ImGui::GetIO().DisplaySize = ImVec2((float)w, (float)h);
                if (w != m_prev_width || h != m_prev_height) {
                    m_prev_width = w;
                    m_prev_height = h;
                    applyRoundedRegion(hwnd, w, h, m_config.frame_corner_radius);
                    renderFrameManually();
                }

                return DefSubclassProc(hwnd, msg, wParam, lParam);
            }
            case WM_NCHITTEST: {
                POINT screen_pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                POINT pt = screen_pt;
                ScreenToClient(hwnd, &pt);

                RECT rect;
                GetClientRect(hwnd, &rect);

                bool left   = pt.x < m_config.resize_border;
                bool right  = pt.x >= rect.right - m_config.resize_border;
                bool top    = pt.y < m_config.resize_border;
                bool bottom = pt.y >= rect.bottom - m_config.resize_border;

                if (top && left)     return HTTOPLEFT;
                if (top && right)    return HTTOPRIGHT;
                if (bottom && left)  return HTBOTTOMLEFT;
                if (bottom && right) return HTBOTTOMRIGHT;
                if (left)   return HTLEFT;
                if (right)  return HTRIGHT;
                if (top)    return HTTOP;
                if (bottom) return HTBOTTOM;

                if (PtInRect(&m_minimize_btn_rect, pt)) {
                    return HTCLIENT;
                }
                if (PtInRect(&m_maximize_btn_rect, pt)) {
                    return HTCLIENT;
                }
                if (PtInRect(&m_close_btn_rect, pt)) {
                    return HTCLIENT;
                }

                const bool in_title_bar_interactive_rect =
                    pt.x >= m_title_bar_interactive_rect.x &&
                    pt.x < m_title_bar_interactive_rect.z &&
                    pt.y >= m_title_bar_interactive_rect.y &&
                    pt.y < m_title_bar_interactive_rect.w;
                if (in_title_bar_interactive_rect) {
                    return HTCLIENT;
                }

                if (pt.y < m_config.title_bar_height) return HTCAPTION;

                return HTCLIENT;
            }
            default:
                return DefSubclassProc(hwnd, msg, wParam, lParam);
        }
    }
#   elif defined(__linux__)

    bool ImGuiFramedWindow::applyCommonWindowSetup() {
        if (m_window.isOpen() || m_is_open) return true;
        m_window.create(
            sf::VideoMode({static_cast<unsigned int>(width()), static_cast<unsigned int>(height())}),
            name(),
            sf::Style::None);
        m_window.setFramerateLimit(60);
        m_is_open = ImGui::SFML::Init(m_window);

#       ifdef IMGUIX_ENABLE_IMPLOT
        m_implot_ctx = ImPlot::CreateContext();
        ImPlot::SetCurrentContext(m_implot_ctx);
#       endif
#       ifdef IMGUIX_ENABLE_IMPLOT3D
        m_implot3d_ctx = ImPlot3D::CreateContext();
        ImPlot3D::SetCurrentContext(m_implot3d_ctx);
#       endif

        return m_is_open;
    }

#   endif

} // namespace ImGuiX::Windows
