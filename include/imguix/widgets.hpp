#pragma once
#ifndef IMGUIX_WIDGETS_HPP_INCLUDED
#define IMGUIX_WIDGETS_HPP_INCLUDED

/// \file widgets.hpp
/// \brief Aggregated include for all ImGuiX widget extensions.

#include <imguix/widgets/auth/auth_js_panel.hpp>
#include <imguix/widgets/auth/auth_panel.hpp>
#include <imguix/widgets/auth/domain_selector.hpp>
#include <imguix/widgets/auth/proxy_endpoint_fields.hpp>
#include <imguix/widgets/auth/proxy_panel.hpp>

#include <imguix/widgets/controls/circle_button.hpp>
#include <imguix/widgets/controls/icon_button.hpp>
#include <imguix/widgets/controls/icon_combo.hpp>
#include <imguix/widgets/controls/image_badge_button.hpp>
#include <imguix/widgets/controls/system_button.hpp>
#include <imguix/widgets/controls/toggle_button.hpp>

#include <imguix/widgets/input/arrow_stepper.hpp>
#include <imguix/widgets/input/input_vertical_stepper.hpp>
#include <imguix/widgets/input/list_editor.hpp>
#include <imguix/widgets/input/validated_input.hpp>
#include <imguix/widgets/input/validated_multiline_input.hpp>
#include <imguix/widgets/input/validated_password_input.hpp>
#include <imguix/widgets/input/vertical_stepper_buttons.hpp>
#include <imguix/widgets/input/virtual_keyboard.hpp>
#include <imguix/widgets/input/virtual_keyboard_overlay.hpp>

#include <imguix/widgets/misc/loading_spinner.hpp>
#include <imguix/widgets/misc/markers.hpp>
#include <imguix/widgets/misc/text_center.hpp>

#include <imguix/widgets/time/date_picker.hpp>
#include <imguix/widgets/time/days_selector.hpp>
#include <imguix/widgets/time/hours_selector.hpp>
#include <imguix/widgets/time/time_picker.hpp>
#include <imguix/widgets/time/expiry_picker.hpp>

#ifdef IMGUIX_ENABLE_IMPLOT
#   include <imguix/widgets/plot/DualMetricBarsPlot.hpp>
#   include <imguix/widgets/plot/DualMetricBarsSet.hpp>
#   include <imguix/widgets/plot/MetricsPlot.hpp>
#   include <imguix/widgets/plot/StylizedDualMetricBarsPlot.hpp>
#   include <imguix/widgets/plot/StylizedMetricsPlot.hpp>
#   include <imguix/widgets/plot/MetricsPlotSet.hpp>
#   include <imguix/widgets/plot/PlotOHLCChart.hpp>
#endif

#endif // IMGUIX_WIDGETS_HPP_INCLUDED
