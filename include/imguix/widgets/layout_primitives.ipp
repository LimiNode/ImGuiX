#include <imguix/config/build.hpp>
#include <imguix/widgets/controls/icon_button.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE bool PageHeader(const PageHeaderConfig& config) {
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float available = std::max(1.0f, ImGui::GetContentRegionAvail().x);
        const float gap = std::max(0.0f, config.gap);
        const float frame_height = ImGui::GetFrameHeight();
        const float leading = config.back_action ? frame_height + gap : 0.0f;
        const float status_width = config.status ? std::max(0.0f, config.status_width) : 0.0f;
        const float actions_width = config.actions ? std::max(0.0f, config.actions_width) : 0.0f;
        const float trailing = status_width + actions_width +
                               (status_width > 0.0f && actions_width > 0.0f ? gap : 0.0f);
        const char* title = config.title.empty() ? "" : config.title.data();
        if (config.title_font != nullptr) {
            ImGui::PushFont(config.title_font);
        }
        const float natural_width = ImGui::CalcTextSize(title, title + config.title.size()).x;
        const bool inline_trailing =
            trailing > 0.0f && leading + natural_width + gap + trailing <= available;
        const float title_width =
            std::max(1.0f, available - leading - (inline_trailing ? trailing + gap : 0.0f));
        const ImVec2 title_size =
            ImGui::CalcTextSize(title, title + config.title.size(), false, title_width);
        const float row_height =
            std::max(title_size.y, config.back_action || inline_trailing ? frame_height : 0.0f);
        if (config.title_font != nullptr) {
            ImGui::PopFont();
        }

        bool is_back_clicked = false;
        ImGui::BeginGroup();
        // Center the native font and back button geometrically, not using
        // per-page pixel offsets. Wrapped titles keep the same leading slot.
        if (config.back_action) {
            ImGui::SetCursorScreenPos(
                ImVec2(origin.x, origin.y + (row_height - frame_height) * 0.5f));
            IconButtonConfig button;
            button.size = ImVec2(frame_height, frame_height);
            button.appear_on_hover = true;
            is_back_clicked = IconButtonCentered(config.back_id, config.back_icon, button);
            if (ImGui::IsItemHovered() && config.back_tooltip != nullptr) {
                ImGui::SetTooltip("%s", config.back_tooltip);
            }
            if (is_back_clicked) {
                config.back_action();
            }
            ImGui::SameLine(0.0f, gap);
        }
        ImGui::SetCursorScreenPos(
            ImVec2(origin.x + leading, origin.y + (row_height - title_size.y) * 0.5f));
        if (config.title_font != nullptr) {
            ImGui::PushFont(config.title_font);
        }
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + title_width);
        ImGui::TextUnformatted(title, title + config.title.size());
        ImGui::PopTextWrapPos();
        if (config.title_font != nullptr) {
            ImGui::PopFont();
        }
        float trailing_x = origin.x + available - trailing;
        const auto draw_slot = [&](const std::function<void()>& callback, const float width) {
            if (!callback) {
                return;
            }
            if (inline_trailing && width > 0.0f) {
                ImGui::SameLine(0.0f, gap);
                ImGui::SetCursorScreenPos(
                    ImVec2(trailing_x, origin.y + (row_height - frame_height) * 0.5f));
                trailing_x += width + gap;
            } else {
                // A slot without a declared width cannot be safely overlaid
                // beside a wrapped title. Start it at the next item row and
                // let the callback own its normal ImGui item spacing.
                ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMin().x);
            }
            callback();
        };
        draw_slot(config.status, status_width);
        draw_slot(config.actions, actions_width);
        ImGui::EndGroup();
        return is_back_clicked;
    }

    namespace {

        struct ResponsiveColumnsState {
            int count{0};
            int next_index{0};
            bool is_horizontal{false};
            bool is_column_open{false};
        };

        std::vector<ResponsiveColumnsState>& responsiveColumnsStack() {
            static thread_local std::vector<ResponsiveColumnsState> stack;
            return stack;
        }

    }  // namespace

    IMGUIX_IMPL_INLINE bool BeginResponsiveColumns(const char* id, const int count,
                                                   const ResponsiveColumnsConfig& config) {
        IM_ASSERT(count > 0 && count <= 32);
        if (count <= 0 || count > 32) {
            return false;
        }
        const float gap = std::max(0.0f, config.gap);
        const float available = std::max(0.0f, ImGui::GetContentRegionAvail().x);
        const float content_width = std::max(0.0f, available - gap * static_cast<float>(count - 1));
        const bool has_weights = config.column_weights.size() == static_cast<size_t>(count);
        float total_weight = 0.0f;
        for (int index = 0; index < count; ++index) {
            total_weight +=
                has_weights ? std::max(0.001f, config.column_weights[static_cast<size_t>(index)])
                            : 1.0f;
        }
        bool is_horizontal = true;
        for (int index = 0; index < count; ++index) {
            const float weight =
                has_weights ? std::max(0.001f, config.column_weights[static_cast<size_t>(index)])
                            : 1.0f;
            is_horizontal =
                is_horizontal && content_width * weight / total_weight >= config.min_column_width;
        }

        // A non-scrolling table owns column work rectangles. Groups alone do
        // not constrain GetContentRegionAvail(), and PushItemWidth is not a
        // container width. Spacer columns provide exact gaps without leaking
        // CellPadding overrides into caller tables or child windows.
        const ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp |
                                      ImGuiTableFlags_NoSavedSettings |
                                      ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX;
        if (!ImGui::BeginTable(id, is_horizontal ? count * 2 - 1 : 1, flags)) {
            return false;
        }
        for (int index = 0; index < (is_horizontal ? count : 1); ++index) {
            const float weight =
                has_weights ? std::max(0.001f, config.column_weights[static_cast<size_t>(index)])
                            : 1.0f;
            const std::string content_id = "##content" + std::to_string(index);
            ImGui::TableSetupColumn(content_id.c_str(), ImGuiTableColumnFlags_WidthStretch, weight);
            if (is_horizontal && index + 1 < count) {
                const std::string gap_id = "##gap" + std::to_string(index);
                ImGui::TableSetupColumn(gap_id.c_str(), ImGuiTableColumnFlags_WidthFixed, gap);
            }
        }
        responsiveColumnsStack().push_back({count, 0, is_horizontal, false});
        return true;
    }

    IMGUIX_IMPL_INLINE bool BeginResponsiveColumn(const int index) {
        auto& stack = responsiveColumnsStack();
        IM_ASSERT(!stack.empty());
        if (stack.empty()) {
            return false;
        }
        auto& state = stack.back();
        IM_ASSERT(!state.is_column_open && index == state.next_index && index < state.count);
        if (index == 0 || !state.is_horizontal) {
            const ImVec2 padding = ImGui::GetStyle().CellPadding;
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(padding.x, 0.0f));
            ImGui::TableNextRow();
            ImGui::PopStyleVar();
        }
        const bool is_visible = ImGui::TableSetColumnIndex(state.is_horizontal ? index * 2 : 0);
        // Logical index is stable across both table topologies. Identically
        // named inputs in separate columns never share active/storage IDs.
        ImGui::PushID(index);
        state.is_column_open = true;
        ++state.next_index;
        return is_visible;
    }

    IMGUIX_IMPL_INLINE float ResponsiveColumnWidth() {
        return std::max(0.0f, ImGui::GetContentRegionAvail().x);
    }

    IMGUIX_IMPL_INLINE void EndResponsiveColumn() {
        auto& stack = responsiveColumnsStack();
        IM_ASSERT(!stack.empty() && stack.back().is_column_open);
        ImGui::PopID();
        stack.back().is_column_open = false;
    }

    IMGUIX_IMPL_INLINE void EndResponsiveColumns() {
        auto& stack = responsiveColumnsStack();
        IM_ASSERT(!stack.empty() && !stack.back().is_column_open &&
                  stack.back().next_index == stack.back().count);
        ImGui::EndTable();
        stack.pop_back();
    }

}  // namespace ImGuiX::Widgets
