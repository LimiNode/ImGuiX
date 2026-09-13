#include <imgui.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <imguix/config/build.hpp>
#include <imguix/widgets/input/arrow_stepper.hpp>
#include <imguix/utils/time_utils.hpp>  // ImGuiX::Utils::format_hms, etc.
#include <imguix/extensions/sizing.hpp> // ImGuiX::Extensions::CalcTimeComboWidth(), etc.
#include <imguix/widgets/controls/icon_combo.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE const std::vector<TimeZoneInfo>& DefaultTimeZones() {
        static const std::vector<TimeZoneInfo> tz = {
            {u8"_custom",               u8"Custom (manual)",                 0,      false},

            // --- Europe / broker-presets ---
            {u8"Europe/London",         u8"London (UTC+0)",                  0,      true },
            {u8"Europe/Frankfurt",      u8"Frankfurt (UTC+1)",               3600,   true },   // alias to Berlin
            {u8"Europe/Berlin",         u8"Frankfurt/Berlin (UTC+1)",        3600,   true },
            {u8"Europe/Paris",          u8"Paris (UTC+1)",                   3600,   true },
            {u8"Europe/Amsterdam",      u8"Amsterdam (UTC+1)",               3600,   true },
            {u8"Europe/Warsaw",         u8"Warsaw (UTC+1)",                  3600,   true },
            {u8"Europe/Prague",         u8"Prague (UTC+1)",                  3600,   true },
            {u8"Europe/Zurich",         u8"Zurich (UTC+1)",                  3600,   true },

            // EET/EEST — часто время MT-сервера (зимой +2, летом +3)
            {u8"Europe/Athens",         u8"Broker (EET/EEST, UTC+2/+3)",     7200,   true },
            {u8"Europe/Helsinki",       u8"Helsinki (UTC+2)",                7200,   true },
            {u8"Europe/Kyiv",           u8"Kyiv (UTC+2)",                    7200,   true },

            // Фиксированное GMT+3 (без переходов) — встречается у брокеров
            {u8"Etc/GMT-3",             u8"Broker (GMT+3, no DST)",          10800,  false},

            {u8"Europe/Istanbul",       u8"Istanbul (UTC+3)",                10800,  false},   // постоянный UTC+3
            {u8"Europe/Moscow",         u8"Moscow (UTC+3)",                  10800,  false},

            // --- Middle East / Gulf ---
            {u8"Asia/Dubai",            u8"Dubai (UTC+4)",                   14400,  false},
            {u8"Asia/Tbilisi",          u8"Tbilisi (UTC+4)",                 14400,  false},

            // --- Asia ---
            {u8"Asia/Tehran",           u8"Tehran (UTC+3:30)",               12600,  false},
            {u8"Asia/Karachi",          u8"Karachi (UTC+5)",                 18000,  false},
            {u8"Asia/Kolkata",          u8"Kolkata (UTC+5:30)",              19800,  false},
            {u8"Asia/Bangkok",          u8"Bangkok (UTC+7)",                 25200,  false},
            {u8"Asia/Almaty",           u8"Almaty (UTC+6)",                  21600,  false},

            {u8"Asia/Shanghai",         u8"Shanghai (UTC+8)",                28800,  false},
            {u8"Asia/Hong_Kong",        u8"Hong Kong (UTC+8)",               28800,  false},
            {u8"Asia/Singapore",        u8"Singapore (UTC+8)",               28800,  false},
            {u8"Asia/Taipei",           u8"Taipei (UTC+8)",                  28800,  false},
            {u8"Asia/Manila",           u8"Manila (UTC+8)",                  28800,  false},

            {u8"Asia/Tokyo",            u8"Tokyo (UTC+9)",                   32400,  false},
            {u8"Asia/Seoul",            u8"Seoul (UTC+9)",                   32400,  false},

            // --- Oceania ---
            {u8"Australia/Sydney",      u8"Sydney (UTC+10)",                 36000,  true },
            {u8"Pacific/Auckland",      u8"Auckland (UTC+12)",               43200,  true },

            // --- Americas ---
            {u8"America/New_York",      u8"New York (UTC-5)",               -18000,  true },
            {u8"America/Toronto",       u8"Toronto (UTC-5)",                -18000,  true },
            {u8"America/Chicago",       u8"Chicago (UTC-6)",                -21600,  true },
            {u8"America/Denver",        u8"Denver (UTC-7)",                 -25200,  true },
            {u8"America/Phoenix",       u8"Phoenix (UTC-7, no DST)",        -25200,  false},   // без DST
            {u8"America/Los_Angeles",   u8"Los Angeles (UTC-8)",            -28800,  true },
            {u8"America/Anchorage",     u8"Anchorage (UTC-9)",              -32400,  true },

            {u8"America/Sao_Paulo",     u8"Sao Paulo (UTC-3)",              -10800,  true },
            {u8"America/Buenos_Aires",  u8"Buenos Aires (UTC-3)",           -10800,  false},
            {u8"America/Santiago",      u8"Santiago (UTC-4)",               -14400,  true },
            {u8"America/Bogota",        u8"Bogota (UTC-5, no DST)",         -18000,  false},
            {u8"America/Lima",          u8"Lima (UTC-5, no DST)",           -18000,  false},
            {u8"America/Mexico_City",   u8"Mexico City (UTC-6)",            -21600,  true },
            {u8"America/Caracas",       u8"Caracas (UTC-4, no DST)",        -14400,  false},

            // --- Africa (иногда удобно для форекса) ---
            {u8"Africa/Johannesburg",   u8"Johannesburg (UTC+2, no DST)",    7200,   false},
            {u8"Africa/Cairo",          u8"Cairo (UTC+2)",                   7200,   true }
        };
        return tz;
    }

    IMGUIX_IMPL_INLINE bool TimePicker(const char* id,
                           int& hour,
                           int& minute,
                           int& second,
                           const TimePickerConfig& cfg) {
        // Preview uses sanitized copy (originals не трогаем до UI взаимодействия)
        int ph = std::clamp(hour,   0, 23);
        int pm = std::clamp(minute, 0, 59);
        int ps = std::clamp(second, 0, 59);
        std::string preview = ImGuiX::Utils::format_hms(
            ImGuiX::Utils::hms_to_seconds(ph, pm, ps)
        );

        const ImGuiStyle& st = ImGui::GetStyle();
        const float combo_width = cfg.combo_width > 0.0f ?
            cfg.combo_width :
            ImGuiX::Extensions::CalcTimeComboWidth();

        const float field_width = cfg.field_width > 0.0f ?
            cfg.field_width :
            ImGuiX::Extensions::CalcHMSFieldWidth();

        bool changed = false;
        ImGui::PushID(id);
        ImGui::SetNextItemWidth(combo_width);
        bool open = cfg.use_icon_combo ?
            BeginIconCombo(cfg.label ? cfg.label : u8"Time", preview.c_str(), cfg.icon_text) :
            ImGui::BeginCombo(cfg.label ? cfg.label : u8"Time", preview.c_str());
        if (open) {
            if (cfg.show_desc && cfg.desc)
                ImGui::TextUnformatted(cfg.desc);

            // Steppers: wrap enabled (как в версии с seconds-of-day)
            ArrowStepperConfig sc_h{0, 23, 1, true, field_width, u8"%02d h"};
            ArrowStepperConfig sc_m{0, 59, 1, true, field_width, u8"%02d m"};
            ArrowStepperConfig sc_s{0, 59, 1, true, field_width, u8"%02d s"};

            bool any = false;
            any |= ArrowStepper(u8"h", hour,   sc_h);
            any |= ArrowStepper(u8"m", minute, sc_m);
            any |= ArrowStepper(u8"s", second, sc_s);

            // Final clamp (на случай внешних правок переменных)
            int nh = std::clamp(hour,   0, 23);
            int nm = std::clamp(minute, 0, 59);
            int ns = std::clamp(second, 0, 59);

            if (nh != hour || nm != minute || ns != second) {
                hour = nh; minute = nm; second = ns; any = true;
            }

            changed |= any;

            ImGui::EndCombo();
        }
        ImGui::PopID();
        return changed;
    }

    IMGUIX_IMPL_INLINE bool TimePicker(const char* id, int& seconds, const TimePickerConfig& cfg) {
        bool changed = false;
        std::string preview = ImGuiX::Utils::format_hms(seconds);

        const float combo_width = cfg.combo_width > 0.0f ?
            cfg.combo_width :
            ImGuiX::Extensions::CalcTimeComboWidth();

        const float field_width = cfg.field_width > 0.0f ?
            cfg.field_width :
            ImGuiX::Extensions::CalcHMSFieldWidth();

        ImGui::PushID(id);
        ImGui::SetNextItemWidth(combo_width);
        bool open = cfg.use_icon_combo ?
            BeginIconCombo(cfg.label ? cfg.label : u8"Time", preview.c_str(), cfg.icon_text) :
            ImGui::BeginCombo(cfg.label ? cfg.label : u8"Time", preview.c_str());
        if (open) {
            if (cfg.show_desc && cfg.desc) ImGui::TextUnformatted(cfg.desc);

            int h, m, s;
            ImGuiX::Utils::seconds_to_hms(seconds, h, m, s);

            ArrowStepperConfig sc_h{0,23,1,true,field_width,u8"%02d h"};
            ArrowStepperConfig sc_m{0,59,1,true,field_width,u8"%02d m"};
            ArrowStepperConfig sc_s{0,59,1,true,field_width,u8"%02d s"};

            bool any = false;
            any |= ArrowStepper(u8"h", h, sc_h);
            any |= ArrowStepper(u8"m", m, sc_m);
            any |= ArrowStepper(u8"s", s, sc_s);

            int prev = seconds;
            seconds = ImGuiX::Utils::hms_to_seconds(h, m, s);
            changed |= (seconds != prev) || any;

            ImGui::EndCombo();
        }
        ImGui::PopID();
        return changed;
    }

    IMGUIX_IMPL_INLINE bool TimeOffsetPicker(
            const char* id,
            int64_t& offset_sec,
            bool&    has_dst_out,
            int&     tz_index_io,
            const TimeOffsetPickerConfig& cfg) {
        const auto& tzlist = cfg.time_zones ? *cfg.time_zones : DefaultTimeZones();
        if (tz_index_io < 0 || tz_index_io >= static_cast<int>(tzlist.size()))
            tz_index_io = 0;

        const float combo_width = cfg.combo_width > 0.0f ?
            cfg.combo_width :
            ImGuiX::Extensions::CalcTimeComboWidth();

        const float tz_field_width = cfg.tz_field_width > 0.0f ?
            cfg.tz_field_width :
            ImGuiX::Extensions::CalcFieldWidthForSample(u8"Johannesburg (UTC+2, no DST)");

        const float hms_field_width = cfg.hms_field_width > 0.0f ?
            cfg.hms_field_width :
            ImGuiX::Extensions::CalcHMSFieldWidth();

        bool changed = false;
        std::string preview = ImGuiX::Utils::format_signed_hms(offset_sec);

        ImGui::PushID(id);
        ImGui::SetNextItemWidth(combo_width);
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(0, 0),
            ImVec2(FLT_MAX, ImGui::GetTextLineHeightWithSpacing() * 16) // максимум 16 строк
        );
        bool open = cfg.use_icon_combo ?
            BeginIconCombo(cfg.label ? cfg.label : u8"Offset", preview.c_str(), cfg.icon_text) :
            ImGui::BeginCombo(cfg.label ? cfg.label : u8"Offset", preview.c_str());
        if (open) {
            if (cfg.show_desc && cfg.desc) ImGui::TextUnformatted(cfg.desc);

            // Timezone combo
            if (cfg.show_tz_list && !tzlist.empty()) {
                const char* cur = tzlist[tz_index_io].label;
                ImGui::SetNextItemWidth(tz_field_width);
                if (ImGui::BeginCombo(
                        cfg.timezone_label ? cfg.timezone_label : u8"Timezone", cur)) {
                    for (int i = 0; i < (int)tzlist.size(); ++i) {
                        bool sel = (i == tz_index_io);
                        if (ImGui::Selectable(tzlist[i].label, sel)) { tz_index_io = i; changed = true; }
                        if (sel) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }

            const bool custom = (tz_index_io == 0);
            if (!custom) {
                const auto& tz = tzlist[tz_index_io];
                offset_sec = tz.offset_sec;
                has_dst_out = tz.has_dst;
            } else {
                has_dst_out = false;
            }

            // Manual controls (only in Custom)
            if (!custom) ImGui::BeginDisabled(true);

            // 1) Direct string edit with sign parsing
            {
                char buf[32];
                std::snprintf(buf, sizeof(buf), u8"%s", ImGuiX::Utils::format_signed_hms(offset_sec).c_str());
                ImGui::SetNextItemWidth(combo_width);
                if (ImGui::InputText(u8"##offset_str", buf, sizeof(buf))) {
                    int64_t v = 0;
                    if (ImGuiX::Utils::parse_signed_hms(buf, v)) {
                        if (v != offset_sec) { offset_sec = v; changed = true; }
                    }
                }
                ImGui::SameLine();
                ImGui::TextUnformatted(
                    cfg.value_format ? cfg.value_format : u8"±HH:MM[:SS]");
            }

            // 2) H/M/S steppers edit magnitude + allow crossing zero to change sign
            {
                bool positive = offset_sec >= 0;
                uint32_t abs_u = static_cast<uint32_t>(positive ? offset_sec : -offset_sec);
                int h, m, s;
                ImGuiX::Utils::seconds_to_hms(static_cast<int>(abs_u), h, m, s);

                // remember old magnitude to detect "at zero" state
                const int old_abs = ImGuiX::Utils::hms_to_seconds(h, m, s);
                const bool was_positive = positive;

                // steppers: NO wrap here (we control borders ourselves)
                ArrowStepperConfig sc_h{0,23,1,false,hms_field_width,u8"%02d h"};
                ArrowStepperConfig sc_m{0,59,1,false,hms_field_width,u8"%02d m"};
                ArrowStepperConfig sc_s{0,59,1,false,hms_field_width,u8"%02d s"};

                sc_h.disable_at_edges = false;
                sc_m.disable_at_edges = false;
                sc_s.disable_at_edges = false;

                int dh = 0, dm = 0, ds = 0; // last deltas from arrows/wheel; 0 if edited via text
                bool any = false;
                any |= ArrowStepper(u8"oh", h, sc_h, &dh);
                any |= ArrowStepper(u8"om", m, sc_m, &dm);
                any |= ArrowStepper(u8"os", s, sc_s, &ds);

                int new_abs = ImGuiX::Utils::hms_to_seconds(h, m, s);

                // Crossing zero logic:
                // if we were exactly 00:00:00 and user pressed DOWN on any field -> go to -00:00:01
                if (old_abs == 0 && (dh < 0 || dm < 0 || ds < 0)) {
                    positive = false;
                    new_abs = 1; // make minimal 1 second
                    h = 0; m = 0; s = 1;
                    any = true;
                }
                // if we were negative and at -00:00:00 (i.e., abs==0) and user pressed UP -> +00:00:01
                else if (!was_positive && old_abs == 0 && (dh > 0 || dm > 0 || ds > 0)) {
                    positive = true;
                    new_abs = 1;
                    h = 0; m = 0; s = 1;
                    any = true;
                }

                if (any) {
                    uint32_t abs_final = static_cast<uint32_t>(new_abs);
                    int64_t  new_val   = positive ? (int64_t)abs_final : -(int64_t)abs_final;
                    if (new_val != offset_sec) { offset_sec = new_val; changed = true; }
                }
            }

            if (!custom) ImGui::EndDisabled();

            if (cfg.show_gmt) {
                std::string g = ImGuiX::Utils::format_signed_hms(offset_sec);
                ImGui::Text("%s %s%s", cfg.gmt_label ? cfg.gmt_label : u8"GMT", g.c_str(),
                            (!custom && has_dst_out && cfg.dst_suffix != nullptr)
                                ? cfg.dst_suffix
                                : "");
            }

            ImGui::EndCombo();
        }
        ImGui::PopID();
        return changed;
    }

} // namespace ImGuiX::Widgets
