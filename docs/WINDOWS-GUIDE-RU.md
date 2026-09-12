# Гайд по окнам

English version: [WINDOWS-GUIDE.md](WINDOWS-GUIDE.md).

Практический гайд по оконной модели ImGuiX с фокусом на:

- `include/imguix/core/window/WindowInterface.hpp`
- `include/imguix/core/window/WindowInstance.hpp`
- `include/imguix/core/window/WindowManager.ipp`
- `include/imguix/windows/ImGuiFramedWindow.hpp`
- `include/imguix/windows/ImGuiFramedWindow.ipp`

## Содержание

- [Быстрый путь (10 минут)](#быстрый-путь-10-минут)
- [Какой базовый класс выбрать](#какой-базовый-класс-выбрать)
- [Жизненный цикл выполнения и конвейер кадра (frame pipeline)](#жизненный-цикл-выполнения-и-конвейер-кадра-frame-pipeline)
- [Карта API: WindowInterface и WindowInstance](#карта-api-windowinterface-и-windowinstance)
- [Модель ImGuiFramedWindow](#модель-imguiframedwindow)
- [Матрица WindowFlags](#матрица-windowflags)
- [Матрица ImGuiFramedWindowConfig](#матрица-imguiframedwindowconfig)
- [Матрица возможностей backend-ов](#матрица-возможностей-backend-ов)
- [Матрица поведения прозрачности](#матрица-поведения-прозрачности)
- [Практические рецепты](#практические-рецепты)
- [Что переопределять по сценариям](#что-переопределять-по-сценариям)
- [Карта smoke-примеров](#карта-smoke-примеров)
- [Частые подводные камни (gotchas)](#частые-подводные-камни-gotchas)
- [Чеклист отладки (Debug Checklist)](#чеклист-отладки-debug-checklist)

## Быстрый путь (10 минут)

1. Выберите базовый класс:
- `ImGuiFramedWindow` для большинства операторских UI.
- `WindowInstance` только если нужна полностью кастомная геометрия/рамка.

2. Выберите режим layout через flags:
- Classic layout: не включать `WindowFlags::HasCornerIconArea`.
- Corner layout: включить `WindowFlags::HasCornerIconArea`.

3. Задайте базовый config:
- `title_bar_height`
- `side_panel_width`
- stroke-поля рамки (`frame_stroke_thickness`, `frame_outer_stroke_thickness`, `frame_inner_stroke_thickness`)

4. Переопределите только нужные hooks:
- `drawTitleBarText()`
- `drawSidePanel()`
- `drawMenuBar()` (только при `HasMenuBar`)
- `drawCornerIcon()` (только в corner mode)

5. Проверьте хотя бы один smoke-таргет:

```powershell
cmake --build external/ImGuiX/build-mingw --target corner_icon_area_demo_v3 --parallel 36
```

## Какой базовый класс выбрать

| Сценарий | Рекомендуемый класс | Почему |
| --- | --- | --- |
| Нужен полностью кастомный цикл отрисовки и собственная рамка | `WindowInstance` | Низкоуровневый контроль lifecycle и window state. |
| Нужна готовая framed-модель (title, side panel, corner icon, системные кнопки) | `Windows::ImGuiFramedWindow` | Поверх `WindowInstance`, даёт готовую геометрию и style-переключатели. |

Правило:

- Начинайте с `ImGuiFramedWindow`.
- Переходите на `WindowInstance`, только если ограничения framed-layout мешают задаче.

## Жизненный цикл выполнения и конвейер кадра (frame pipeline)

Есть два уровня:

1. `WindowManager` оркестрирует все окна.
2. Каждый `WindowInstance` проходит свой frame-cycle.

### Lifecycle в WindowManager

Источник: `include/imguix/core/window/WindowManager.ipp`.

- `prepareFrame()`:
  - `flushPending()`
  - `initializePending()` -> `window->fontsStartInit()`, `window->onInit()`, `window->buildFonts()`
  - `removeClosed()`
  - `initializeControllers()`

- `processFrame()`:
  - `handleEvents()`
  - `tickAll()`
  - `drawContentAll()`
  - `drawUiAll()`
  - `presentAll()`
  - `loadIniAll()`
  - `saveIniAll()`

### Контракт кадра окна

Базовые хуки `WindowInstance`:

- `handleEvents()`
- `tick()`
- `drawContent()`
- `drawUi()`
- `present()`

`ImGuiFramedWindow` переопределяет `drawUi()` в backend-specific файлах и внутри вызывает:

- `drawClassicLayout(...)`, когда `WindowFlags::HasCornerIconArea` выключен
- `drawCornerLayout(...)`, когда `WindowFlags::HasCornerIconArea` включен

## Карта API: WindowInterface и WindowInstance

### Геометрия и состояние

- Идентичность: `id()`, `name()`
- Размеры: `width()`, `height()`, `setSize(int, int)`
- Видимость/фокус: `setVisible(bool)`, `setActive(bool)`, `isActive()`
- Жизненный цикл: `isOpen()`, `close()`
- Состояние окна: `minimize()`, `maximize()`, `restore()`, `isMaximized()`, `toggleMaximizeRestore()`

### Theme, options, event bus, resources

- Тема: `setTheme(std::string)`, `themeManager()`
- Опции: `options()`
- События: `eventBus()`
- Ресурсы: `registry()`
- Уведомления: `notifications()`

### Шрифты и i18n

- Поиск шрифта: `getFont(FontRole)`
- Доступ к FontManager: `fontsView()`, `fontsControl()`
- Язык: `langStore()`, `requestLanguageChange(...)`, `applyPendingLanguageChange()`
- Init-only API для ручной сборки atlas:
  - `fontsBeginManual()`
  - `fontsSetLocale(...)`
  - `fontsSetRangesPreset(...)`
  - `fontsSetRangesExplicit(...)`
  - `fontsClearRanges()`
  - `fontsAddBody(...)`
  - `fontsAddHeadline(...)`
  - `fontsAddMerge(...)`
  - `fontsBuildNow()`

### Сохранение ImGui ini

- `iniPath()`
- `initIni()`
- `loadIni()`
- `saveIniNow()`

## Модель ImGuiFramedWindow

Главные hooks для кастомизации:

- `drawTitleBarText()`
- `drawSidePanel()`
- `drawCornerIcon()`
- `drawMenuBar()`

Режимы layout:

1. Classic mode:
- Активен, если `WindowFlags::HasCornerIconArea` выключен.
- Строка title есть всегда.
- Side panel есть только если `side_panel_width > 0`.

2. Corner mode:
- Активен, если `WindowFlags::HasCornerIconArea` включен.
- Резервируется top-left зона под icon slot.
- Ширина side panel:
  - `side_panel_width > 0`: явная
  - `side_panel_width <= 0`: auto-width из corner-параметров

## Матрица WindowFlags

Источник: `include/imguix/windows/window_flags.hpp`.

| Флаг | Эффект | Важно |
| --- | --- | --- |
| `HasMenuBar` | Включает menu bar region и вызов `drawMenuBar()` | В corner mode позиция зависит от `corner_menu_bar_placement`. |
| `EnableTransparency` | Включает прозрачный backend/native clear path | Поведение зависит от backend-а. |
| `DisableBackground` | Делает фон root ImGui host прозрачным | Рамка/бордер при этом могут оставаться видимыми. |
| `ShowControlButtons` | Показывает min/max/close | Геометрия зависит от style и stroke config. |
| `MacStyledControlButtons` | Круглые кнопки в стиле macOS | Взаимоисключаемо с другими style-флагами. |
| `ImGuiStyledControlButtons` | Стиль `Widgets::SystemButton` | Release fallback при конфликте style-флагов. |
| `ClassicStyledControlButtons` | Текстовые кнопки | Взаимоисключаемо с другими style-флагами. |
| `HasCornerIconArea` | Включает corner layout и icon slot | Переключает classic -> corner layout. |
| `CornerModeRounding` | Включает скругление title/side в corner mode | Радиус из `corner_icon_mode_rounding_radius`. |
| `CornerModeBorder` | Включает border-штрихи title/side в corner mode | Использует `frame_stroke_thickness`. |
| `DefaultControlButtons` | `ShowControlButtons | ImGuiStyledControlButtons` | Shortcut. |

Политика конфликта стилей кнопок (`resolveControlButtonsStyle()`):

- Debug: assert, если включено больше одного style-флага.
- Release: fallback к ImGui style.

## Матрица ImGuiFramedWindowConfig

Источник: `include/imguix/windows/ImGuiFramedWindow.hpp`.

### Геометрия и рамка

| Поле | Назначение | Default / auto |
| --- | --- | --- |
| `min_width`, `min_height` | Минимальный размер client area | `640x480` |
| `frame_corner_radius` | Радиус native region и источник скругления root host | `8` |
| `resize_border` | Толщина manual resize hit-test (Win32 borderless path) | `8` |
| `title_bar_height` | Высота title row | `32` |
| `side_panel_width` | Classic: `<=0` выключает side panel; Corner: `<=0` включает auto-width | `0` |
| `frame_stroke_thickness` | Толщина разделителей title/side | `1.0f` |
| `frame_outer_stroke_thickness` | Толщина внешнего host-frame штриха | `2.0f` |
| `frame_inner_stroke_thickness` | Толщина внутреннего host-frame штриха | `1.0f` |

Примечания:

- В SFML framed backend сумма `frame_outer_stroke_thickness` + `frame_inner_stroke_thickness` уменьшает доступную геометрию `main_region` и menu slices основного столбца по тем внешним сторонам, которых они касаются.
- Этот frame-aware inset не меняет геометрию title bar, side panel и corner icon chrome.

### Insets контента

| Поле | Назначение | Auto-правило |
| --- | --- | --- |
| `title_content_left_inset` | Базовый левый inset для title content | `<0`: auto по style |
| `side_panel_content_left_inset` | Базовый левый inset для side content | `<0`: auto по style |
| `main_region_padding` | Внутренний padding основной области framed-window | Каждый компонент `<0`: fallback к runtime `style.WindowPadding` |
| `side_panel_content_alignment` | Политика выравнивания контента внутри side panel | `LegacyLeftInset`: старое выравнивание только слева; `SymmetricInset`: ширина panel не меняется, а внутренняя content-area получает симметричные horizontal inset-ы |

Примечания:

- `main_region_padding` влияет только на `main_region`, где рендерятся контроллеры и виджеты.
- `main_region_padding` не меняет padding у title bar, menu bar, side panel и corner icon area.
- Frame-aware inset от host-frame stroke сначала уменьшает доступный rect, и только потом `main_region_padding` применяется как внутренний child padding внутри уже уменьшенной области.
- `SymmetricInset` не меняет ширину самой side panel; он только сужает и центрирует внутреннюю область контента.
- `SymmetricInset` лучше всего подходит для контролов одинаковой ширины или виджетов, которые берут размер из `GetContentRegionAvail().x`.
- `SymmetricInset` не означает автоматическое поэлементное центрирование узкого текста или ссылок.

### Corner-параметры

| Поле | Назначение | Важно |
| --- | --- | --- |
| `corner_icon_mode_rounding_radius` | Радиус скругления corner title/side | Работает только при `CornerModeRounding`. |
| `corner_rounding_style` | Политика скругления (`Legacy`, `NoTopLeftOnTitleAndSide`) | Работает только при `CornerModeRounding`. |
| `corner_icon_mode_area_width` | Ширина зоны corner icon | `<0`: fallback формула. |
| `corner_icon_mode_icon_size` | Размер corner icon | `<0`: auto-fit с frame-aware компенсацией. |
| `corner_icon_mode_gap` | Gap между icon area и title/side | `<0`: runtime `style.WindowPadding.x`. |
| `corner_menu_bar_placement` | `MainRegion`, `InTitleBar`, `BelowTitleBar` | Имеет смысл только при `HasMenuBar`. |
| `title_bar_menu_presentation` | `Menu` или `NavigationStrip` | Работает с `InTitleBar`; strip убирает ведущий inset, при пустом title начинается от края title-region и использует theme navigation surfaces. |

### Подписи кнопок и clear color

- `close_button_text`, `minimize_button_text`, `maximize_button_text`: labels/IDs для classic text-buttons.
- `clear_color`: цвет очистки backend-а при выключенной прозрачности.

## Матрица возможностей backend-ов

| Возможность | SFML | GLFW | SDL2 |
| --- | --- | --- | --- |
| Native rounded region (`SetWindowRgn`) во framed backend | Да (Win32 path) | Нет | Нет |
| Поддержка `EnableTransparency` во framed backend | Да | Да | Да |
| Meaningful успех `ImGuiFramedWindow::setWindowIcon(...)` | Да | Нет (`false`) | Нет (`false`) |
| Manual Win32 hit-test resize во framed backend | Да (SFML Win32 path) | Нет | Нет |

## Матрица поведения прозрачности

| Управление | Область | Типичный визуальный результат |
| --- | --- | --- |
| `WindowFlags::EnableTransparency` | Backend/native clear path | Прозрачный clear path (backend-зависимо). |
| `WindowFlags::DisableBackground` | Фон root ImGui host | Фон host-прозрачный, рамка может оставаться видимой. |
| `setDisableBackground(bool)` | Runtime-тоггл фонового поведения | То же поведение, переключаемое во время работы. |

Практическая последовательность:

1. Сначала проверяйте визуал через `DisableBackground`.
2. Добавляйте `EnableTransparency`, если нужна именно native/translucent поверхность backend-окна.

## Практические рецепты

### Минимальный skeleton класса framed-окна (полный пример)

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

### Полный setup для classic mode

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

### Полный setup для corner V3

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

## Что переопределять по сценариям

| Сценарий | Обязательные overrides | Опциональные overrides |
| --- | --- | --- |
| Classic app (title + menu + side) | `drawTitleBarText()`, `drawMenuBar()`, `drawSidePanel()` | `drawCornerIcon()` (обычно не нужен) |
| Corner app (icon + side, без menu) | `drawTitleBarText()`, `drawSidePanel()` | `drawCornerIcon()` |
| Menu in title bar (corner mode) | `drawTitleBarText()`, `drawMenuBar()` | `drawSidePanel()`, `drawCornerIcon()` |
| Navigation strip in title bar | `drawMenuBar()` | `drawTitleBarText()`, если title slot намеренно пуст |

## Карта smoke-примеров

Источник: `examples/smoke/`.

| Пример | Что демонстрирует |
| --- | --- |
| `corner_icon_area_off_demo` | Classic layout, menu + side panel |
| `corner_icon_area_off_no_side_demo` | Classic layout, menu, side panel off |
| `corner_icon_area_off_no_menu_demo` | Classic layout только с title row |
| `corner_icon_area_off_no_menu_odd_title_demo` | Проверка выравнивания при odd высоте title |
| `corner_icon_area_demo` | Corner layout + menu in title bar |
| `corner_icon_area_demo_v2` | Corner layout + menu below title |
| `corner_icon_area_demo_v3` | Corner layout baseline без menu bar |
| `corner_icon_area_demo_v4` | Corner layout + вложенный child в `main_region` для проверки framed-window padding в mgc-подобном content flow |
| `corner_icon_area_demo_v3_no_top_left` | Corner layout с `NoTopLeftOnTitleAndSide` |
| `corner_icon_area_demo_v3_mac` | Corner layout + Mac-style control buttons |

Сборка одного smoke-target из корня:

```powershell
cmake --build external/ImGuiX/build-mingw --target corner_icon_area_demo_v3 --parallel 36
```

Полный профиль сборки smoke-таргетов: `agents/imguix-smoke-build.md`.

## Частые подводные камни (gotchas)

1. Клиппинг углов в SFML/Win32
- При `SetWindowRgn` пикселей вне региона нет.

2. Разное поведение `setWindowIcon`
- В текущем `ImGuiFramedWindow` meaningful успех есть только на SFML.

3. Разная семантика `side_panel_width`
- Classic: `<= 0` выключает side panel.
- Corner: `<= 0` включает auto width.

4. `side_panel_content_alignment = SymmetricInset` центрирует content-area, а не каждый виджет отдельно
- Контролы одинаковой ширины ведут себя ожидаемо.
- Узкий текст/ссылки всё ещё следуют обычному ImGui flow, если не центрировать их вручную.

5. `corner_menu_bar_placement` без `HasMenuBar` не влияет
- Логика placement выполняется только при активном menu-флаге.

6. `title_bar_menu_presentation` работает только с `InTitleBar`
- `NavigationStrip` — это presentation policy chrome, а не замена
  `drawMenuBar()` и не доменная navigation model.

7. Конфликт style-флагов control buttons
- Несколько style-флагов дают debug assert и release fallback.

## Чеклист отладки (Debug Checklist)

1. Проверить выбранный layout mode (`HasCornerIconArea` вкл/выкл).
2. Проверить, что style-флаги control buttons не конфликтуют.
3. Проверить семантику `side_panel_width` для текущего режима.
4. Для SFML/Win32 проверить углы и бордер во время resize при rounded region.
5. Проверить комбинацию прозрачности (`EnableTransparency`, `DisableBackground`, runtime toggle).
