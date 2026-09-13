# Widgets Overview

ImGuiX provides a collection of widgets grouped by domain. The examples below are
trimmed snippets taken from `examples/smoke/widgets_demo.cpp` unless noted otherwise.

For the Russian version, see [WIDGETS-RU.md](WIDGETS-RU.md).

## Authorization

### AuthPanel
Login panel combining email, password, host and token fields with validation and
virtual keyboard support. Returns a bitmask of changes.
```cpp
ImGuiX::Widgets::AuthPanel("login.panel", cfg, data);
```

### AuthJsPanel
Edits browser identification data such as User-Agent and Accept-Language. Fields
can be validated and backed by a virtual keyboard.
```cpp
ImGuiX::Widgets::AuthJsPanel("js.panel", js_cfg, js_state);
```

### DomainSelector
Dropdown with preset domains and a fallback input for custom values.
```cpp
ImGuiX::Widgets::DomainSelector("domain.selector", dom_cfg, host);
```

### ProxyPanel
Configures proxy usage with host, port, type and optional authentication.
```cpp
ImGuiX::Widgets::ProxyPanel("proxy.panel", proxy_cfg, proxy);
```

## Controls

### ToggleButton
Animated boolean toggle drawn using public ImGui API.
```cpp
bool enabled = false;
ImGuiX::Widgets::ToggleButton("demo.toggle", &enabled);
```

### IconButtonCentered
Button that centers an icon or text inside its frame.
```cpp
ImGuiX::Widgets::IconButtonCentered("ok", "\u2713");
```

### CircleButton
Simple circular button filled with a chosen color.
```cpp
ImGuiX::Widgets::CircleButton("dot", 16.0f, ImVec4(0.4f,0.8f,0.4f,1.0f));
```

### SystemButton
Close, minimize or maximize button styled like native window controls.
```cpp
ImGuiX::Widgets::SystemButton("close", ImGuiX::Widgets::SystemButtonType::Close,
                              ImVec2(16,16));
```

## Input

### InputTextValidated
`std::string` input with regex validation and optional keyboard trigger.
```cpp
ImGuiX::Widgets::InputTextValidated(
    "apikey##validated", "api key", value,
    true, ImGuiX::Widgets::InputValidatePolicy::OnTouch,
    R"(^[A-Za-z0-9.\-:]+$)", valid
);
```

### InputPasswordWithToggleVK
Password field with inline eye toggle and virtual keyboard overlay.
```cpp
ImGuiX::Widgets::PasswordToggleConfig toggle_cfg;
ImGuiX::Widgets::InputPasswordWithToggleVK(
    "pass", "password", pwd,
    true, ImGuiX::Widgets::InputValidatePolicy::OnTouch,
    R"(^[A-Za-z0-9.\-:]+$)", valid,
    toggle_cfg, kb_cfg
);
```

### ArrowStepper
Compact numeric stepper with up/down arrows and optional wrap.
```cpp
int minutes = 0;
ImGuiX::Widgets::ArrowStepper("mins", minutes);
```

### InputIntVerticalStepper
`InputInt` with a vertical column of [+] and [−] buttons.
```cpp
int amount = 42;
ImGuiX::Widgets::InputIntVerticalStepper("Amount", &amount);
```

### VerticalStepperButtons
Standalone [+]/[−] button column used by other steppers.
```cpp
int delta = ImGuiX::Widgets::VerticalStepperButtons("vstep", ImVec2(20,40), 1, 10);
```

### VirtualKeyboard
On-screen keyboard with multiple locales and optional clipboard buttons.
```cpp
ImGuiX::Widgets::VirtualKeyboard("vk.email", email, vk_cfg);
```

### VirtualKeyboardOverlay
Centered overlay wrapper for `VirtualKeyboard`.
```cpp
bool vk_visible = true;
ImGuiX::Widgets::VirtualKeyboardOverlay("vk", text, vk_cfg, &vk_visible);
```

### ListEditor
Combo-like editor for lists of strings or integers.
```cpp
ImGuiX::Widgets::ListEditor("list.names", "Names", names);
```

## Misc

### LoadingSpinner
Animated arc spinner configurable by radius, thickness and speed.
```cpp
ImGuiX::Widgets::LoadingSpinner("spinner", sp_cfg);
```

### Text Center Helpers
Utilities to center formatted, unformatted or wrapped text blocks.
```cpp
ImGuiX::Widgets::TextCenteredFmt("Welcome, %s", user.c_str());
```

### Markers
Colored badges and tooltip icons for help, info, warnings and success messages.
```cpp
ImGuiX::Widgets::WarningMarker("Funding API rate-limit almost reached");
```

## Time

### TimePicker
Picker for absolute time of day.
```cpp
ImGuiX::Widgets::TimePicker("time", seconds, tp_cfg);
```

### TimeOffsetPicker
Selects a signed UTC offset or timezone from a predefined list.
```cpp
ImGuiX::Widgets::TimeOffsetPicker("offset", tz_offset, has_dst, tz_index, to_cfg);
```
`TimeOffsetPickerConfig` accepts optional `timezone_label`, `value_format`,
`dst_suffix`, and `gmt_label` overrides.  These labels are borrowed only for
the duration of the call, so localized callers must keep their backing strings
alive until the widget returns.

### DatePicker
Calendar date picker operating on Y/M/D or timestamp.
```cpp
ImGuiX::Widgets::DatePicker("date_struct", y, m, d, dc);
```

### HoursSelector
Grid-based selector for working hours.
```cpp
ImGuiX::Widgets::HoursSelector("hours", hours, hs_cfg);
```

### DaysOfWeekSelector
Grid widget for choosing weekdays with quick presets.
```cpp
ImGuiX::Widgets::DaysOfWeekSelector("days", days, ds_cfg);
```

## Plots

### MetricsPlot
Categorical bar/line chart with drag-and-drop selection. A model can push new
measurements via `MetricsPlotUpdateEvent`, and controllers update their local
`MetricsPlotData` and `MetricsPlotState`.
```cpp
// Model
auto data = std::make_shared<ImGuiX::Widgets::MetricsPlotData>(collect());
bus.notifyAsync(ImGuiX::Events::MetricsPlotUpdateEvent{data});

// Controller
eventBus().subscribe<ImGuiX::Events::MetricsPlotUpdateEvent>(
    [this](const auto& e){ plot_data = *e.data; });
ImGuiX::Widgets::MetricsPlot(plot_data, plot_state);
```
