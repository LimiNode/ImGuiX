#pragma once
#ifndef _IMGUIX_WIDGETS_PLOT_PLOT_OVERLAY_HPP_INCLUDED
#define _IMGUIX_WIDGETS_PLOT_PLOT_OVERLAY_HPP_INCLUDED

/// \file plot_overlay.hpp
/// \brief Place ordinary ImGui widgets in the screen-space overlay of an ImPlot canvas.

#include <functional>

#include <imgui.h>
#include <implot.h>

namespace ImGuiX::Widgets {

    /// \brief Geometry exposed to a plot overlay callback.
    struct PlotOverlayContext {
        ImVec2 plot_pos{};     ///< Top-left screen position of the plot canvas.
        ImVec2 plot_size{};    ///< Screen-space size of the plot canvas.
        ImVec2 content_pos{};  ///< Screen position after overlay offset and padding.
    };

    /// \brief Placement and clipping policy for a plot overlay.
    struct PlotOverlayConfig {
        ImVec2 offset{};               ///< Screen-space offset from the plot top-left corner.
        ImVec2 padding{-1.0f, -1.0f};  ///< Negative components use ItemInnerSpacing.
        bool clip_to_plot = true;      ///< Clip widget draw commands to the plot canvas.
    };

    using PlotOverlayCallback = std::function<void(const PlotOverlayContext&)>;

    /// \brief Draw ordinary ImGui widgets over the current ImPlot canvas.
    /// \param config Screen-space placement and clipping policy.
    /// \param callback Widgets to submit while the overlay cursor is active.
    /// \pre Called between a successful ImPlot::BeginPlot() and ImPlot::EndPlot().
    /// \post Parent ImGui cursor and layout state are unchanged by the overlay.
    /// \note Popup windows created by the callback are not clipped to the plot, as they
    ///       are separate ImGui windows.
    void PlotOverlay(const PlotOverlayConfig& config, const PlotOverlayCallback& callback);

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#include "plot_overlay.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_PLOT_PLOT_OVERLAY_HPP_INCLUDED
