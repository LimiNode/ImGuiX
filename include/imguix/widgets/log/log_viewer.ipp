#include <imguix/config/build.hpp>

#include <imguix/extensions/scoped_style.hpp>

#include <algorithm>
#include <iterator>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ImGuiX::Widgets {

    namespace {

        const char* non_null_label(const char* value, const char* fallback) {
            return value != nullptr ? value : fallback;
        }

        std::string icon_label(const char* icon, const char* text, const char* suffix) {
            std::string value = icon != nullptr ? icon : "";
            if (text != nullptr && *text != '\0') {
                if (!value.empty()) {
                    value.push_back(' ');
                }
                value += text;
            }
            value += suffix;
            return value;
        }

        std::string default_clipboard_row(const LogViewerEntry& entry) {
            return entry.timestamp + "\t" + entry.level + "\t" + entry.message;
        }

    }  // namespace

    IMGUIX_IMPL_INLINE bool LogViewer(
        const char* id,
        const LogViewerEntry* entries,
        const std::size_t entry_count,
        LogViewerState& state,
        const LogViewerConfig& config) {
        IM_ASSERT(
            entries != nullptr ||
            entry_count == 0U);
        if (entries == nullptr && entry_count != 0U) {
            return false;
        }

        const char* widget_id = id != nullptr ? id : "##log_viewer";
        ImGui::PushID(widget_id);

        const auto entry_at = [entries](const std::size_t index)
            -> const LogViewerEntry& { return entries[index]; };

        const auto prune_state = [&]() {
            std::unordered_set<std::uint64_t> all_ids;
            std::unordered_set<std::uint64_t> visible_ids;
            all_ids.reserve(entry_count);
            visible_ids.reserve(entry_count);
            for (std::size_t index = 0; index < entry_count; ++index) {
                const LogViewerEntry& entry = entry_at(index);
                const auto insertion = all_ids.insert(entry.id);
                IM_ASSERT(
                    insertion.second &&
                    "LogViewerEntry::id must be unique within a LogViewer data set");
                if (entry.level_rank < state.min_level_rank) {
                    continue;
                }
                visible_ids.insert(entry.id);
            }

            for (auto selected = state.selected_ids.begin();
                 selected != state.selected_ids.end();) {
                if (visible_ids.count(*selected) == 0U) {
                    selected = state.selected_ids.erase(selected);
                } else {
                    ++selected;
                }
            }
            if (state.selection_anchor_id.has_value() &&
                visible_ids.count(*state.selection_anchor_id) == 0U) {
                state.selection_anchor_id.reset();
            }
        };

        prune_state();

        bool viewer_focused = false;
        const bool panel_open = BeginRoundedPanel("##panel", config.size, config.panel);
        if (panel_open) {
            {
                const ImGuiStyle& style = ImGui::GetStyle();
                const float toolbar_padding_y =
                    config.toolbar_padding_y >= 0.0f ? config.toolbar_padding_y
                                                     : style.WindowPadding.y;
                const ImGuiX::Extensions::ScopedStyleVar toolbar_padding(
                    ImGuiStyleVar_WindowPadding,
                    ImVec2(config.toolbar_padding_x, toolbar_padding_y));
                const bool toolbar_open = ImGui::BeginChild(
                    "##toolbar", ImVec2(0.0f, 0.0f),
                    ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding);
                if (toolbar_open) {
                    ImGui::SeparatorText(non_null_label(config.labels.title, "Logs"));

                    ImGui::BeginGroup();
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextUnformatted(
                        non_null_label(config.labels.minimum_level, "Minimum level"));
                    ImGui::SameLine();

                    const char* current_level = "-";
                    if (config.levels != nullptr) {
                        for (std::size_t index = 0; index < config.level_count; ++index) {
                            if (config.levels[index].rank == state.min_level_rank) {
                                current_level =
                                    non_null_label(config.levels[index].label, "-");
                                break;
                            }
                        }
                    }
                    ImGui::SetNextItemWidth(config.level_combo_width);
                    if (config.levels != nullptr && config.level_count > 0U &&
                        ImGui::BeginCombo("##level", current_level)) {
                        for (std::size_t index = 0; index < config.level_count; ++index) {
                            const LogViewerLevel& level = config.levels[index];
                            const bool selected = level.rank == state.min_level_rank;
                            if (ImGui::Selectable(
                                    non_null_label(level.label, "-"), selected)) {
                                state.min_level_rank = level.rank;
                                prune_state();
                            }
                            if (selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    } else if (config.levels == nullptr || config.level_count == 0U) {
                        ImGui::TextUnformatted(current_level);
                    }
                    ImGui::EndGroup();

                    struct ActionButton final {
                        std::string label;
                        const char* tooltip{nullptr};
                        bool enabled{false};
                        bool copy_all{false};
                        std::function<void()> callback;
                    };
                    std::vector<ActionButton> action_buttons;
                    action_buttons.reserve(4);
                    const auto add_action = [&action_buttons](
                                                const bool visible,
                                                std::string label,
                                                const char* tooltip,
                                                const bool enabled,
                                                const bool copy_all,
                                                std::function<void()> callback) {
                        if (visible) {
                            action_buttons.push_back(
                                {std::move(label), tooltip, enabled, copy_all, std::move(callback)});
                        }
                    };

                    add_action(
                        config.show_refresh,
                        icon_label(config.icons.refresh, nullptr, "##refresh"),
                        config.labels.refresh_tooltip,
                        config.refresh_enabled && static_cast<bool>(config.actions.on_refresh),
                        false,
                        config.actions.on_refresh);
                    add_action(
                        config.show_copy_all,
                        icon_label(config.icons.copy, config.labels.copy_all, "##copy_all"),
                        nullptr,
                        true,
                        true,
                        nullptr);
                    add_action(
                        config.show_clear,
                        icon_label(config.icons.clear, nullptr, "##clear"),
                        config.labels.clear_tooltip,
                        config.clear_enabled && static_cast<bool>(config.actions.on_clear),
                        false,
                        config.actions.on_clear);
                    add_action(
                        config.show_open_folder,
                        icon_label(config.icons.open_folder, nullptr, "##open_folder"),
                        config.labels.open_folder_tooltip,
                        config.open_folder_enabled &&
                            static_cast<bool>(config.actions.on_open_folder),
                        false,
                        config.actions.on_open_folder);

                    const ImGuiStyle& toolbar_style = ImGui::GetStyle();
                    const auto button_width = [&toolbar_style](const std::string& label) {
                        return ImGui::CalcTextSize(label.c_str(), nullptr, true).x +
                            toolbar_style.FramePadding.x * 2.0f;
                    };
                    float action_buttons_width = 0.0f;
                    for (const ActionButton& action : action_buttons) {
                        action_buttons_width += button_width(action.label);
                    }
                    if (action_buttons.size() > 1U) {
                        action_buttons_width += toolbar_style.ItemSpacing.x *
                            static_cast<float>(action_buttons.size() - 1U);
                    }
                    const float content_right =
                        ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
                    const float filter_group_right = ImGui::GetItemRectMax().x;
                    const float same_line_available = std::max(
                        0.0f,
                        content_right - filter_group_right - toolbar_style.ItemSpacing.x);
                    if (!action_buttons.empty() &&
                        same_line_available >= action_buttons_width) {
                        ImGui::SameLine();
                    }

                    bool first_action = true;
                    for (const ActionButton& action : action_buttons) {
                        if (!first_action) {
                            const float next_button_width =
                                button_width(action.label);

                            const float current_line_available =
                                std::max(
                                    0.0f,
                                    content_right -
                                        ImGui::GetItemRectMax().x -
                                        toolbar_style.ItemSpacing.x);

                            if (current_line_available >= next_button_width) {
                                ImGui::SameLine();
                            }
                        }
                        first_action = false;

                        ImGui::BeginDisabled(!action.enabled);
                        if (ImGui::Button(action.label.c_str())) {
                            if (action.copy_all) {
                                std::string output;
                                for (std::size_t index = 0; index < entry_count; ++index) {
                                    const LogViewerEntry& entry = entry_at(index);
                                    if (entry.level_rank < state.min_level_rank) {
                                        continue;
                                    }
                                    if (!output.empty()) {
                                        output.push_back('\n');
                                    }
                                    output += config.format_clipboard_row
                                                  ? config.format_clipboard_row(entry)
                                                  : default_clipboard_row(entry);
                                }
                                if (!output.empty()) {
                                    ImGui::SetClipboardText(output.c_str());
                                }
                            } else if (action.callback) {
                                action.callback();
                            }
                        }
                        ImGui::EndDisabled();
                        if (action.tooltip != nullptr &&
                            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                            ImGui::SetTooltip("%s", action.tooltip);
                        }
                    }

                    if (!state.selected_ids.empty()) {
                        ImGui::AlignTextToFramePadding();
                        std::string selected_label;
                        if (config.format_selected_count) {
                            selected_label = config.format_selected_count(state.selected_ids.size());
                        } else {
                            selected_label = std::to_string(state.selected_ids.size()) +
                                " selected";
                        }
                        ImGui::TextUnformatted(selected_label.c_str());
                        ImGui::SameLine();

                        const std::string copy_selected_label = icon_label(
                            config.icons.copy, config.labels.copy_selected, "##copy_selected");
                        if (ImGui::Button(copy_selected_label.c_str())) {
                            std::string output;
                            for (std::size_t index = 0; index < entry_count; ++index) {
                                const LogViewerEntry& entry = entry_at(index);
                                if (entry.level_rank < state.min_level_rank ||
                                    state.selected_ids.count(entry.id) == 0U) {
                                    continue;
                                }
                                if (!output.empty()) {
                                    output.push_back('\n');
                                }
                                output += config.format_clipboard_row
                                              ? config.format_clipboard_row(entry)
                                              : default_clipboard_row(entry);
                            }
                            if (!output.empty()) {
                                ImGui::SetClipboardText(output.c_str());
                                if (config.clear_selection_after_copy) {
                                    state.selected_ids.clear();
                                    state.selection_anchor_id.reset();
                                }
                            }
                        }
                        ImGui::SameLine();
                        const std::string deselect_label = icon_label(
                            config.icons.deselect, config.labels.deselect, "##deselect");
                        if (ImGui::Button(deselect_label.c_str())) {
                            state.selected_ids.clear();
                            state.selection_anchor_id.reset();
                        }
                    }
                }
                ImGui::EndChild();
            }

            std::vector<std::size_t> visible_indices;
            visible_indices.reserve(entry_count);
            for (std::size_t index = 0; index < entry_count; ++index) {
                if (entry_at(index).level_rank >= state.min_level_rank) {
                    visible_indices.push_back(index);
                }
            }

            const ImGuiStyle& panel_style = ImGui::GetStyle();
            const float table_edge_inset = std::max(1.0f, panel_style.ChildBorderSize);
            const ImVec2 table_available = ImGui::GetContentRegionAvail();
            const ImVec2 table_size(
                std::max(0.0f, table_available.x - table_edge_inset * 2.0f),
                std::max(0.0f, table_available.y - table_edge_inset));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + table_edge_inset);

            const ImGuiX::Extensions::ScopedStyleVar table_child_rounding(
                ImGuiStyleVar_ChildRounding, 0.0f);
            if (ImGui::BeginTable(
                    "##table", 3,
                    ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
                        ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_ScrollY |
                        ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp |
                        ImGuiTableFlags_PadOuterX,
                    table_size)) {
                const ImGuiStyle& style = ImGui::GetStyle();
                const char* timestamp_header = non_null_label(config.labels.timestamp, "Timestamp");
                const char* level_header = non_null_label(config.labels.level, "Level");
                float timestamp_content_width =
                    ImGui::CalcTextSize(timestamp_header).x;
                float level_content_width =
                    ImGui::CalcTextSize(level_header).x;
                for (const std::size_t index : visible_indices) {
                    const LogViewerEntry& entry = entry_at(index);
                    timestamp_content_width = std::max(
                        timestamp_content_width,
                        ImGui::CalcTextSize(entry.timestamp.c_str()).x);
                    level_content_width = std::max(
                        level_content_width,
                        ImGui::CalcTextSize(entry.level.c_str()).x);
                }
                const float timestamp_width =
                    config.timestamp_column_width > 0.0f
                        ? config.timestamp_column_width
                        : timestamp_content_width + style.CellPadding.x * 2.0f;
                const float level_width =
                    config.level_column_width > 0.0f
                        ? config.level_column_width
                        : level_content_width + style.CellPadding.x * 2.0f;
                ImGui::TableSetupColumn(
                    timestamp_header, ImGuiTableColumnFlags_WidthFixed, timestamp_width,
                    ImGui::GetID("##timestamp_column"));
                ImGui::TableSetupColumn(
                    level_header, ImGuiTableColumnFlags_WidthFixed, level_width,
                    ImGui::GetID("##level_column"));
                ImGui::TableSetupColumn(
                    non_null_label(config.labels.message, "Message"), ImGuiTableColumnFlags_None,
                    0.0f, ImGui::GetID("##message_column"));
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();
                ImGui::TableSetColumnIndex(2);
                const float message_width = std::max(1.0f, ImGui::GetContentRegionAvail().x);
                ImGui::TableSetColumnIndex(0);

                const ImGuiIO& io = ImGui::GetIO();
                const ImVec2 table_window_pos = ImGui::GetWindowPos();
                const ImVec2 table_window_size = ImGui::GetWindowSize();
                const float header_bottom = ImGui::GetItemRectMax().y;
                const bool mouse_in_table_body =
                    io.MousePos.x >= table_window_pos.x &&
                    io.MousePos.x < table_window_pos.x + table_window_size.x &&
                    io.MousePos.y >= header_bottom &&
                    io.MousePos.y < table_window_pos.y + table_window_size.y;
                if (io.KeyShift && io.MouseWheelRequestAxisSwap && io.MouseWheel != 0.0f &&
                    ImGui::IsWindowHovered() && mouse_in_table_body) {
                    ImGui::SetScrollY(ImGui::GetScrollY() -
                                      io.MouseWheel * ImGui::GetTextLineHeightWithSpacing() * 3.0f);
                }

                const auto select_row = [&](const std::size_t visible_index) {
                    if (visible_index >= visible_indices.size()) {
                        return;
                    }
                    const std::size_t index = visible_indices[visible_index];
                    const std::uint64_t id_value = entry_at(index).id;
                    if (io.KeyShift && state.selection_anchor_id.has_value()) {
                        const auto anchor = std::find_if(
                            visible_indices.begin(), visible_indices.end(),
                            [entries, anchor_id = *state.selection_anchor_id](
                                const std::size_t candidate) {
                                return entries[candidate].id == anchor_id;
                            });
                        if (anchor == visible_indices.end()) {
                            state.selected_ids.clear();
                            state.selected_ids.insert(id_value);
                            state.selection_anchor_id = id_value;
                            return;
                        }
                        const std::size_t anchor_index = static_cast<std::size_t>(
                            std::distance(visible_indices.begin(), anchor));
                        const std::size_t begin = std::min(anchor_index, visible_index);
                        const std::size_t end = std::max(anchor_index, visible_index);
                        if (!io.KeyCtrl) {
                            state.selected_ids.clear();
                        }
                        for (std::size_t range_index = begin; range_index <= end; ++range_index) {
                            state.selected_ids.insert(
                                entry_at(visible_indices[range_index]).id);
                        }
                    } else if (io.KeyCtrl) {
                        if (!state.selected_ids.erase(id_value)) {
                            state.selected_ids.insert(id_value);
                        }
                        state.selection_anchor_id = id_value;
                    } else {
                        state.selected_ids.clear();
                        state.selected_ids.insert(id_value);
                        state.selection_anchor_id = id_value;
                    }
                };

                {
                    const ImGuiX::Extensions::ScopedStyleVar data_cell_padding(
                        ImGuiStyleVar_CellPadding, ImVec2(style.CellPadding.x, 0.0f));
                    for (std::size_t visible_index = 0;
                         visible_index < visible_indices.size(); ++visible_index) {
                        const std::size_t index = visible_indices[visible_index];
                        const LogViewerEntry& entry = entry_at(index);
                        const bool selected = state.selected_ids.count(entry.id) != 0U;
                        const float message_height =
                            std::max(ImGui::GetTextLineHeight(),
                                     ImGui::CalcTextSize(entry.message.c_str(), nullptr, false,
                                                         message_width)
                                         .y);
                        ImGui::TableNextRow(ImGuiTableRowFlags_None, message_height);
                        ImGui::TableNextColumn();

                        bool row_pressed = false;
                        bool row_hovered = false;
                        bool row_active = false;
                        {
                            const ImGuiX::Extensions::ScopedStyleVar selectable_spacing(
                                ImGuiStyleVar_ItemSpacing,
                                ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));
                            const ImVec4 transparent(0.0f, 0.0f, 0.0f, 0.0f);
                            const ImGuiX::Extensions::ScopedStyleColor selectable_color(
                                ImGuiCol_Header, transparent);
                            const ImGuiX::Extensions::ScopedStyleColor selectable_hovered(
                                ImGuiCol_HeaderHovered, transparent);
                            const ImGuiX::Extensions::ScopedStyleColor selectable_active(
                                ImGuiCol_HeaderActive, transparent);
                            const std::string row_id = "##row_" + std::to_string(entry.id);
                            row_pressed = ImGui::Selectable(
                                row_id.c_str(), false,
                                ImGuiSelectableFlags_SpanAllColumns |
                                    ImGuiSelectableFlags_AllowOverlap,
                                ImVec2(0.0f, message_height));
                            row_hovered = ImGui::IsItemHovered();
                            row_active = ImGui::IsItemActive();
                        }
                        if (row_active) {
                            ImGui::TableSetBgColor(
                                ImGuiTableBgTarget_RowBg1,
                                ImGui::GetColorU32(ImGuiCol_HeaderActive));
                        } else if (row_hovered) {
                            ImGui::TableSetBgColor(
                                ImGuiTableBgTarget_RowBg1,
                                ImGui::GetColorU32(ImGuiCol_HeaderHovered));
                        } else if (selected) {
                            ImGui::TableSetBgColor(
                                ImGuiTableBgTarget_RowBg1,
                                ImGui::GetColorU32(ImGuiCol_NavHighlight));
                        }
                        if (row_pressed) {
                            select_row(visible_index);
                        }
                        if (ImGui::BeginPopupContextItem()) {
                            if (ImGui::MenuItem(
                                    non_null_label(config.labels.copy_row, "Copy row"))) {
                                ImGui::SetClipboardText(
                                    (config.format_clipboard_row
                                         ? config.format_clipboard_row(entry)
                                         : default_clipboard_row(entry))
                                        .c_str());
                            }
                            if (ImGui::MenuItem(non_null_label(
                                    config.labels.copy_timestamp, "Copy timestamp"))) {
                                ImGui::SetClipboardText(entry.timestamp.c_str());
                            }
                            if (ImGui::MenuItem(
                                    non_null_label(config.labels.copy_level, "Copy level"))) {
                                ImGui::SetClipboardText(entry.level.c_str());
                            }
                            if (ImGui::MenuItem(non_null_label(
                                    config.labels.copy_message, "Copy message"))) {
                                ImGui::SetClipboardText(entry.message.c_str());
                            }
                            ImGui::EndPopup();
                        }
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(entry.level.c_str());
                        ImGui::TableNextColumn();
                        ImGui::TextWrapped("%s", entry.message.c_str());
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(entry.timestamp.c_str());
                    }
                }
                ImGui::EndTable();
            }

            viewer_focused =
                ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
        }
        EndRoundedPanel();

        if (viewer_focused &&
            !state.selected_ids.empty() &&
            ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_C)) {
            std::string output;
            for (std::size_t index = 0; index < entry_count; ++index) {
                const LogViewerEntry& entry = entry_at(index);
                if (entry.level_rank < state.min_level_rank ||
                    state.selected_ids.count(entry.id) == 0U) {
                    continue;
                }
                if (!output.empty()) {
                    output.push_back('\n');
                }
                output += config.format_clipboard_row
                              ? config.format_clipboard_row(entry)
                              : default_clipboard_row(entry);
            }
            if (!output.empty()) {
                ImGui::SetClipboardText(output.c_str());
                if (config.clear_selection_after_copy) {
                    state.selected_ids.clear();
                    state.selection_anchor_id.reset();
                }
            }
        }

        ImGui::PopID();
        return panel_open;
    }

}  // namespace ImGuiX::Widgets
