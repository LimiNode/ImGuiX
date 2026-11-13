#pragma once
#ifndef _IMGUIX_WIDGETS_EDITABLE_LABEL_HPP_INCLUDED
#define _IMGUIX_WIDGETS_EDITABLE_LABEL_HPP_INCLUDED

/// \file editable_label.hpp
/// \brief Text label with an aligned edit action.

#include <imguix/widgets/controls/icon_button.hpp>

#include <functional>
#include <string_view>

namespace ImGuiX::Widgets {

    /// \brief Configuration for EditableLabel.
    struct EditableLabelConfig {
        const char* id{"##editable_label"};
        std::string_view text{};
        const char* edit_icon{u8"\uE3C9"};
        const char* edit_tooltip{nullptr};
        ImFont* text_font{nullptr};
        ImVec4 text_color{};
        float gap{6.0f};
        bool use_text_color{false};
        bool disabled{false};
        std::function<void()> on_edit{};
    };

    /// \brief Draw a label and a keyboard-focusable edit button on one row.
    /// \param config Stable identity, borrowed label text, and edit callback.
    /// \return True when the edit button was clicked.
    bool EditableLabel(const EditableLabelConfig& config = {});

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#include "editable_label.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_EDITABLE_LABEL_HPP_INCLUDED
