#pragma once
#ifndef _IMGUIX_WIDGETS_PLOT_STYLIZED_DUAL_METRIC_BARS_PLOT_HPP_INCLUDED
#define _IMGUIX_WIDGETS_PLOT_STYLIZED_DUAL_METRIC_BARS_PLOT_HPP_INCLUDED

/// \file StylizedDualMetricBarsPlot.hpp
/// \brief Render grouped dual-metric bars with a drag-and-drop category panel.

#include <string_view>
#include <vector>

#include <imgui.h>
#include <implot.h>

#include <imguix/widgets/controls/icon_button.hpp>
#include <imguix/widgets/plot/DualMetricBarsData.hpp>
#include <imguix/widgets/plot/DualMetricBarsPlot.hpp>
#include <imguix/widgets/plot/MetricsPlot.hpp>

namespace ImGuiX::Widgets {

    /// \brief Configuration for StylizedDualMetricBarsPlot.
    struct StylizedDualMetricBarsPlotConfig {
        const char* id = "StylizedDualMetricBarsPlot"; ///< Widget and plot identifier.
        const char* dnd_payload = "STYLIZED_DUAL_METRIC_BARS_PLOT"; ///< Drag-n-drop payload ID.
        const char* annotation_checkbox = "Annotations"; ///< Annotation checkbox label.
        const char* legend_checkbox = "Legend"; ///< Legend checkbox label.
        const char* button_show_all = "Show all"; ///< Select all button label.
        const char* button_reset = "Reset"; ///< Reset selection button label.
        const char* left_list_header = nullptr; ///< Optional left list header.
        const char* plot_header = "Plot header"; ///< Header in right child before plot.
        const char* settings_header = "Chart settings"; ///< Context menu settings header.
        bool annotations_default_on = false; ///< Default annotation visibility.
        bool list_bordered = false; ///< Inner border for scroll list.
        bool left_panel_bordered = true; ///< Border/background for left panel.
        bool cap_by_avail_y = false; ///< Limit height by available space.
        bool input_enabled = true; ///< Enable plot mouse input for this frame.
        bool force_all_visible = false; ///< Plot every category and hide left panel.
        bool use_sticky_select = false; ///< Click list items to toggle instead of DND.
        bool legend_force_off = false; ///< Force legend off regardless of state.
        bool show_annotation_checkbox = false; ///< Show annotation checkbox in compact mode.
        int default_show_index = -1; ///< One category to show by default; -1 means none.
        std::vector<int> default_show_indices; ///< Multiple categories to show by default.
        int max_visible = 8; ///< Reserved for API parity with StylizedMetricsPlot.
        float plot_height = 0.0f; ///< Height of the plot area.
        float aspect_w_over_h = 2.0f; ///< Width to height ratio when height is auto.
        float auto_height_min = 0.0f; ///< Lower bound for auto height.
        float auto_height_max = 0.0f; ///< 0 disables upper bound.
        float dnd_width = 0.0f; ///< Width of DND list, 0 means auto.
        float panels_spacing = 0.0f; ///< Spacing between left panel and plot panel.
        float dnd_scroll_h = 0.0f; ///< Scroll area height inside the left panel.
        ImVec2 fit_padding{0.3f, 0.5f}; ///< ImPlot fit padding.
        ImVec2 right_child_padding{0.0f, 0.0f}; ///< Right child window padding.
        ImPlotLocation legend_loc = ImPlotLocation_South; ///< Legend location.
        ImPlotLegendFlags legend_flags = ImPlotLegendFlags_Horizontal; ///< Legend flags.
        ImPlotLocation legend_loc_compact = ImPlotLocation_South; ///< Compact legend location.
        ImPlotLegendFlags legend_flags_compact = ImPlotLegendFlags_Horizontal; ///< Compact legend flags.
        ImGuiX::Widgets::IconButtonConfig show_side_panel_cfg; ///< Show/hide panel button config.
        DualMetricBarsPlotConfig plot; ///< Inner dual-bars plot configuration.
    };

    /// \brief Runtime state for StylizedDualMetricBarsPlot.
    /// \invariant dnd.size() matches data.labels.size().
    struct StylizedDualMetricBarsPlotState {
        /// \brief Drag-and-drop category item.
        struct DndItem {
            size_t index = 0; ///< Index in data arrays.
            bool is_plot = false; ///< True if category is plotted.
            std::string_view label; ///< Display label.
            ImVec4 color{}; ///< List icon color.
        };

        std::vector<DndItem> dnd; ///< DND category items.
        DualMetricBarsPlotState plot_state; ///< Inner plot state.
        bool show_left_panel = true; ///< Show category panel.
        bool initialized = false; ///< True after defaults have been applied.
    };

    namespace stylized_dual_metric_bars_plot_detail {
        inline constexpr int kUpdateCounterMax = 5;

        inline float calc_plot_height(const StylizedDualMetricBarsPlotConfig& cfg) {
            if (cfg.plot_height > 0.0f) return cfg.plot_height;
            const float avail_w = ImGui::GetContentRegionAvail().x;
            float h = avail_w / ImMax(0.5f, cfg.aspect_w_over_h);
            if (cfg.auto_height_min > 0.0f) h = ImMax(h, cfg.auto_height_min);
            if (cfg.auto_height_max > 0.0f) h = ImMin(h, cfg.auto_height_max);
            if (cfg.cap_by_avail_y) h = ImMin(h, ImGui::GetContentRegionAvail().y);
            return h;
        }
    } // namespace stylized_dual_metric_bars_plot_detail

    /// \brief Render grouped dual-metric bars with drag-and-drop category selection.
    /// \param data Input data arrays.
    /// \param state Runtime state (modified in-place).
    /// \param cfg Widget configuration.
    /// \thread_safety Not thread-safe.
    void StylizedDualMetricBarsPlot(
            const DualMetricBarsData& data,
            StylizedDualMetricBarsPlotState& state,
            const StylizedDualMetricBarsPlotConfig& cfg = {}
        );

#   ifdef IMGUIX_DEMO
    /// \brief Engineering demo for StylizedDualMetricBarsPlot.
    /// \thread_safety Not thread-safe.
    inline void DemoStylizedDualMetricBarsPlot();
#   endif

} // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#include "StylizedDualMetricBarsPlot.ipp"
#endif

#endif // _IMGUIX_WIDGETS_PLOT_STYLIZED_DUAL_METRIC_BARS_PLOT_HPP_INCLUDED
