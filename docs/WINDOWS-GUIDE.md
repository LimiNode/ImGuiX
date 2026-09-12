# Windows Guide

For the Russian version, see [WINDOWS-GUIDE-RU.md](WINDOWS-GUIDE-RU.md).

Practical guide for ImGuiX windowing with focus on:

- `include/imguix/core/window/WindowInterface.hpp`
- `include/imguix/core/window/WindowInstance.hpp`
- `include/imguix/core/window/WindowManager.ipp`
- `include/imguix/windows/ImGuiFramedWindow.hpp`
- `include/imguix/windows/ImGuiFramedWindow.ipp`

## Table of Contents

- [Quick Start Path (10 minutes)](#quick-start-path-10-minutes)
- [Which Base Class to Use](#which-base-class-to-use)
- [Runtime Lifecycle and Frame Pipeline](#runtime-lifecycle-and-frame-pipeline)
- [WindowInterface and WindowInstance API Map](#windowinterface-and-windowinstance-api-map)
- [ImGuiFramedWindow Model](#imguiframedwindow-model)
- [WindowFlags Matrix](#windowflags-matrix)
- [ImGuiFramedWindowConfig Matrix](#imguiframedwindowconfig-matrix)
- [Backend Capability Matrix](#backend-capability-matrix)
- [Transparency Behavior Matrix](#transparency-behavior-matrix)
- [Practical Recipes](#practical-recipes)
- [What to Override by Scenario](#what-to-override-by-scenario)
- [Smoke Demo Map](#smoke-demo-map)
- [Common Gotchas](#common-gotchas)
- [Debug Checklist](#debug-checklist)

## Quick Start Path (10 minutes)

1. Choose base class:
- Use `ImGuiFramedWindow` for most operator UIs.
- Use `WindowInstance` only when you need fully custom window chrome/layout.

2. Choose layout mode via flags:
- Classic layout: do not set `WindowFlags::HasCornerIconArea`.
- Corner layout: set `WindowFlags::HasCornerIconArea`.

3. Set minimal config:
- `title_bar_height`
- `side_panel_width`
- frame strokes (`frame_stroke_thickness`, `frame_outer_stroke_thickness`, `frame_inner_stroke_thickness`)

4. Add only required virtual hooks:
- `drawTitleBarText()`
- `drawSidePanel()`
- `drawMenuBar()` (only with `HasMenuBar`)
- `drawCornerIcon()` (only in corner mode)

5. Validate with one smoke target:

```powershell
cmake --build external/ImGuiX/build-mingw --target corner_icon_area_demo_v3 --parallel 36
```

## Which Base Class to Use

| Use case | Recommended base class | Why |
| --- | --- | --- |
| Full custom rendering loop and custom native/chrome behavior | `WindowInstance` | Lowest-level window abstraction with full lifecycle control. |
| Ready framed host window (title, side, corner icon area, control buttons) | `Windows::ImGuiFramedWindow` | Builds on `WindowInstance` and gives reusable framed geometry/styling. |

Rule of thumb:

- Start with `ImGuiFramedWindow`.
- Drop to `WindowInstance` only when framed layout rules are a limitation.

## Runtime Lifecycle and Frame Pipeline

Two levels are involved:

1. `WindowManager` orchestrates all windows.
2. Each `WindowInstance` processes its own frame.

### WindowManager lifecycle

Source: `include/imguix/core/window/WindowManager.ipp`.

- `prepareFrame()`:
  - `flushPending()`
  - `initializePending()` -> `window->fontsStartInit()`, `window->onInit()`, `window->buildFonts()`
  - `removeClosed()`
  - `initializeControllers()`

- `processFrame()` order:
  - `handleEvents()`
  - `tickAll()`
  - `drawContentAll()`
  - `drawUiAll()`
  - `presentAll()`
  - `loadIniAll()`
  - `saveIniAll()`

### Per-window frame contract

Base hooks in `WindowInstance`:

- `handleEvents()`
- `tick()`
- `drawContent()`
- `drawUi()`
- `present()`

`ImGuiFramedWindow` overrides `drawUi()` in backend-specific files and dispatches to:

- `drawClassicLayout(...)` when `WindowFlags::HasCornerIconArea` is not set
- `drawCornerLayout(...)` when `WindowFlags::HasCornerIconArea` is set

## WindowInterface and WindowInstance API Map

### Geometry and state

- Identity: `id()`, `name()`
- Size: `width()`, `height()`, `setSize(int, int)`
- Visibility/focus: `setVisible(bool)`, `setActive(bool)`, `isActive()`
- Open/close: `isOpen()`, `close()`
- Window state: `minimize()`, `maximize()`, `restore()`, `isMaximized()`, `toggleMaximizeRestore()`

### Theme, options, event bus, resources

- Theme: `setTheme(std::string)`, `themeManager()`
- Options: `options()` (control/view)
- Event bus: `eventBus()`
- Resources: `registry()`
- Notifications: `notifications()`

### Fonts and i18n

- Font lookup: `getFont(FontRole)`
- Font manager access: `fontsView()`, `fontsControl()`
- Language: `langStore()`, `requestLanguageChange(...)`, `applyPendingLanguageChange()`
- Manual atlas init API (init-only):
  - `fontsBeginManual()`
  - `fontsSetLocale(...)`
  - `fontsSetRangesPreset(...)`
  - `fontsSetRangesExplicit(...)`
  - `fontsClearRanges()`
  - `fontsAddBody(...)`
  - `fontsAddHeadline(...)`
  - `fontsAddMerge(...)`
  - `fontsBuildNow()`

### ImGui ini persistence

- `iniPath()`
- `initIni()`
- `loadIni()`
- `saveIniNow()`

## ImGuiFramedWindow Model

Main customization hooks:

- `drawTitleBarText()`
- `drawSidePanel()`
- `drawCornerIcon()`
- `drawMenuBar()`

Layout modes:

1. Classic mode:
- Active when `WindowFlags::HasCornerIconArea` is not set.
- Title row always exists.
- Side panel exists only if `side_panel_width > 0`.

2. Corner mode:
- Active when `WindowFlags::HasCornerIconArea` is set.
- Top-left icon area is reserved.
- Side width:
  - `side_panel_width > 0`: explicit width
  - `side_panel_width <= 0`: auto width from corner parameters

## WindowFlags Matrix

Source: `include/imguix/windows/window_flags.hpp`.

| Flag | Effect | Notes |
| --- | --- | --- |
| `HasMenuBar` | Enables menu bar region and `drawMenuBar()` calls | In corner mode, placement depends on `corner_menu_bar_placement`. |
| `EnableTransparency` | Requests transparent backend/native clear path | Backend-dependent behavior. |
| `DisableBackground` | Makes root ImGui host background transparent | Frame/border can remain visible. |
| `ShowControlButtons` | Shows minimize/maximize/close buttons | Geometry depends on style and stroke config. |
| `MacStyledControlButtons` | Circular mac-like buttons | Mutually exclusive with other style flags. |
| `ImGuiStyledControlButtons` | `Widgets::SystemButton` style | Release fallback style if style flags conflict. |
| `ClassicStyledControlButtons` | Text-button style | Mutually exclusive with other style flags. |
| `HasCornerIconArea` | Enables corner layout and icon slot | Switches from classic to corner layout. |
| `CornerModeRounding` | Enables rounded title/side geometry in corner mode | Radius from `corner_icon_mode_rounding_radius`. |
| `CornerModeBorder` | Enables corner-mode border strokes for title/side | Uses `frame_stroke_thickness`. |
| `DefaultControlButtons` | `ShowControlButtons | ImGuiStyledControlButtons` | Convenience alias. |

Control-button style conflict policy (`resolveControlButtonsStyle()`):

- Debug builds: assert if more than one style flag is set.
- Release builds: fallback to ImGui style.

## ImGuiFramedWindowConfig Matrix

Source: `include/imguix/windows/ImGuiFramedWindow.hpp`.

### Geometry and frame

| Field | Meaning | Default / auto semantics |
| --- | --- | --- |
| `min_width`, `min_height` | Minimum client size | `640x480` |
| `frame_corner_radius` | Native rounded region radius and root host rounding source | `8` |
| `resize_border` | Manual resize hit-test thickness (Win32 borderless path) | `8` |
| `title_bar_height` | Title row height | `32` |
| `side_panel_width` | Classic: `<=0` disables side panel; Corner: `<=0` means auto width | `0` |
| `frame_stroke_thickness` | Title/side separator stroke thickness | `1.0f` |
| `frame_outer_stroke_thickness` | Outer host frame stroke thickness | `2.0f` |
| `frame_inner_stroke_thickness` | Inner host frame stroke thickness | `1.0f` |

Notes:

- In the SFML framed backend, `frame_outer_stroke_thickness` + `frame_inner_stroke_thickness` reduce the available layout space of `main_region` and menu slices rendered in the main column on the outer edges they touch.
- This frame-aware inset does not change title-bar, side-panel, or corner-icon chrome geometry.

### Content insets

| Field | Meaning | Auto rule |
| --- | --- | --- |
| `title_content_left_inset` | Baseline left inset for title content | `<0`: style-based auto inset |
| `side_panel_content_left_inset` | Baseline left inset for side-panel content | `<0`: style-based auto inset |
| `main_region_padding` | Inner padding of framed-window main content area | Each component `<0`: fallback to runtime `style.WindowPadding` |
| `side_panel_content_alignment` | Side-panel content alignment policy | `LegacyLeftInset`: left-only legacy placement; `SymmetricInset`: keep panel width, center the inner content region with equal horizontal inset |

Notes:

- `main_region_padding` affects only the `main_region` child where controllers/widgets are rendered.
- `main_region_padding` does not affect title bar, menu bar, side panel, or corner icon area.
- Frame-aware host-stroke insets are still applied first; `main_region_padding` is then used as the inner child padding inside that already reduced rect.
- `SymmetricInset` does not resize the side panel itself; it only narrows and centers the content region inside it.
- `SymmetricInset` is best for equal-width controls or widgets sized from `GetContentRegionAvail().x`.
- `SymmetricInset` does not automatically center arbitrary intrinsic-width text or links item-by-item.

### Corner options

| Field | Meaning | Notes |
| --- | --- | --- |
| `corner_icon_mode_rounding_radius` | Radius for corner-mode title/side rounding | Effective only with `CornerModeRounding`. |
| `corner_rounding_style` | Corner rounding policy (`Legacy`, `NoTopLeftOnTitleAndSide`) | Effective only with `CornerModeRounding`. |
| `corner_icon_mode_area_width` | Reserved width for corner icon area | `<0`: fallback formula. |
| `corner_icon_mode_icon_size` | Corner icon content size | `<0`: auto-fit with frame-aware compensation. |
| `corner_icon_mode_gap` | Gap between icon area and title/side | `<0`: runtime `style.WindowPadding.x`. |
| `corner_menu_bar_placement` | `MainRegion`, `InTitleBar`, `BelowTitleBar` | Effective only with `HasMenuBar`. |
| `title_bar_menu_presentation` | `Menu` or `NavigationStrip` | Effective with `InTitleBar`; the strip removes the leading inset, starts at the title-region edge when the title is empty, and uses theme navigation surfaces. |

### Button labels and clear color

- `close_button_text`, `minimize_button_text`, `maximize_button_text`: labels/IDs for classic text buttons.
- `clear_color`: backend clear color when transparency is disabled.

## Backend Capability Matrix

| Capability | SFML | GLFW | SDL2 |
| --- | --- | --- | --- |
| Native rounded region (`SetWindowRgn`) in framed backend | Yes (Win32 path) | No | No |
| `EnableTransparency` clear-path handling in framed backend | Yes | Yes | Yes |
| `ImGuiFramedWindow::setWindowIcon(...)` meaningful success | Yes | No (returns `false`) | No (returns `false`) |
| Manual Win32 hit-test resize in framed backend | Yes (SFML Win32 path) | No | No |

## Transparency Behavior Matrix

| Control | Scope | Typical result |
| --- | --- | --- |
| `WindowFlags::EnableTransparency` | Backend/native clear path | Transparent clear path (backend-dependent). |
| `WindowFlags::DisableBackground` | Root ImGui host background | Host background transparent; frame can stay visible. |
| `setDisableBackground(bool)` | Runtime toggle for background behavior | Same visual effect as disable-background mode, toggled at runtime. |

Practical sequence:

1. Start with `DisableBackground` to validate framed visuals.
2. Add `EnableTransparency` only when native/translucent surface behavior is required.

## Practical Recipes

### Minimal framed-window skeleton (complete class)

```cpp
#include <imguix/windows/ImGuiFramedWindow.hpp>

class DemoWindow final : public ImGuiX::Windows::ImGuiFramedWindow {
public:
    DemoWindow(int id, ImGuiX::ApplicationContext& app)
        : ImGuiFramedWindow(id, app, "demo_window", "Demo Window", buildFlags(), buildConfig()) {}

    void onInit() override {
        setWindowIcon("data/resources/icons/icon.png");
        create(1280, 720);
    }

protected:
    void drawTitleBarText() override {
        ImGui::TextUnformatted("Demo Window");
    }

    void drawSidePanel() override {
        ImGui::TextUnformatted("A1");
        ImGui::TextUnformatted("A2");
    }

    void drawMenuBar() override {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

private:
    static ImGuiX::Windows::WindowFlags buildFlags() {
        using namespace ImGuiX::Windows;
        return WindowFlags::HasMenuBar | WindowFlags::DefaultControlButtons;
    }

    static ImGuiX::Windows::ImGuiFramedWindowConfig buildConfig() {
        ImGuiX::Windows::ImGuiFramedWindowConfig cfg{};
        cfg.title_bar_height = 40;
        cfg.side_panel_width = 48;
        return cfg;
    }
};
```

### Complete classic-mode setup

```cpp
static ImGuiX::Windows::WindowFlags buildClassicFlags() {
    using namespace ImGuiX::Windows;
    return WindowFlags::HasMenuBar | WindowFlags::DefaultControlButtons;
}

static ImGuiX::Windows::ImGuiFramedWindowConfig buildClassicConfig() {
    ImGuiX::Windows::ImGuiFramedWindowConfig cfg{};
    cfg.title_bar_height = 40;
    cfg.side_panel_width = 48;
    cfg.main_region_padding = ImVec2(16.0f, 12.0f);
    cfg.frame_stroke_thickness = 1.0f;
    return cfg;
}
```

### Complete corner V3 setup

```cpp
static ImGuiX::Windows::WindowFlags buildCornerV3Flags() {
    using namespace ImGuiX::Windows;
    return WindowFlags::DefaultControlButtons |
        WindowFlags::HasCornerIconArea |
        WindowFlags::CornerModeRounding |
        WindowFlags::CornerModeBorder;
}

static ImGuiX::Windows::ImGuiFramedWindowConfig buildCornerV3Config() {
    ImGuiX::Windows::ImGuiFramedWindowConfig cfg{};
    cfg.title_bar_height = 40;
    cfg.side_panel_width = 0; // corner auto-width
    cfg.corner_menu_bar_placement = ImGuiX::Windows::CornerMenuBarPlacement::MainRegion;
    cfg.title_bar_menu_presentation =
        ImGuiX::Windows::TitleBarMenuPresentation::Menu;
    return cfg;
}
```

## What to Override by Scenario

| Scenario | Required overrides | Optional overrides |
| --- | --- | --- |
| Classic app (title + menu + side) | `drawTitleBarText()`, `drawMenuBar()`, `drawSidePanel()` | `drawCornerIcon()` (not used) |
| Corner app (icon + side, no menu) | `drawTitleBarText()`, `drawSidePanel()` | `drawCornerIcon()` |
| Menu in title bar (corner mode) | `drawTitleBarText()`, `drawMenuBar()` | `drawSidePanel()`, `drawCornerIcon()` |
| Navigation strip in title bar | `drawMenuBar()` | `drawTitleBarText()` when the title slot is intentionally empty |

## Smoke Demo Map

Source: `examples/smoke/`.

| Demo | What it demonstrates |
| --- | --- |
| `corner_icon_area_off_demo` | Classic layout, menu + side panel |
| `corner_icon_area_off_no_side_demo` | Classic layout, menu, no side panel |
| `corner_icon_area_off_no_menu_demo` | Classic layout with title only |
| `corner_icon_area_off_no_menu_odd_title_demo` | Classic layout with odd title-height alignment checks |
| `corner_icon_area_demo` | Corner layout + menu in title bar |
| `corner_icon_area_demo_v2` | Corner layout + menu below title bar |
| `corner_icon_area_demo_v3` | Corner layout baseline (no menu bar) |
| `corner_icon_area_demo_v4` | Corner layout + nested child in `main_region` to inspect framed-window padding against mgc-style content flow |
| `corner_icon_area_demo_v3_no_top_left` | Corner layout + `NoTopLeftOnTitleAndSide` |
| `corner_icon_area_demo_v3_mac` | Corner layout + Mac-style control buttons |

Build one smoke target from repository root:

```powershell
cmake --build external/ImGuiX/build-mingw --target corner_icon_area_demo_v3 --parallel 36
```

For full smoke profile, see `agents/imguix-smoke-build.md`.

## Common Gotchas

1. Rounded region clipping on SFML/Win32
- With `SetWindowRgn`, pixels outside region do not exist.

2. `setWindowIcon` behavior differs by backend
- In current `ImGuiFramedWindow`, meaningful success is SFML-only.

3. `side_panel_width` semantics differ by layout mode
- Classic: `<= 0` disables side panel.
- Corner: `<= 0` enables auto width.

4. `side_panel_content_alignment = SymmetricInset` centers the content region, not each widget
- Equal-width controls behave as expected.
- Narrow text/link items still use normal ImGui flow unless you center them explicitly.

5. `corner_menu_bar_placement` has no effect without `HasMenuBar`
- Placement logic runs only when menu flag is active.

6. `title_bar_menu_presentation` has no effect outside `InTitleBar`
- `NavigationStrip` is a chrome presentation policy, not a replacement for
  `drawMenuBar()` and not a domain navigation model.

7. Control-button style flag conflicts
- Multiple style flags trigger debug assert and release fallback.

## Debug Checklist

1. Verify layout mode first (`HasCornerIconArea` on/off).
2. Verify style flag exclusivity for control buttons.
3. Verify `side_panel_width` semantics for the active layout mode.
4. On SFML/Win32, validate corner borders during resize with rounded region enabled.
5. Validate background/transparency combination (`EnableTransparency`, `DisableBackground`, runtime toggle).
