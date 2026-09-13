#pragma once
#ifndef _IMGUIX_WIDGETS_TIME_PICKER_HPP_INCLUDED
#define _IMGUIX_WIDGETS_TIME_PICKER_HPP_INCLUDED

/// \file time_picker.hpp
/// \brief Time pickers: absolute time-of-day (HH:MM:SS) and signed offset (+/-HH:MM:SS) with TZ list.
/// \note Requires arrow_stepper.hpp

#include <imgui.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <imguix/widgets/input/arrow_stepper.hpp>
#include <imguix/utils/time_utils.hpp>  // ImGuiX::Utils::format_hms, etc.
#include <imguix/extensions/sizing.hpp> // ImGuiX::Extensions::CalcTimeComboWidth(), etc.
#include <imguix/config/icons.hpp>

namespace ImGuiX::Widgets {

    // ---------- configs & data ---------------------------------------------------

    /// \brief Configuration for time-of-day picker.
    struct TimePickerConfig {
        const char* label       = u8"Time";      ///< Combo label.
        const char* desc        = u8"HH:MM:SS"; ///< Format description.
        float       combo_width = 0.0f;         ///< Width for combo preview.
        bool        use_icon_combo = true;      ///< Use BeginIconCombo for combo.
        const char* icon_text = IMGUIX_ICON_CLOCK; ///< Combo icon glyph.
        bool        show_desc   = true;         ///< Show format description.
        float       field_width = 0.0f;         ///< Width per ArrowStepper field.
    };

    /// \brief Timezone entry with offset and DST flag.
    struct TimeZoneInfo {
        const char* key;        ///< Identifier key.
        const char* label;      ///< Display label.
        int         offset_sec; ///< Offset in seconds from UTC.
        bool        has_dst;    ///< True if observes DST.
    };

    /// \brief Get default timezone list.
    /// \return Reference to built-in time zones.
    const std::vector<TimeZoneInfo>& DefaultTimeZones();

    /// \brief Configuration for signed offset picker.
    struct TimeOffsetPickerConfig {
        const char* label        = u8"Offset";     ///< Combo label.
        const char* desc         = u8"±HH:MM:SS"; ///< Format description.
        float       combo_width  = 0.0f;          ///< Width for combo preview.
        bool        use_icon_combo = true;       ///< Use BeginIconCombo for combo.
        const char* icon_text    = IMGUIX_ICON_GLOBE; ///< Combo icon glyph.
        bool        show_desc    = true;          ///< Show format description.
        bool        show_gmt     = true;          ///< Show GMT offset.
        bool        show_tz_list = true;          ///< Show timezone list.
        float       tz_field_width  = 0.0f;       ///< Width for timezone field.
        float       hms_field_width = 0.0f;       ///< Width for HH:MM:SS field.
        const std::vector<TimeZoneInfo>* time_zones = nullptr; ///< Custom timezone list.
        // Optional presentation labels are appended to preserve source
        // compatibility for existing aggregate initializers.
        const char* timezone_label = u8"Timezone"; ///< Label for the timezone selector inside the popup.
        const char* dst_suffix    = u8" (DST observed)"; ///< Suffix shown for DST-aware zones.
        const char* value_format  = u8"±HH:MM[:SS]"; ///< Inline format hint for manual editing.
        const char* gmt_label     = u8"GMT"; ///< Prefix for the effective UTC offset.
    };

    // ---------- widgets ----------------------------------------------------------

    /// \brief Time-of-day picker editing hour, minute, and second separately.
    /// \param id Unique widget identifier.
    /// \param hour In/out [0..23].
    /// \param minute In/out [0..59].
    /// \param second In/out [0..59].
    /// \param cfg Picker configuration.
    /// \return True if any component changed.
    bool TimePicker(const char* id,
                    int& hour,
                    int& minute,
                    int& second,
                    const TimePickerConfig& cfg = {});

    /// \brief Time-of-day picker (0..86399 seconds).
    /// \param id Unique widget identifier.
    /// \param seconds In/out selected time in seconds since midnight.
    /// \param cfg Picker configuration.
    /// \return True if value changed.
    bool TimePicker(const char* id, int& seconds, const TimePickerConfig& cfg = {});

    /// \brief Signed offset picker with timezone list (sign inside value; no separate checkbox).
    /// \param id Unique widget identifier.
    /// \param offset_sec In/out signed seconds (range not enforced; UI edits magnitude 0..23:59:59).
    /// \param has_dst_out Output flag; set when chosen TZ observes DST (false for Custom).
    /// \param tz_index_io In/out index in timezone list (0 = Custom/manual).
    /// \param cfg Picker configuration.
    /// \return True if value changed.
    bool TimeOffsetPicker(
            const char* id,
            int64_t& offset_sec,
            bool&    has_dst_out,
            int&     tz_index_io,
            const TimeOffsetPickerConfig& cfg = {});

#ifdef IMGUIX_DEMO
    /// \brief Render demo for time and offset pickers.
    inline void DemoTimePicker() {
        static int h = 14, m = 30, s = 0;
        TimePickerConfig simple_tp;
        simple_tp.label = "Time (H/M/S)";
        TimePicker("simple.time", h, m, s, simple_tp);

        static int      seconds = 18 * 3600;
        static int64_t  tz_offset_sec = 2 * 3600;
        static bool     has_dst_out = false;
        static int      tz_index_io = 0;
        static TimePickerConfig       tp_cfg;
        static TimeOffsetPickerConfig to_cfg;
        bool t_changed = TimePicker("time", seconds, tp_cfg);
        ImGui::SameLine();
        bool tz_changed = TimeOffsetPicker(
            "offset",
            tz_offset_sec,
            has_dst_out,
            tz_index_io,
            to_cfg
        );
        if (t_changed || tz_changed) {
            // handle new time/offset
        }
    }
#endif

} // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#   include "time_picker.ipp"
#endif

#endif // _IMGUIX_WIDGETS_TIME_PICKER_HPP_INCLUDED
