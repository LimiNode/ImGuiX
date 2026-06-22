#pragma once
#ifndef _IMGUIX_WIDGETS_PLOT_STYLIZED_DUAL_METRIC_BARS_PLOT_IPP_INCLUDED
#define _IMGUIX_WIDGETS_PLOT_STYLIZED_DUAL_METRIC_BARS_PLOT_IPP_INCLUDED

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    namespace stylized_dual_metric_bars_plot_detail {

        struct Ctx {
            const DualMetricBarsData& data;
            StylizedDualMetricBarsPlotState& state;
            const StylizedDualMetricBarsPlotConfig& cfg;
            float dnd_w = 0.0f;
            float btn_w = 0.0f;
            float plot_h = 0.0f;
            int plotted = 0;
        };

        inline bool has_valid_data(const DualMetricBarsData& data) {
            return data.metric1.size() == data.labels.size()
                && data.metric2.size() == data.labels.size();
        }

        inline void sync_dnd(Ctx& c) {
            c.state.dnd.resize(c.data.labels.size());
            for (size_t i = 0; i < c.data.labels.size(); ++i) {
                auto& item = c.state.dnd[i];
                item.index = i;
                item.label = c.data.labels[i];
                item.color = ImPlot::GetColormapColor(static_cast<int>(i));
            }

            if (c.cfg.force_all_visible) {
                for (auto& it : c.state.dnd) it.is_plot = true;
            }
        }

        inline int count_plotted(const StylizedDualMetricBarsPlotState& state) {
            int count = 0;
            for (const auto& it : state.dnd) {
                if (it.is_plot) ++count;
            }
            return count;
        }

        inline DualMetricBarsData make_filtered_data(Ctx& c) {
            DualMetricBarsData filtered;
            filtered.title = c.data.title;
            filtered.label_x = c.data.label_x;
            filtered.label_y1 = c.data.label_y1;
            filtered.label_y2 = c.data.label_y2;
            filtered.metric1_name = c.data.metric1_name;
            filtered.metric2_name = c.data.metric2_name;
            filtered.metric1_color = c.data.metric1_color;
            filtered.metric2_color = c.data.metric2_color;

            if (!has_valid_data(c.data)) {
                return filtered;
            }

            for (const auto& it : c.state.dnd) {
                if (!it.is_plot || it.index >= c.data.labels.size()) continue;
                filtered.labels.push_back(c.data.labels[it.index]);
                filtered.metric1.push_back(c.data.metric1[it.index]);
                filtered.metric2.push_back(c.data.metric2[it.index]);
            }

            return filtered;
        }

        inline void draw_controls(Ctx& c) {
            const bool controls_disabled = !c.cfg.input_enabled;
            if (controls_disabled) ImGui::BeginDisabled();

            if (ImGui::Checkbox(c.cfg.annotation_checkbox, &c.state.plot_state.show_annotations)) {
                c.state.plot_state.update_counter = kUpdateCounterMax;
            }

            if (!c.cfg.legend_force_off) {
                if (ImGui::Checkbox(c.cfg.legend_checkbox, &c.state.plot_state.show_legend)) {
                    c.state.plot_state.update_counter = kUpdateCounterMax;
                }
            }

            if (c.cfg.force_all_visible) {
                if (controls_disabled) ImGui::EndDisabled();
                return;
            }

            if (ImGui::Button(c.cfg.button_show_all, ImVec2(c.btn_w, 0))) {
                for (auto& it : c.state.dnd) it.is_plot = true;
                c.plotted = static_cast<int>(c.state.dnd.size());
                c.state.plot_state.update_counter = kUpdateCounterMax;
            }

            if (ImGui::Button(c.cfg.button_reset, ImVec2(c.btn_w, 0))) {
                for (auto& it : c.state.dnd) it.is_plot = false;
                c.plotted = 0;
                c.state.plot_state.update_counter = kUpdateCounterMax;
            }

            if (controls_disabled) ImGui::EndDisabled();
        }

        inline void draw_left_panel(Ctx& c) {
            c.plotted = count_plotted(c.state);

            if (c.cfg.left_panel_bordered) {
                const ImGuiStyle& st = ImGui::GetStyle();
                ImGui::PushStyleColor(ImGuiCol_ChildBg, st.Colors[ImGuiCol_FrameBg]);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, st.FrameRounding);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, st.FrameBorderSize);
            }

            ImGui::BeginChild("##child_left", ImVec2(c.dnd_w, c.plot_h), ImGuiChildFlags_Borders);
            {
                if (c.cfg.left_list_header) ImGui::SeparatorText(c.cfg.left_list_header);
                else ImGui::Separator();

                ImGuiChildFlags list_flags = ImGuiChildFlags_None;
                if (c.cfg.list_bordered) list_flags |= ImGuiChildFlags_Borders;

                ImGui::BeginChild("##dnd_scroll", ImVec2(0, c.cfg.dnd_scroll_h), list_flags);
                {
                    if (!c.cfg.input_enabled) ImGui::BeginDisabled();

                    for (size_t k = 0; k < c.state.dnd.size(); ++k) {
                        auto& it = c.state.dnd[k];

                        ImPlot::ItemIcon(it.color);
                        ImGui::SameLine();

                        bool selected = it.is_plot;
                            if (ImGui::Selectable(it.label.data(), selected, 0, ImVec2(c.dnd_w, 0))) {
                                it.is_plot = !it.is_plot;
                                c.plotted += it.is_plot ? 1 : -1;
                                c.state.plot_state.update_counter = kUpdateCounterMax;
                            }

                        if (c.cfg.input_enabled && ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                            ImGui::TextUnformatted(it.label.data());
                            ImGui::PopTextWrapPos();
                            ImGui::EndTooltip();
                        }

                        if (c.cfg.input_enabled && !c.cfg.use_sticky_select) {
                            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                                const int payload_index = static_cast<int>(k);
                                ImGui::SetDragDropPayload(c.cfg.dnd_payload, &payload_index, sizeof(payload_index));
                                ImPlot::ItemIcon(it.color);
                                ImGui::SameLine();
                                ImGui::TextUnformatted(it.label.data());
                                ImGui::EndDragDropSource();
                            }
                        }
                    }

                    if (c.cfg.input_enabled && !c.cfg.use_sticky_select) {
                        if (ImGui::BeginDragDropTarget()) {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(c.cfg.dnd_payload)) {
                                const int i = *static_cast<const int*>(payload->Data);
                                if (i >= 0 && static_cast<size_t>(i) < c.state.dnd.size()) {
                                    c.state.dnd[static_cast<size_t>(i)].is_plot = false;
                                    c.state.plot_state.update_counter = kUpdateCounterMax;
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                    }

                    if (!c.cfg.input_enabled) ImGui::EndDisabled();
                }
                ImGui::EndChild();

                draw_controls(c);
            }
            ImGui::EndChild();

            if (c.cfg.left_panel_bordered) {
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor();
            }
        }

        inline void begin_right_child(Ctx& c) {
            const ImVec4 bg = ImPlot::GetStyle().Colors[ImPlotCol_FrameBg];
            ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, c.cfg.right_child_padding);
            ImGui::BeginChild("##child_right", ImVec2(-1, c.plot_h), ImGuiChildFlags_Borders);
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        inline void end_right_child() {
            ImGui::EndChild();
        }

        inline void accept_drop(Ctx& c) {
            if (c.cfg.use_sticky_select) return;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(c.cfg.dnd_payload)) {
                const int i = *static_cast<const int*>(payload->Data);
                if (i >= 0 && static_cast<size_t>(i) < c.state.dnd.size()) {
                    auto& item = c.state.dnd[static_cast<size_t>(i)];
                    if (!item.is_plot) {
                        item.is_plot = true;
                        c.plotted = count_plotted(c.state);
                    }
                    c.state.plot_state.update_counter = kUpdateCounterMax;
                }
            }
        }

        inline void draw_plot_context_popup(Ctx& c) {
            if (c.cfg.input_enabled && ImGui::BeginPopupContextItem("##stylized_dual_metric_bars_plot_context")) {
                ImGui::TextUnformatted(c.cfg.settings_header);
                ImGui::Separator();
                draw_controls(c);
                ImGui::EndPopup();
            }
        }

        inline void draw_dual_bars_plot(Ctx& c, const DualMetricBarsData& filtered) {
            DualMetricBarsPlotConfig plot_cfg = c.cfg.plot;
            plot_cfg.id = c.cfg.id;
            plot_cfg.drag_line_color = c.cfg.plot.drag_line_color;

            const size_t n = filtered.labels.size();
            const bool valid = has_valid_data(filtered);
            const float fit_x = 1.0f / static_cast<float>(std::max<size_t>(1, n));
            const bool want_legend = !c.cfg.legend_force_off && c.state.plot_state.show_legend;
            ImPlotFlags flags = ImPlotFlags_NoMenus | ImPlotFlags_NoFrame;
            if (!want_legend) flags |= ImPlotFlags_NoLegend;
            if (!c.cfg.input_enabled) flags |= ImPlotFlags_NoInputs;

            if (c.state.plot_state.update_counter > 0) {
                if (c.state.plot_state.update_counter == kUpdateCounterMax
                    || c.state.plot_state.update_counter == 1) {
                    ImPlot::SetNextAxisToFit(ImAxis_X1);
                    ImPlot::SetNextAxisToFit(ImAxis_Y1);
                    ImPlot::SetNextAxisToFit(ImAxis_Y2);
                }
                --c.state.plot_state.update_counter;
            }

            ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, ImVec2(fit_x, c.cfg.fit_padding.y));
            if (ImPlot::BeginPlot(plot_cfg.id, ImVec2(-1, -1), flags)) {
                const bool left_panel_hidden = c.cfg.force_all_visible || !c.state.show_left_panel;
                const ImPlotLocation loc = left_panel_hidden ? c.cfg.legend_loc_compact : c.cfg.legend_loc;
                const ImPlotLegendFlags legend_flags =
                    left_panel_hidden ? c.cfg.legend_flags_compact : c.cfg.legend_flags;
                ImPlot::SetupLegend(loc, legend_flags);
                ImPlot::SetupAxes(filtered.label_x.c_str(), filtered.label_y1.c_str());
                ImPlot::SetupAxis(ImAxis_Y2, filtered.label_y2.c_str(), ImPlotAxisFlags_AuxDefault);

                std::vector<double> positions(n);
                std::vector<const char*> ticks(n);
                for (size_t i = 0; i < n; ++i) {
                    positions[i] = static_cast<double>(i);
                    ticks[i] = filtered.labels[i].empty() ? "<Empty>" : filtered.labels[i].c_str();
                }

                if (n > 0) {
                    ImPlot::SetupAxisTicks(ImAxis_X1, positions.data(), static_cast<int>(n), ticks.data());
                }
                ImPlot::SetupAxisLimits(ImAxis_X1, -1.0, static_cast<double>(n));

                double max1 = 0.0;
                double max2 = 0.0;
                if (valid) {
                    for (size_t i = 0; i < n; ++i) {
                        max1 = std::max(max1, filtered.metric1[i]);
                        max2 = std::max(max2, filtered.metric2[i]);
                    }
                }
                const double y1_max = plot_cfg.force_y1_percent
                    ? 100.0
                    : std::max(100.0, max1) + plot_cfg.y_indent;
                const double y2_max = max2 + plot_cfg.y_indent;
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, y1_max);
                ImPlot::SetupAxisLimits(ImAxis_Y2, 0.0, y2_max);

                if (valid && n > 0) {
                    const float off1 = -plot_cfg.bar_width * 0.5f;
                    const float off2 = plot_cfg.bar_width * 0.5f;

                    ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
                    ImPlot::SetNextFillStyle(filtered.metric1_color);
                    ImPlot::PlotBars(
                            filtered.metric1_name.c_str(),
                            filtered.metric1.data(),
                            static_cast<int>(n),
                            plot_cfg.bar_width,
                            off1);

                    ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
                    ImPlot::SetNextFillStyle(filtered.metric2_color);
                    ImPlot::PlotBars(
                            filtered.metric2_name.c_str(),
                            filtered.metric2.data(),
                            static_cast<int>(n),
                            plot_cfg.bar_width,
                            off2);

                    const char* fmt1 = plot_cfg.metric1_fmt
                        ? plot_cfg.metric1_fmt
                        : (plot_cfg.force_y1_percent ? "%.1f%%" : "%.2f");
                    const char* fmt2 = plot_cfg.metric2_fmt ? plot_cfg.metric2_fmt : "%.0f";
                    const bool hov1 = c.cfg.input_enabled && ImPlot::IsLegendEntryHovered(filtered.metric1_name.c_str());
                    const bool hov2 = c.cfg.input_enabled && ImPlot::IsLegendEntryHovered(filtered.metric2_name.c_str());
                    const bool show1 = c.state.plot_state.show_annotations && (!hov2 || hov1);
                    const bool show2 = c.state.plot_state.show_annotations && (!hov1 || hov2);
                    char buf[32];
                    for (size_t i = 0; i < n; ++i) {
                        const double px = positions[i];
                        if (show1) {
                            ImVec4 col = filtered.metric1_color;
                            col.w = 0.5f;
                            const ImVec2 off = filtered.metric1[i] >= 0.0
                                ? ImVec2(0, -5)
                                : ImVec2(0, 5);
                            std::snprintf(buf, sizeof(buf), fmt1, filtered.metric1[i]);
                            ImPlot::Annotation(
                                    px + off1,
                                    filtered.metric1[i],
                                    col,
                                    off,
                                    false,
                                    "%s",
                                    buf);
                        }
                        if (show2) {
                            ImVec4 col = filtered.metric2_color;
                            col.w = 0.5f;
                            const ImVec2 off = filtered.metric2[i] >= 0.0
                                ? ImVec2(0, -5)
                                : ImVec2(0, 5);
                            std::snprintf(buf, sizeof(buf), fmt2, filtered.metric2[i]);
                            ImPlot::Annotation(
                                    px + off2,
                                    filtered.metric2[i],
                                    col,
                                    off,
                                    false,
                                    "%s",
                                    buf);
                        }
                    }
                }

                if (c.cfg.input_enabled && !c.cfg.use_sticky_select && ImPlot::BeginDragDropTargetPlot()) {
                    accept_drop(c);
                    ImPlot::EndDragDropTarget();
                }

                if (c.cfg.input_enabled && !c.cfg.use_sticky_select && ImPlot::BeginDragDropTargetLegend()) {
                    accept_drop(c);
                    ImPlot::EndDragDropTarget();
                }

                if (c.cfg.input_enabled && ImPlot::IsPlotHovered()) {
                    ImPlotPoint mouse = ImPlot::GetPlotMousePos();
                    static ImPlotDragToolFlags tool_flags =
                        ImPlotDragToolFlags_None | ImPlotDragToolFlags_NoInputs;
                    ImPlot::DragLineX(1, &mouse.x, plot_cfg.drag_line_color, 1, tool_flags);
                    ImPlot::DragLineY(2, &mouse.y, plot_cfg.drag_line_color, 1, tool_flags);
                }

                if (c.cfg.input_enabled) {
                    draw_plot_context_popup(c);
                }
                ImPlot::EndPlot();
            }
            ImPlot::PopStyleVar();
        }

        inline void draw_plot(Ctx& c) {
            const DualMetricBarsData filtered = make_filtered_data(c);
            if (ImGui::BeginChild(
                    "##plot_layout",
                    ImVec2(-1.0f, -1.0f),
                    ImGuiChildFlags_AlwaysUseWindowPadding,
                    ImGuiWindowFlags_None)) {
                draw_dual_bars_plot(c, filtered);
            }
            ImGui::EndChild();
        }

    } // namespace stylized_dual_metric_bars_plot_detail

    IMGUIX_IMPL_INLINE void StylizedDualMetricBarsPlot(
            const DualMetricBarsData& data,
            StylizedDualMetricBarsPlotState& state,
            const StylizedDualMetricBarsPlotConfig& cfg
        ) {
        ImGui::PushID(cfg.id);

        stylized_dual_metric_bars_plot_detail::Ctx c{data, state, cfg};
        c.dnd_w = cfg.dnd_width > 0.0f ? cfg.dnd_width : CalcMetricsDndWidth();
        c.btn_w = CalcMetricsDndButtonWidth(c.dnd_w);
        c.plot_h = IM_TRUNC(stylized_dual_metric_bars_plot_detail::calc_plot_height(cfg));

        stylized_dual_metric_bars_plot_detail::sync_dnd(c);

        if (!c.state.initialized) {
            auto mark_plot = [&](int idx) {
                if (idx >= 0 && idx < static_cast<int>(c.state.dnd.size())) {
                    c.state.dnd[static_cast<size_t>(idx)].is_plot = true;
                }
            };

            if (!c.cfg.default_show_indices.empty()) {
                for (int idx : c.cfg.default_show_indices) mark_plot(idx);
                c.state.plot_state.update_counter =
                    stylized_dual_metric_bars_plot_detail::kUpdateCounterMax;
            } else if (c.cfg.default_show_index >= 0) {
                mark_plot(c.cfg.default_show_index);
                c.state.plot_state.update_counter =
                    stylized_dual_metric_bars_plot_detail::kUpdateCounterMax;
            }

            c.state.plot_state.show_annotations = c.cfg.annotations_default_on;
            c.state.initialized = true;
        }

        if (c.cfg.legend_force_off) {
            c.state.plot_state.show_legend = false;
        }

        ImGui::BeginGroup();
        if (!c.cfg.force_all_visible) {
            if (c.state.show_left_panel) {
                stylized_dual_metric_bars_plot_detail::draw_left_panel(c);
                if (cfg.panels_spacing > 0.0f) {
                    ImGui::SameLine(0.0f, cfg.panels_spacing);
                } else {
                    ImGui::SameLine();
                }
            }
        } else {
            c.plotted = stylized_dual_metric_bars_plot_detail::count_plotted(c.state);
        }

        stylized_dual_metric_bars_plot_detail::begin_right_child(c);
        ImGui::TextUnformatted(c.cfg.plot_header);
        if (!c.cfg.force_all_visible) {
            ImGui::SameLine();
            const char* icon = c.state.show_left_panel ? "\xef\x81\x94" : "\xef\x81\x93";
            if (!c.cfg.input_enabled) ImGui::BeginDisabled();
            if (ImGuiX::Widgets::IconButtonCentered(
                    "show_side_panel_bttn",
                    icon,
                    c.cfg.show_side_panel_cfg)) {
                c.state.show_left_panel = !c.state.show_left_panel;
                c.state.plot_state.update_counter =
                    stylized_dual_metric_bars_plot_detail::kUpdateCounterMax;
            }
            if (!c.cfg.input_enabled) ImGui::EndDisabled();
        }
        if (c.cfg.force_all_visible && c.cfg.show_annotation_checkbox) {
            ImGui::SameLine();
            if (!c.cfg.input_enabled) ImGui::BeginDisabled();
            if (ImGui::Checkbox(c.cfg.annotation_checkbox, &c.state.plot_state.show_annotations)) {
                c.state.plot_state.update_counter =
                    stylized_dual_metric_bars_plot_detail::kUpdateCounterMax;
            }
            if (!c.cfg.input_enabled) ImGui::EndDisabled();
        }
        stylized_dual_metric_bars_plot_detail::draw_plot(c);
        stylized_dual_metric_bars_plot_detail::end_right_child();
        ImGui::EndGroup();

        ImGui::PopID();
    }

#   ifdef IMGUIX_DEMO
    inline void DemoStylizedDualMetricBarsPlot() {
        static DualMetricBarsData data;
        static StylizedDualMetricBarsPlotState state;
        static bool init = false;
        if (!init) {
            data.title = "Signal / Winrate";
            data.label_x = "Signal";
            data.label_y1 = "Winrate";
            data.label_y2 = "Trades";
            data.metric1_name = "Winrate";
            data.metric2_name = "Trades";
            data.labels = {"Signal A", "Signal B", "Signal C"};
            data.metric1 = {62.5, 48.0, 71.2};
            data.metric2 = {120.0, 85.0, 64.0};
            init = true;
        }

        StylizedDualMetricBarsPlotConfig cfg;
        cfg.id = "StylizedDualMetricBarsPlot_Demo";
        cfg.left_list_header = "Signals";
        cfg.plot_header = "Signal / Winrate";
        cfg.default_show_indices = {0, 1};
        cfg.plot.force_y1_percent = true;
        StylizedDualMetricBarsPlot(data, state, cfg);
    }
#   endif

} // namespace ImGuiX::Widgets

#endif // _IMGUIX_WIDGETS_PLOT_STYLIZED_DUAL_METRIC_BARS_PLOT_IPP_INCLUDED
