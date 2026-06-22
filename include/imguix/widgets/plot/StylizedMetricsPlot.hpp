#pragma once
#ifndef _IMGUIX_WIDGETS_STYLIZED_PLOT_METRICS_PLOT_HPP_INCLUDED
#define _IMGUIX_WIDGETS_STYLIZED_PLOT_METRICS_PLOT_HPP_INCLUDED

/// \file StylizedMetricsPlot.hpp
/// \brief Plot categorical metrics as bars or lines with drag-and-drop.

#include <string>
#include <string_view>
#include <vector>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include <imguix/widgets/plot/MetricsPlotData.hpp>
#include <imguix/events/MetricsPlotUpdateEvent.hpp>
#include <imguix/widgets/controls/icon_button.hpp>


namespace ImGuiX::Widgets {

    /// \brief Configuration for MetricsPlot.
    struct StylizedMetricsPlotConfig {
        const char *id = "MetricsPlot";             ///< Plot identifier.
        const char *x_label = u8"X";                ///< X axis label.
        const char *y_label = u8"Y";                ///< Y axis label.
        const char *dnd_payload = u8"METRICS_PLOT"; ///< Drag-n-drop payload ID.
        const char *annotation_checkbox = u8"Annotations"; ///< Annotation checkbox label.
        const char *legend_checkbox = u8"Legend";   ///< Legend checkbox label.
        const char *button_show_all = u8"Show all"; ///< Select all button label.
        const char *button_reset = u8"Reset";       ///< Reset selection button label.
        const char* left_list_header = nullptr;     ///< e.g. "Series"; nullptr shows plain line.
        const char* plot_header = u8"Plot header";  ///< Header in right child before plot.
        const char* settings_header = u8"Chart settings"; ///< Settings section header.
        const char *value_fmt = "%+.2f";            ///< Format for values.
        const char *label_value_fmt = "%s = %+.2f"; ///< Annotation format.
        bool annotations_default_on = true;         ///< дефолтный показ аннотаций (только для bars)
        bool list_bordered       = false;           ///< Inner border for scroll list.
        bool y_axis_right        = true;            ///< Draw Y axis labels on the right.
        bool left_panel_bordered = true;            ///< Border or background for left checkbox panel.
        bool cap_by_avail_y      = false;           ///< Limit height by available space.
        bool show_tooltip        = true;            ///< Enable tooltips.
        bool input_enabled       = true;            ///< Enable plot mouse input for this frame.
        bool show_plot_header    = true;            ///< Show header row inside the plot panel.
        bool force_all_visible  = false;            ///< If true: all series are always plotted; left panel is hidden; menu shows only annotations.
        bool use_sticky_select  = false;            ///< Click list items to toggle instead of drag-and-drop.
        bool legend_force_off   = false;              ///< Жёстко отключить легенду (игнорирует state.show_legend)
        bool show_annotation_checkbox    = false;              ///< Только если force_all_visible true: включает отображение чекбокса аннотации
        ImPlotLocation    legend_loc    = ImPlotLocation_West;
        ImPlotLegendFlags legend_flags  = ImPlotLegendFlags_None;
        ImPlotLocation    legend_loc_compact   = ImPlotLocation_West;
        ImPlotLegendFlags legend_flags_compact = ImPlotLegendFlags_Outside;
        int default_show_index = -1;                ///< Одна серия по умолчанию; -1 = нет
        std::vector<int> default_show_indices;      ///< несколько серий; пусто = нет
        float bar_tooltip_snap_factor = 0.7f;       ///< Bars: X tolerance as fraction of bar width (0..1). 0.5 equals half width.
        float line_tooltip_snap_px    = 8.0f;       ///< Lines: tolerance in pixels to nearest point.
        float plot_height     = 0.0f;               ///< Height of the plot area.
        float aspect_w_over_h = 2.0f;               ///< Width to height ratio (2:1 default).
        float auto_height_min = 0.0f;               ///< Lower bound for auto height.
        float auto_height_max = 0.0f;               ///< 0 disables upper bound.
        float dnd_width       = 0.0f;               ///< Width of DND list (0 = auto).
        float panels_spacing  = 0.0f;               ///< Отступ между левой панелью и графиком.
        float dnd_scroll_h    = 0.0f;
        ImVec2 fit_padding{0.3f, 0.5f};             ///< Fit padding for auto-fit.
        ImVec2 right_child_padding{0.0f, 0.0f};     ///< Window padding for child with plot.
        int max_visible = 8; ///< Max items before legend disabled.
        ImVec4 drag_line_color{1.0f, 1.0f, 1.0f, 0.5f}; ///< Crosshair color.
        ImVec4 plot_color{0.012f, 0.66f, 0.95f, 1.0f};  ///< Default bar fill color.
        ImGuiX::Widgets::IconButtonConfig show_side_panel_cfg;
    };

