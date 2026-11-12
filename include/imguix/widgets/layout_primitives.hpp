#pragma once
#ifndef _IMGUIX_WIDGETS_LAYOUT_PRIMITIVES_HPP_INCLUDED
#define _IMGUIX_WIDGETS_LAYOUT_PRIMITIVES_HPP_INCLUDED

/// \file layout_primitives.hpp
/// \brief Reusable page header and responsive column composition helpers.

#include <imgui.h>

#include <functional>
#include <string_view>
#include <vector>

namespace ImGuiX::Widgets {

    /// \brief Generic page header configuration.
    struct PageHeaderConfig {
        std::string_view title{};
        const char* back_id{"##page_back"};
        const char* back_icon{u8"\uE5C4"};
        ImFont* title_font{nullptr};
        float gap{8.0f};
        const char* back_tooltip{nullptr};
        std::function<void()> back_action{};
        std::function<void()> status{};
        std::function<void()> actions{};
        float status_width{
            0.0f};  ///< Width of the single-frame-height callback; zero wraps it below.
        float actions_width{
            0.0f};  ///< Width of the single-frame-height callback; zero wraps it below.
    };

    /// \brief Draw one aligned page header row.
    /// \param config Title and optional generic callbacks.
    /// \return True when the back action was clicked.
    /// \note Text is borrowed for this call. Callbacks balance their own scopes.
    ///       Trailing slots are right-aligned when their declared widths fit;
    ///       otherwise they move below the wrapped title. Use PushID for multiple headers.
    bool PageHeader(const PageHeaderConfig& config);

    /// \brief Layout settings for responsive columns.
    struct ResponsiveColumnsConfig {
        float min_column_width{320.0f};
        float gap{16.0f};
        std::vector<float> column_weights{};
    };

    /// \brief Begin a responsive column group.
    /// \param id Stable identifier for the group.
    /// \param count Number of columns to compose.
    /// \param config Minimum width, gap, and optional weights.
    /// \return True when the group was opened; only then call EndResponsiveColumns().
    bool BeginResponsiveColumns(const char* id, int count,
                                const ResponsiveColumnsConfig& config = {});

    /// \brief Begin one column in the current responsive group.
    /// \param index Zero-based column index; submit each column once in ascending order.
    /// \return True when column contents should be submitted. Always call EndResponsiveColumn().
    bool BeginResponsiveColumn(int index);

    /// \brief Return available content width in the active responsive column.
    /// \return Width for contained cards/plots, independent of stacked/horizontal mode.
    float ResponsiveColumnWidth();

    /// \brief Finish the current responsive column.
    void EndResponsiveColumn();

    /// \brief Finish the current responsive column group.
    void EndResponsiveColumns();

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#include "layout_primitives.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_LAYOUT_PRIMITIVES_HPP_INCLUDED
