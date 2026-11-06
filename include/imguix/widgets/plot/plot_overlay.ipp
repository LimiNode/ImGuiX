#include <imgui_internal.h>
#include <implot_internal.h>

#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE void PlotOverlay(const PlotOverlayConfig& config,
                                        const PlotOverlayCallback& callback) {
        if (!callback) {
            return;
        }

        ImPlotPlot* current_plot = ImPlot::GetCurrentPlot();
        IM_ASSERT(current_plot != nullptr &&
                  "PlotOverlay() must be called between BeginPlot() and EndPlot()");
        if (current_plot == nullptr) {
            return;
        }

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        ImGuiContext& context = *GImGui;
        const ImGuiWindowTempData saved_window_data = window->DC;
        const ImGuiLastItemData saved_last_item = context.LastItemData;

        const ImVec2 plot_pos = ImPlot::GetPlotPos();
        const ImVec2 plot_size = ImPlot::GetPlotSize();
        const ImVec2 style_padding = ImGui::GetStyle().ItemInnerSpacing;
        const ImVec2 padding(config.padding.x >= 0.0f ? config.padding.x : style_padding.x,
                             config.padding.y >= 0.0f ? config.padding.y : style_padding.y);
        const ImVec2 content_pos(plot_pos.x + config.offset.x + padding.x,
                                 plot_pos.y + config.offset.y + padding.y);

        if (config.clip_to_plot) {
            ImGui::PushClipRect(plot_pos,
                                ImVec2(plot_pos.x + plot_size.x, plot_pos.y + plot_size.y), true);
        }
        ImGui::SetCursorScreenPos(content_pos);
        callback(PlotOverlayContext{plot_pos, plot_size, content_pos});
        if (config.clip_to_plot) {
            ImGui::PopClipRect();
        }

        // Overlay widgets must not consume parent layout space or make a later
        // SameLine()/ItemSize() depend on the last widget drawn over the plot.
        window->DC = saved_window_data;
        context.LastItemData = saved_last_item;
    }

}  // namespace ImGuiX::Widgets