    /// \brief Runtime state for MetricsPlot.
    /// \invariant dnd.size() matches data.labels.size().
    struct StylizedMetricsPlotState {
        /// \brief Drag-and-drop list item.
        struct DndItem {
            size_t index = 0;       ///< Index in data arrays.
            bool is_plot = false;   ///< True if item plotted.
            std::string_view label; ///< Display label.
            ImVec4 color{};         ///< Item color.
        };

        std::vector<DndItem> dnd;    ///< DND items.
        bool show_annotation = true; ///< Show value annotations.
        bool show_legend = true;     ///< Show legend.
        bool show_left_panel = true; ///< Show left panel.
        int update_counter = 0;      ///< Auto-fit counter.
        bool initialized = false;
    };

    namespace stylized_metrics_plot_detail {
        inline constexpr int kUpdateCounterMax = 5;

        inline float calc_plot_height(const StylizedMetricsPlotConfig& cfg) {
            if (cfg.plot_height > 0.0f) return cfg.plot_height;
            const float avail_w = ImGui::GetContentRegionAvail().x;
            float h = avail_w / ImMax(0.5f, cfg.aspect_w_over_h);
            if (cfg.auto_height_min > 0.0f) h = ImMax(h, cfg.auto_height_min);
            if (cfg.auto_height_max > 0.0f) h = ImMin(h, cfg.auto_height_max);
            if (cfg.cap_by_avail_y) h = ImMin(h, ImGui::GetContentRegionAvail().y);
            return h;
        }
    } // namespace detail

    /// \brief Calculate default DND list width.
    /// \return Recommended width in pixels.
    /// \thread_safety Not thread-safe.
    float CalcMetricsDndWidth();

    /// \brief Calculate button width inside DND list.
    /// \param dnd_width Width of the DND list.
    /// \pre dnd_width > 0.0f.
    /// \return Button width.
    /// \thread_safety Not thread-safe.
    float CalcMetricsDndButtonWidth(float dnd_width);

