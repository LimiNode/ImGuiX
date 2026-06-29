#include <algorithm>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include <imgui.h>
#include <implot.h>
#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE float StylizedCalcMetricsDndWidth() {
        const ImGuiStyle& st = ImGui::GetStyle();
        return ImGui::GetFontSize() * 8.0f + st.FramePadding.x * 2.0f;
    }

    IMGUIX_IMPL_INLINE float StylizedCalcMetricsDndButtonWidth(float dnd_width) {
        const ImGuiStyle& st = ImGui::GetStyle();
        return dnd_width - st.FramePadding.x * 2.0f;
    }

    namespace stylized_metrics_plot_detail {

        struct Ctx {
            const MetricsPlotData& data;
            StylizedMetricsPlotState& state;
            const StylizedMetricsPlotConfig& cfg;
            float dnd_w = 0.0f;
            float btn_w = 0.0f;
            float plot_h = 0.0f;
            int plotted = 0;
        };

        inline void sync_dnd(Ctx& c) {
            // Match DND items with data labels.
            c.state.dnd.resize(c.data.labels.size());
            for (size_t i = 0; i < c.data.labels.size(); ++i) {
                auto& item = c.state.dnd[i];
                item.index = i;
                item.label = c.data.labels[i];
                item.color = ImPlot::GetColormapColor(static_cast<int>(i));
            }
            // Force show-all mode: plot every series
            if (c.cfg.force_all_visible) {
                for (auto& it : c.state.dnd) it.is_plot = true;
                
                const int series = (int)c.state.dnd.size();
                if (series >= c.cfg.max_visible) {
                    if (c.state.show_legend) {
                        c.state.show_legend = false;
                        c.state.update_counter = kUpdateCounterMax;
                    }
                }
            }
        }

        inline bool is_series_visible(const char* label) {
            // Return true if legend shows the series.
            if (ImPlotItem* it = ImPlot::GetItem(label)) return it->Show;
            return true;
        }

        inline void toggle_plot_item(Ctx& c, StylizedMetricsPlotState::DndItem& item) {
            item.is_plot = !item.is_plot;
            c.plotted += item.is_plot ? 1 : -1;
            if (item.is_plot && c.state.show_legend && c.plotted >= c.cfg.max_visible) {
                c.state.show_legend = false;
            }
            c.state.update_counter = kUpdateCounterMax;
        }

        inline float calc_checkbox_stack_height(int count) {
            if (count <= 0) return 0.0f;
            const ImGuiStyle& style = ImGui::GetStyle();
            return ImGui::GetFrameHeight() * static_cast<float>(count) +
                style.ItemSpacing.y * static_cast<float>(count - 1);
        }

        inline void align_next_block_to_bottom(float block_h) {
            if (block_h <= 0.0f) return;
            const float bottom_y = ImGui::GetWindowContentRegionMax().y;
            ImGui::SetCursorPosY(ImMax(
                ImGui::GetCursorPosY(),
                bottom_y - block_h));
        }

        inline void same_line_right_aligned_checkbox(const char* label) {
            const ImGuiStyle& style = ImGui::GetStyle();
            const float checkbox_w =
                ImGui::GetFrameHeight() +
                style.ItemInnerSpacing.x +
                ImGui::CalcTextSize(label).x;
            const float right_x = ImGui::GetWindowContentRegionMax().x;

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImMax(
                ImGui::GetCursorPosX(),
                right_x - checkbox_w));
        }

        inline void draw_controls(Ctx& c, bool align_checkboxes_to_bottom = false) {
            // Checkboxes and action buttons shared by panels.
            const bool controls_disabled = !c.cfg.input_enabled;
            if (controls_disabled) ImGui::BeginDisabled();

            if (!c.cfg.force_all_visible) {
                if (ImGui::Button(c.cfg.button_show_all, ImVec2(c.btn_w, 0))) {
                    if (c.state.dnd.size() >= static_cast<size_t>(c.cfg.max_visible)) {
                        c.state.show_legend = false;
                    }
                    for (auto& it : c.state.dnd) it.is_plot = true;
                    c.state.update_counter = kUpdateCounterMax;
                }

                if (ImGui::Button(c.cfg.button_reset, ImVec2(c.btn_w, 0))) {
                    for (auto& it : c.state.dnd) it.is_plot = false;
                }
            }

            const bool show_annotation_checkbox = !c.data.values.empty();
            const bool show_legend_checkbox = !c.cfg.legend_force_off;
            if (align_checkboxes_to_bottom) {
                align_next_block_to_bottom(calc_checkbox_stack_height(
                    static_cast<int>(show_annotation_checkbox) +
                    static_cast<int>(show_legend_checkbox)));
            }

            if (show_annotation_checkbox) {
                if (ImGui::Checkbox(c.cfg.annotation_checkbox, &c.state.show_annotation)) {
                    c.state.update_counter = kUpdateCounterMax;
                }
            }
            
            if (show_legend_checkbox) {
                if (!c.state.show_legend && c.plotted >= c.cfg.max_visible) {
                    ImGui::BeginDisabled(true);
                    ImGui::Checkbox(c.cfg.legend_checkbox, &c.state.show_legend);
                    ImGui::EndDisabled();
                } else {
                    ImGui::Checkbox(c.cfg.legend_checkbox, &c.state.show_legend);
                }
            }

            if (controls_disabled) ImGui::EndDisabled();
        }

        inline void draw_left_panel(Ctx& c) {
            // Left panel with selectable series and DND source.
            
            c.plotted = 0;
            for (const auto& it : c.state.dnd) {
                if (it.is_plot) ++c.plotted;
            }

            if (c.cfg.left_panel_bordered) {
                const ImGuiStyle& st = ImGui::GetStyle();
                ImGui::PushStyleColor(ImGuiCol_ChildBg, st.Colors[ImGuiCol_FrameBg]); // child bg
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, st.FrameRounding);   // child rounding
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, st.FrameBorderSize); // child border
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
                        if (c.cfg.input_enabled && ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                            ImGui::TextUnformatted(it.label.data());
                            ImGui::PopTextWrapPos();
                            ImGui::EndTooltip();
                        }

                        ImGui::SameLine();
                        bool selected = it.is_plot;
                        if (ImGui::Selectable(it.label.data(), selected, 0, ImVec2(c.dnd_w, 0))) {
                            toggle_plot_item(c, it);
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
                                int i = *static_cast<const int*>(payload->Data);
                                if (i >= 0 && static_cast<size_t>(i) < c.state.dnd.size()) {
                                    c.state.dnd[i].is_plot = false;
                                    c.state.update_counter = kUpdateCounterMax;
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                    }

                    if (!c.cfg.input_enabled) ImGui::EndDisabled();
                }
                ImGui::EndChild();

                draw_controls(c, true);
            }
            ImGui::EndChild();

            if (c.cfg.left_panel_bordered) {
                ImGui::PopStyleVar(2); // child rounding, border
                ImGui::PopStyleColor(); // child bg
            }
        }

        inline void begin_right_child(Ctx& c) {
            // Begin right plot panel with custom background.
            const ImVec4 bg = ImPlot::GetStyle().Colors[ImPlotCol_FrameBg];
            ImGui::PushStyleColor(ImGuiCol_ChildBg, bg); // child bg
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, c.cfg.right_child_padding); // window padding
            ImGui::BeginChild("##child_right", ImVec2(-1, c.plot_h), ImGuiChildFlags_Borders);
            ImGui::PopStyleVar();   // window padding
            ImGui::PopStyleColor(); // child bg
        }

        inline void end_right_child() {
            // End right plot panel.
            ImGui::EndChild();
        }

        inline void draw_lines(Ctx& c) {
            // Plot line series with tooltip for nearest point.
            ImPlotAxisFlags y_flags = c.cfg.y_axis_right ? ImPlotAxisFlags_Opposite : ImPlotAxisFlags_None;
            ImPlot::SetupAxis(ImAxis_Y1, c.cfg.y_label, y_flags);
            ImPlot::SetupAxis(ImAxis_X1, c.cfg.x_label);
            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
            ImPlot::SetupAxisFormat(ImAxis_Y1, c.cfg.value_fmt);

            // Validate that each line series has matching X/Y array lengths.
            std::vector<bool> skip(c.state.dnd.size(), false);
            for (size_t k = 0; k < c.state.dnd.size(); ++k) {
                if (c.data.line_x[k].size() != c.data.line_y[k].size()) {
                    IM_ASSERT(c.data.line_x[k].size() == c.data.line_y[k].size() &&
                              "StylizedMetricsPlot: line_x and line_y size mismatch");
                    skip[k] = true;
                }
            }

            for (size_t k = 0; k < c.state.dnd.size(); ++k) {
                if (skip[k]) continue;
                auto& it = c.state.dnd[k];
                if (!it.is_plot) continue;
                ImPlot::SetAxis(ImAxis_Y1);
                ImPlot::SetNextLineStyle(it.color);
                ImPlot::PlotLine(
                        it.label.data(),
                        c.data.line_x[k].data(),
                        c.data.line_y[k].data(),
                        static_cast<int>(c.data.line_y[k].size())
                );
                if (c.cfg.input_enabled && !c.cfg.use_sticky_select && ImPlot::BeginDragDropSourceItem(it.label.data())) {
                    const int payload_index = static_cast<int>(k);
                    ImGui::SetDragDropPayload(c.cfg.dnd_payload, &payload_index, sizeof(payload_index));
                    ImPlot::ItemIcon(it.color);
                    ImGui::SameLine();
                    ImGui::TextUnformatted(it.label.data());
                    ImPlot::EndDragDropSource();
                }
            }

            if (c.cfg.input_enabled && c.cfg.show_tooltip && ImPlot::IsPlotHovered()) {
                const ImPlotPoint mp = ImPlot::GetPlotMousePos();
                const ImVec2 mp_px = ImPlot::PlotToPixels(mp);

                int best_k = -1, best_i = -1;
                float best_d2 = std::numeric_limits<float>::max();
                const float max_d2 = c.cfg.line_tooltip_snap_px * c.cfg.line_tooltip_snap_px;

                for (size_t k = 0; k < c.state.dnd.size(); ++k) {
                    const auto& it = c.state.dnd[k];
                    if (!it.is_plot) continue;
                    if (!is_series_visible(it.label.data())) continue;

                    const auto& X = c.data.line_x[k];
                    const auto& Y = c.data.line_y[k];
                    if (X.empty() || X.size() != Y.size()) continue;

                    auto itx = std::lower_bound(X.begin(), X.end(), mp.x);
                    int idx = (int)std::distance(X.begin(), itx);
                    int cand[3] = {
                        ImClamp(idx - 1, 0, (int)X.size() - 1),
                        ImClamp(idx,      0, (int)X.size() - 1),
                        ImClamp(idx + 1,  0, (int)X.size() - 1)
                    };

                    for (int c_i : cand) {
                        ImVec2 pt_px = ImPlot::PlotToPixels(ImPlotPoint(X[c_i], Y[c_i]));
                        float d2 = (pt_px.x - mp_px.x) * (pt_px.x - mp_px.x) +
                                   (pt_px.y - mp_px.y) * (pt_px.y - mp_px.y);
                        if (d2 < best_d2) {
                            best_d2 = d2;
                            best_k = (int)k;
                            best_i = c_i;
                        }
                    }
                }

                if (best_k >= 0 && best_i >= 0 && best_d2 <= max_d2) {
                    const auto& it = c.state.dnd[best_k];
                    const double x = c.data.line_x[best_k][best_i];
                    const double y = c.data.line_y[best_k][best_i];

                    char vbuf[64];
                    ImFormatString(vbuf, IM_ARRAYSIZE(vbuf), c.cfg.value_fmt, y);

                    if (ImGui::BeginTooltip()) {
                        ImPlot::ItemIcon(it.color);
                        ImGui::SameLine();
                        ImGui::TextUnformatted(it.label.data());
                        ImGui::Separator();
                        ImGui::Text("x: %.6g", x);
                        ImGui::Text("%s: %s", c.cfg.y_label ? c.cfg.y_label : "Value", vbuf);
                        ImGui::EndTooltip();
                    }
                }
            }
        }

        inline void draw_bars(Ctx& c) {
            // Plot bar series with annotation and tooltip.
            ImPlotAxisFlags y_flags = c.cfg.y_axis_right ? ImPlotAxisFlags_Opposite : ImPlotAxisFlags_None;
            ImPlotAxisFlags x_flags = c.data.tick_labels_x.empty() ? ImPlotAxisFlags_NoTickLabels : ImPlotAxisFlags_None;

            ImPlot::SetupAxis(ImAxis_Y1, c.cfg.y_label, y_flags);
            ImPlot::SetupAxis(ImAxis_X1, c.cfg.x_label, x_flags);
            ImPlot::SetupAxisFormat(ImAxis_Y1, c.cfg.value_fmt);
            ImPlot::SetupAxisFormat(ImAxis_X1, "");
            
            const double bar_w = 0.67;

            if (!c.data.tick_labels_x.empty()) {
                std::vector<double> xs; 
                xs.reserve(c.state.dnd.size());
                std::vector<const char*> lbls; lbls.reserve(c.state.dnd.size());
                for (size_t k = 0, i = 0; k < c.state.dnd.size(); ++k) {
                    const auto& it = c.state.dnd[k];
                    if (!it.is_plot) continue;
                    xs.push_back((double)i);
                    lbls.push_back((it.index < c.data.tick_labels_x.size()) ? 
                        c.data.tick_labels_x[it.index].c_str() : 
                        it.label.data()); // fallback
                    ++i;
                }
                if (!xs.empty()) {
                    ImPlot::SetupAxisTicks(ImAxis_X1, xs.data(), (int)xs.size(), lbls.data(), /*keep_auto=*/false);
                }
                const int m = (int)xs.size();
                ImPlot::SetupAxisLimits(ImAxis_X1, -bar_w, (m - 1.0) + bar_w, ImGuiCond_Always);
            }

            std::vector<char*> labels_ptr(c.state.dnd.size());
            for (size_t k = 0; k < c.state.dnd.size(); ++k) {
                labels_ptr[k] = const_cast<char*>(c.state.dnd[k].label.data());
            }

            for (size_t k = 0, i = 0; k < c.state.dnd.size(); ++k) {
                auto& it = c.state.dnd[k];
                if (!it.is_plot) continue;
                double pos = static_cast<double>(i);
                ImPlot::SetAxis(ImAxis_Y1);
                ImPlot::SetNextFillStyle(it.color);
                ImPlot::PlotBars(it.label.data(), &pos, &c.data.values[k], 1, bar_w);
                if (c.state.show_annotation && is_series_visible(it.label.data())) {
                    ImVec4 col = it.color; col.w = 0.5f;
                    const ImVec2 off = (c.data.values[k] >= 0) ? ImVec2(0, -5) : ImVec2(0, 5);
                    ImPlot::Annotation(
                            pos, c.data.values[k], col, off, false,
                            c.cfg.label_value_fmt, labels_ptr[k], c.data.values[k]
                    );
                }

                if (c.cfg.input_enabled && !c.cfg.use_sticky_select && ImPlot::BeginDragDropSourceItem(it.label.data())) {
                    const int payload_index = static_cast<int>(k);
                    ImGui::SetDragDropPayload(c.cfg.dnd_payload, &payload_index, sizeof(payload_index));
                    ImPlot::ItemIcon(it.color);
                    ImGui::SameLine();
                    ImGui::TextUnformatted(it.label.data());
                    ImPlot::EndDragDropSource();
                }
                ++i;
            }

            if (c.cfg.input_enabled && c.cfg.show_tooltip && ImPlot::IsPlotHovered()) {
                ImPlotPoint mp = ImPlot::GetPlotMousePos();
                int best_k = -1;
                double best_dx = 1e9;
                const double half_snap = 0.5 * bar_w * c.cfg.bar_tooltip_snap_factor;

                for (size_t k = 0, i = 0; k < c.state.dnd.size(); ++k) {
                    const auto& it = c.state.dnd[k];
                    if (!it.is_plot) continue;

                    const double pos = static_cast<double>(i);
                    if (!is_series_visible(it.label.data())) {
                        ++i;
                        continue;
                    }

                    const double dx = std::abs(mp.x - pos);
                    if (dx <= half_snap && dx < best_dx) {
                        best_dx = dx;
                        best_k = (int)k;
                    }
                    ++i;
                }

                if (best_k >= 0) {
                    const auto& it = c.state.dnd[best_k];
                    char vbuf[64];
                    ImFormatString(vbuf, IM_ARRAYSIZE(vbuf), c.cfg.value_fmt, c.data.values[best_k]);

                    if (ImGui::BeginTooltip()) {
                        ImPlot::ItemIcon(it.color);
                        ImGui::SameLine();
                        ImGui::TextUnformatted(it.label.data());
                        ImGui::Separator();
                        ImGui::Text("%s: %s", c.cfg.y_label ? c.cfg.y_label : "Value", vbuf);
                        ImGui::EndTooltip();
                    }
                }
            }
        }

        inline void draw_context_popup(Ctx& c) {
            // Context menu for plot settings.
            if (c.cfg.input_enabled && ImPlot::IsPlotHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                ImGui::OpenPopup("##custom_menu");
            }

            if (ImGui::BeginPopup("##custom_menu")) {
                ImGui::TextUnformatted(c.cfg.settings_header);
                ImGui::Separator();
                draw_controls(c);
                ImGui::EndPopup();
            }
        }

        inline void draw_annotation(Ctx& c) {
            if (!c.data.values.empty()) {
                same_line_right_aligned_checkbox(c.cfg.annotation_checkbox);
                if (ImGui::Checkbox(c.cfg.annotation_checkbox, &c.state.show_annotation)) {
                    c.state.update_counter = kUpdateCounterMax;
                }
            }
        }

        inline void draw_plot(Ctx& c) {
            // Plot area with drag-drop, crosshair and context popup.
            
            const bool left_panel_hidden = c.cfg.force_all_visible;  // или другая ваша логика
            bool want_legend = !c.cfg.legend_force_off && c.state.show_legend;

            int flags = ImPlotFlags_None | ImPlotFlags_NoMenus | ImPlotFlags_NoFrame; // убрала 
            flags |= want_legend ? ImPlotFlags_None : ImPlotFlags_NoLegend;
            if (!c.cfg.input_enabled) {
                flags |= ImPlotFlags_NoInputs;
            }

            if (c.state.update_counter > 0) {
                if (c.state.update_counter == kUpdateCounterMax || c.state.update_counter == 1) {
                    ImPlot::SetNextAxisToFit(ImAxis_X1);
                    ImPlot::SetNextAxisToFit(ImAxis_Y1);
                }
                --c.state.update_counter;
            }

            if (ImGui::BeginChild(
                    "##plot_layout",
                    ImVec2(-1.0f, -1.0f),
                    ImGuiChildFlags_AlwaysUseWindowPadding,
                    ImGuiWindowFlags_None)){
                ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, c.cfg.fit_padding); // FitPadding
                if (ImPlot::BeginPlot(c.cfg.id, ImVec2(-1, -1), flags)) {
                    //ImPlot::SetupLegend(ImPlotLocation_West);
                    {
                        const ImPlotLocation    loc  = left_panel_hidden ? c.cfg.legend_loc_compact  : c.cfg.legend_loc;
                        const ImPlotLegendFlags lflg = left_panel_hidden ? c.cfg.legend_flags_compact: c.cfg.legend_flags;
                        ImPlot::SetupLegend(loc, lflg);
                    }

                    if (c.data.values.empty()) {
                        draw_lines(c);
                    } else {
                        draw_bars(c);
                    }

                    if (c.cfg.input_enabled && !c.cfg.use_sticky_select && ImPlot::BeginDragDropTargetPlot()) {
                        if (const ImGuiPayload* payload =
                                ImGui::AcceptDragDropPayload(c.cfg.dnd_payload)) {
                            int i = *static_cast<const int*>(payload->Data);
                            if (i >= 0 && static_cast<size_t>(i) < c.state.dnd.size()) {
                                const bool was_plot = c.state.dnd[i].is_plot;
                                if (!was_plot) {
                                    int cnt = 0;
                                    for (const auto& d : c.state.dnd) {
                                        if (d.is_plot) ++cnt;
                                    }
                                    ++cnt;
                                    if (c.state.show_legend && cnt > c.cfg.max_visible) {
                                        c.state.show_legend = false;
                                    }
                                }

                                c.state.dnd[i].is_plot = true;
                                c.state.update_counter = kUpdateCounterMax;
                            }
                        }
                        ImPlot::EndDragDropTarget();
                    }

                    if (c.cfg.input_enabled && !c.cfg.use_sticky_select && ImPlot::BeginDragDropTargetLegend()) {
                        if (const ImGuiPayload* payload =
                                ImGui::AcceptDragDropPayload(c.cfg.dnd_payload)) {
                            int i = *static_cast<const int*>(payload->Data);
                            if (i >= 0 && static_cast<size_t>(i) < c.state.dnd.size()) {
                                const bool was_plot = c.state.dnd[i].is_plot;
                                if (!was_plot) {
                                    int cnt = 0;
                                    for (const auto& d : c.state.dnd) {
                                        if (d.is_plot) ++cnt;
                                    }
                                    ++cnt;
                                    if (c.state.show_legend && cnt > c.cfg.max_visible) {
                                        c.state.show_legend = false;
                                    }
                                }

                                c.state.dnd[i].is_plot = true;
                                c.state.update_counter = kUpdateCounterMax;
                            }
                        }
                        ImPlot::EndDragDropTarget();
                    }

                    if (c.cfg.input_enabled && ImPlot::IsPlotHovered()) {
                        ImPlotPoint mouse_pos = ImPlot::GetPlotMousePos();
                        static ImPlotDragToolFlags tool_flags =
                            ImPlotDragToolFlags_None | ImPlotDragToolFlags_NoInputs;
                        ImPlot::DragLineX(1, &mouse_pos.x, c.cfg.drag_line_color, 1, tool_flags);
                        ImPlot::DragLineY(2, &mouse_pos.y, c.cfg.drag_line_color, 1, tool_flags);
                    }

                    if (c.cfg.input_enabled) {
                        draw_context_popup(c);
                    }

                    ImPlot::EndPlot();
                }
                ImPlot::PopStyleVar(); // FitPadding

            }
            ImGui::EndChild();

           
        }

    } // namespace stylized_metrics_plot_detail

    IMGUIX_IMPL_INLINE void StylizedMetricsPlot(
            const MetricsPlotData& data,
            StylizedMetricsPlotState& state,
            const StylizedMetricsPlotConfig& cfg
        ) {
        ImGui::PushID(cfg.id);

        stylized_metrics_plot_detail::Ctx c{data, state, cfg};
        c.dnd_w = cfg.dnd_width > 0.0f ? cfg.dnd_width : CalcMetricsDndWidth();
        c.btn_w = CalcMetricsDndButtonWidth(c.dnd_w);
        c.plot_h = IM_TRUNC(stylized_metrics_plot_detail::calc_plot_height(cfg));

        stylized_metrics_plot_detail::sync_dnd(c);
        
        if (!c.state.initialized) {
            auto mark_plot = [&](int idx) {
                if (idx >= 0 && idx < (int)state.dnd.size())
                    state.dnd[idx].is_plot = true;
            };
            
            if (!c.cfg.default_show_indices.empty()) {
                for (int idx : c.cfg.default_show_indices) mark_plot(idx);
                c.state.update_counter = stylized_metrics_plot_detail::kUpdateCounterMax;
            } else 
            if (c.cfg.default_show_index >= 0) {
                mark_plot(c.cfg.default_show_index);
                c.state.update_counter = stylized_metrics_plot_detail::kUpdateCounterMax;
            }
            
            int cnt = 0; 
            for (auto& it : c.state.dnd) if (it.is_plot) ++cnt;
            if (c.state.show_legend && cnt > c.cfg.max_visible) {
                c.state.show_legend = false;
            }

            c.state.show_annotation = c.cfg.annotations_default_on;
            c.state.initialized     = true;
        }

        ImGui::BeginGroup();
        if (!c.cfg.force_all_visible) {
            if (c.state.show_left_panel){
                stylized_metrics_plot_detail::draw_left_panel(c);
                if (cfg.panels_spacing > 0.0f) {
                    ImGui::SameLine(0.0f, cfg.panels_spacing);
                } else {
                    ImGui::SameLine();
                }
            }
        } else {
            c.plotted = 0;
            for (const auto& it : c.state.dnd) {
                if (it.is_plot) ++c.plotted;
            }
        }
        stylized_metrics_plot_detail::begin_right_child(c);
        if (c.cfg.show_plot_header) {
            ImGui::Text(c.cfg.plot_header);
            if (!c.cfg.force_all_visible) {
                ImGui::SameLine();
                std::string show_side_panel_bttn_name = "show_side_panel_bttn";
                std::string show_side_panel_bttn_icon = c.state.show_left_panel ? u8"\uf053" : u8"\uf054";
                if (!c.cfg.input_enabled) ImGui::BeginDisabled();
                if (ImGuiX::Widgets::IconButtonCentered(show_side_panel_bttn_name.c_str(), show_side_panel_bttn_icon.c_str(), c.cfg.show_side_panel_cfg)) {
                    c.state.show_left_panel = !c.state.show_left_panel;
                }
                if (!c.cfg.input_enabled) ImGui::EndDisabled();
            }
            if (c.cfg.force_all_visible && c.cfg.show_annotation_checkbox) {
                stylized_metrics_plot_detail::draw_annotation(c);
            }
        }
        stylized_metrics_plot_detail::draw_plot(c);
        stylized_metrics_plot_detail::end_right_child();
        ImGui::EndGroup();

        ImGui::PopID();
    }

} // namespace ImGuiX::Widgets
