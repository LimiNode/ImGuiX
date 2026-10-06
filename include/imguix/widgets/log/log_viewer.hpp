#pragma once
#ifndef _IMGUIX_WIDGETS_LOG_VIEWER_HPP_INCLUDED
#define _IMGUIX_WIDGETS_LOG_VIEWER_HPP_INCLUDED

/// \file log_viewer.hpp
/// \brief Reusable tabular log viewer with filtering, selection, and copy actions.

#include <imgui.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_set>

#include <imguix/config/icons.hpp>
#include <imguix/widgets/containers/rounded_panel.hpp>

namespace ImGuiX::Widgets {

    /// \brief Display-ready log row consumed by LogViewer.
    struct LogViewerEntry final {
        std::uint64_t id{0};
        int level_rank{0};
        std::string timestamp;
        std::string level;
        std::string message;
    };

    /// \brief One selectable minimum-level option.
    struct LogViewerLevel final {
        int rank{0};
        const char* label{nullptr};
    };

    /// \brief Persistent selection and filter state owned by the caller.
    struct LogViewerState final {
        int min_level_rank{0};
        std::unordered_set<std::uint64_t> selected_ids;
        std::optional<std::uint64_t> selection_anchor_id;
    };

    /// \brief Localized visible strings used by LogViewer.
    struct LogViewerLabels final {
        const char* title{"Logs"};
        const char* minimum_level{"Minimum level"};

        const char* timestamp{"Timestamp"};
        const char* level{"Level"};
        const char* message{"Message"};

        const char* copy_all{"Copy all"};
        const char* copy_selected{"Copy"};
        const char* deselect{"Deselect"};

        const char* refresh_tooltip{"Refresh logs"};
        const char* clear_tooltip{"Clear logs"};
        const char* open_folder_tooltip{"Open logs folder"};

        const char* copy_row{"Copy row"};
        const char* copy_timestamp{"Copy timestamp"};
        const char* copy_level{"Copy level"};
        const char* copy_message{"Copy message"};
    };

    /// \brief Icons used by LogViewer toolbar and selection actions.
    struct LogViewerIcons final {
        const char* refresh{IMGUIX_ICON_REFRESH};
        const char* copy{IMGUIX_ICON_COPY};
        const char* clear{IMGUIX_ICON_DELETE};
        const char* open_folder{IMGUIX_ICON_FOLDER};
        const char* deselect{IMGUIX_ICON_CLOSE};
    };

    /// \brief Application-owned actions invoked by LogViewer.
    struct LogViewerActions final {
        std::function<void()> on_refresh;
        std::function<void()> on_clear;
        std::function<void()> on_open_folder;
    };

    /// \brief Presentation, localization, and action configuration for LogViewer.
    struct LogViewerConfig final {
        ImVec2 size{0.0f, 0.0f};
        RoundedPanelConfig panel{};

        float toolbar_padding_x{24.0f};
        float toolbar_padding_y{-1.0f};
        float level_combo_width{110.0f};

        const LogViewerLevel* levels{nullptr};
        std::size_t level_count{0};
        LogViewerLabels labels{};
        LogViewerIcons icons{};
        LogViewerActions actions{};

        bool show_refresh{true};
        bool show_copy_all{true};
        bool show_clear{true};
        bool show_open_folder{true};

        bool refresh_enabled{true};
        bool clear_enabled{true};
        bool open_folder_enabled{true};

        bool clear_selection_after_copy{true};
        std::function<std::string(std::size_t)> format_selected_count;
        std::function<std::string(const LogViewerEntry&)> format_clipboard_row;
    };

    /// \brief Render a rounded, filterable, selectable log table.
    ///
    /// The widget owns generic presentation, selection, filtering, context-menu,
    /// and clipboard behavior. The caller owns log acquisition and supplies only
    /// display-ready rows plus application actions.
    /// \param id Stable widget identifier.
    /// \param entries Display-ready rows; may be null when entry_count is zero.
    /// \param entry_count Number of rows in entries.
    /// \param state Persistent filter and selection state.
    /// \param config Presentation, labels, levels, and application callbacks.
    /// \return True when the rounded panel submitted its contents.
    bool LogViewer(
        const char* id,
        const LogViewerEntry* entries,
        std::size_t entry_count,
        LogViewerState& state,
        const LogViewerConfig& config = {});

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#   include "log_viewer.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_LOG_VIEWER_HPP_INCLUDED