    /// \brief Render metrics plot with drag-and-drop selection.
    /// \param data Input data arrays.
    /// \param state Runtime state (modified in-place).
    /// \param cfg Widget configuration.
    /// \thread_safety Not thread-safe.
    /// \complexity O(m + n) where m is categories and n is total points.
    /// \code{.cpp}
    /// MetricsPlotData data;
    /// StylizedMetricsPlotState state;
    /// StylizedMetricsPlot(data, state);
    /// \endcode
    void StylizedMetricsPlot(const MetricsPlotData &data, StylizedMetricsPlotState &state,
                     const StylizedMetricsPlotConfig &cfg = {});
                     
#   ifdef IMGUIX_DEMO
    /// \brief Engineering demo for MetricsPlot.
    /// \details Shows: (1) categorical bars; (2) time-series lines.
    /// \note Uses static state to persist selections between frames.
    /// \thread_safety Not thread-safe.
    inline void DemoMetricsPlot() {
        // ---------- 1) Bars: subsystem latency ----------
        ImGui::TextUnformatted("MetricsPlot / Bars: Subsystem latency");
        {
            static MetricsPlotData  data;
            static StylizedMetricsPlotState state;
            static bool init = false;

            if (!init) {
                // Categories (subsystems)
                const char* kLabels[] = {
                    "ADC","DSP","IO","CTRL","UI","NET","LOG","FS","DB","PHY","RF","PMU"
                };
                data.labels.assign(std::begin(kLabels), std::end(kLabels));
                data.values.resize(data.labels.size());
                // Synthetic values in ms; some negative for demo
                for (size_t i = 0; i < data.values.size(); ++i)
                    data.values[i] = (i % 2 ? +1.0 : -1.0) * (5.0 + 3.0 * std::sin(0.45 * double(i)));
                init = true;
            }

            StylizedMetricsPlotConfig cfg;
            cfg.id           = "MetricsPlot_Bars";
            cfg.x_label      = "Subsystem";
            cfg.left_list_header = "Series";
            cfg.y_label      = "Latency, ms";
            cfg.plot_height  = 0.0f;
            cfg.max_visible  = 8;            // >8 disables legend
            cfg.value_fmt    = "%+.2f";
            cfg.label_value_fmt = "%s = %+.2f";

            StylizedMetricsPlot(data, state, cfg);
        }
        
        // ---------- 2) Bars: Weekday metric (with X tick labels) ----------
        ImGui::TextUnformatted("MetricsPlot / Bars: Weekday metric (with X tick labels)");
        {
            static MetricsPlotData  data;
            static StylizedMetricsPlotState state;
            static bool init = false;

            if (!init) {
                // 7 категорий — дни недели
                const char* kDays[] = { "Mon","Tue","Wed","Thu","Fri","Sat","Sun" };
                data.labels.assign(std::begin(kDays), std::end(kDays));

                // Значения метрики (пример)
                data.values = { 12.5, 9.8, 11.2, 10.7, 13.3, 7.4, 6.1 };

                // ВКЛ подписи оси X: заданы в данных (декорации X появятся автоматически)
                data.tick_labels_x = { "Mon","Tue","Wed","Thu","Fri","Sat","Sun" };

                init = true;
            }

            StylizedMetricsPlotConfig cfg;
            cfg.id              = "MetricsPlot_Bars_Weekday";
            cfg.x_label         = "Weekday";
            cfg.y_label         = "Metric";
            cfg.value_fmt       = "%.1f";
            cfg.label_value_fmt = "%s: %.1f";
            cfg.max_visible     = 8;  // запас на будущее

            // Бар-режим активен, т.к. values не пустой
            StylizedMetricsPlot(data, state, cfg);
        }

        // ---------- 3) Lines: temperatures over time ----------
        ImGui::TextUnformatted("MetricsPlot / Lines: Sensor temperatures");
        {
            static MetricsPlotData  data;
            static StylizedMetricsPlotState state;
            static bool init = false;

            if (!init) {
                const int series = 3;  // three sensors
                data.labels = { "Temp A", "Temp B", "Temp C" };
                data.line_x.resize(series);
                data.line_y.resize(series);

                const int N = 120; // ~120 time samples
                for (int s = 0; s < series; ++s) {
                    data.line_x[s].resize(N);
                    data.line_y[s].resize(N);
                    for (int i = 0; i < N; ++i) {
                        data.line_x[s][i] = double(i); // time in seconds
                        data.line_y[s][i] = 25.0 + 2.0 * s + 5.0 * std::sin(0.08 * i + 0.6 * s);
                    }
                }
                init = true;
            }

            StylizedMetricsPlotConfig cfg;
            cfg.id           = "MetricsPlot_Lines";
            cfg.x_label      = "Time, s";
            cfg.y_label      = "Temperature, °C";
            cfg.plot_height  = 0.0f;
            cfg.max_visible  = 8;
            cfg.value_fmt    = "%.1f";
            cfg.default_show_index = 1;

            // Line mode enabled when data.values is empty
            data.values.clear();
            StylizedMetricsPlot(data, state, cfg);
        }
        // ---------- 4) Compact: Bars, all series, no left panel ----------
        ImGui::TextUnformatted("MetricsPlot / Compact: All series (no left panel)");
        {
            static MetricsPlotData  data;
            static StylizedMetricsPlotState state;
            static bool init = false;

            if (!init) {
                // Каналы CH0..CH9
                const char* kLabels[] = { "CH0","CH1","CH2","CH3","CH4","CH5","CH6","CH7","CH8","CH9" };
                data.labels.assign(std::begin(kLabels), std::end(kLabels));
                data.values.resize(data.labels.size());
                // Синтетический throughput (MB/s)
                for (size_t i = 0; i < data.values.size(); ++i)
                    data.values[i] = 120.0 + 35.0 * std::sin(0.4 * double(i));
                init = true;
            }

            StylizedMetricsPlotConfig cfg;
            cfg.id              = "MetricsPlot_CompactAll";
            cfg.x_label         = "Channel";
            cfg.y_label         = "Throughput, MB/s";
            cfg.value_fmt       = "%.1f";
            cfg.label_value_fmt = "%s: %.1f MB/s";
            cfg.max_visible     = 16;                 // при >=6 легенда авто-скроется
            cfg.force_all_visible = true;            // <-- ключ: без левого чилда, все серии на плоте

            // Бар-режим (values не пустой), аннотации можно включать из контекст-меню
            StylizedMetricsPlot(data, state, cfg);
        }
        // ---------- 5) Compact: Bars with X ticks (24h), legend forced off ----------
        ImGui::TextUnformatted("MetricsPlot / Compact: Hourly metric (bars with X ticks, no legend)");
        {
            static MetricsPlotData  data;
            static StylizedMetricsPlotState state;
            static bool init = false;

            if (!init) {
                // 24 часа
                static const char* kHours[] = {
                    "00","01","02","03","04","05","06","07","08","09","10","11",
                    "12","13","14","15","16","17","18","19","20","21","22","23"
                };
                // метки серий (для DnD/легенды; легенда будет выключена)
                data.labels.assign(std::begin(kHours), std::end(kHours));
                // тики X — показываем подписи часов на оси X
                data.tick_labels_x.assign(std::begin(kHours), std::end(kHours));

                // синтетическая почасовая метрика (диурнальный профиль)
                data.values.resize(24);
                for (int h = 0; h < 24; ++h) {
                    double t = (2.0 * 3.14159265358979323846 * h) / 24.0;
                    data.values[h] = 60.0
                                   + 25.0 * std::sin(t - 3.14159265358979323846 * 0.5)
                                   + 8.0  * std::sin(2.0 * t);
                }
                init = true;
            }

            StylizedMetricsPlotConfig cfg;
            cfg.id                = "MetricsPlot_CompactAll_24h";
            cfg.x_label           = "Hour";
            cfg.y_label           = "Metric";
            cfg.value_fmt         = "%.1f";
            cfg.label_value_fmt   = "%s: %.1f";
            cfg.max_visible       = 32;          // не критично: легенда всё равно офф
            cfg.force_all_visible = true;        // без левой панели, все серии
            cfg.legend_force_off  = true;        // легенда принудительно отключена
            cfg.annotations_default_on = false;

            // Бар-режим (values не пустой); тики X появятся из data.tick_labels_x
            StylizedMetricsPlot(data, state, cfg);
        }

        // ---------- 6) Sticky select: toggle series by click ----------
        ImGui::TextUnformatted("MetricsPlot / Sticky select demo");
        {
            static MetricsPlotData  data;
            static StylizedMetricsPlotState state;
            static bool init = false;

            if (!init) {
                const char* kLabels[] = {"A","B","C"};
                data.labels.assign(std::begin(kLabels), std::end(kLabels));
                data.values = {1.0, 2.0, 3.0};
                init = true;
            }

            StylizedMetricsPlotConfig cfg;
            cfg.id              = "MetricsPlot_Sticky";
            cfg.left_list_header = "Series";
            cfg.use_sticky_select = true;
            StylizedMetricsPlot(data, state, cfg);
        }
    }
#   endif

} // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#include "StylizedMetricsPlot.ipp"
#endif

#endif // _IMGUIX_WIDGETS_STYLIZED_PLOT_METRICS_PLOT_HPP_INCLUDED
