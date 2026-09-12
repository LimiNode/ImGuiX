#pragma once
#ifndef _IMGUIX_WINDOWS_CORNER_LAYOUT_OPTIONS_HPP_INCLUDED
#define _IMGUIX_WINDOWS_CORNER_LAYOUT_OPTIONS_HPP_INCLUDED

/// \file corner_layout_options.hpp
/// \brief Corner-layout policy enums for ImGui framed windows.

namespace ImGuiX::Windows {

    /// \brief Corner-mode menu bar placement strategy.
    /// \details Effective only when \ref WindowFlags::HasCornerIconArea and \ref WindowFlags::HasMenuBar are enabled.
    enum class CornerMenuBarPlacement {
        MainRegion,  ///< Backward-compatible behavior: menu bar is rendered in main region.
        InTitleBar,  ///< Menu bar is rendered inside title bar, after title text and before control buttons.
        BelowTitleBar ///< Menu bar is rendered in a dedicated child directly below corner title bar.
    };

    /// \brief Presentation policy for a menu rendered inside the title bar.
    enum class TitleBarMenuPresentation {
        Menu,           ///< Preserve normal menu-bar insets and transparent menu header surfaces.
        NavigationStrip ///< Use a flush title-bar strip with theme navigation surfaces.
    };

    /// \brief Corner-mode style for title/side rounding mask.
    enum class CornerRoundingStyle {
        Legacy,                  ///< Preserve existing rounding flags.
        NoTopLeftOnTitleAndSide  ///< Remove top-left rounding for title bar and side panel.
    };

} // namespace ImGuiX::Windows

#endif // _IMGUIX_WINDOWS_CORNER_LAYOUT_OPTIONS_HPP_INCLUDED
