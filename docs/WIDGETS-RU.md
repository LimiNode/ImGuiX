# Обзор виджетов

ImGuiX предоставляет набор виджетов, сгруппированных по доменам. Примеры ниже —
сокращённые фрагменты из `examples/smoke/widgets_demo.cpp`, если не указано иначе.

## Авторизация

### AuthPanel
Панель логина, объединяющая поля e-mail, пароль, хост и токен с валидацией и
поддержкой виртуальной клавиатуры. Возвращает битовую маску изменений.
```cpp
ImGuiX::Widgets::AuthPanel("login.panel", cfg, data);
```

### AuthJsPanel
Редактирует данные идентификации браузера, такие как User-Agent и Accept-Language.
Поля могут валидироваться и поддерживать виртуальную клавиатуру.
```cpp
ImGuiX::Widgets::AuthJsPanel("js.panel", js_cfg, js_state);
```

### DomainSelector
Выпадающий список с предустановленными доменами и запасным полем для
произвольных значений.
```cpp
ImGuiX::Widgets::DomainSelector("domain.selector", dom_cfg, host);
```

### ProxyPanel
Настройка использования прокси: хост, порт, тип и необязательная авторизация.
```cpp
ImGuiX::Widgets::ProxyPanel("proxy.panel", proxy_cfg, proxy);
```

## Управляющие элементы

### ToggleButton
Анимированный булев переключатель, рисующийся через публичное ImGui API.
```cpp
bool enabled = false;
ImGuiX::Widgets::ToggleButton("demo.toggle", &enabled);
```

### IconButtonCentered
Кнопка, центрирующая иконку или текст внутри рамки.
```cpp
ImGuiX::Widgets::IconButtonCentered("ok", "\\u2713");
```

### CircleButton
Простая круглая кнопка, заполненная выбранным цветом.
```cpp
ImGuiX::Widgets::CircleButton("dot", 16.0f, ImVec4(0.4f,0.8f,0.4f,1.0f));
```

### SystemButton
Кнопка закрытия, сворачивания или развёртывания в стиле системных элементов управления окна.
```cpp
ImGuiX::Widgets::SystemButton("close", ImGuiX::Widgets::SystemButtonType::Close,
                              ImVec2(16,16));
```

## Ввод

### InputTextValidated
Ввод `std::string` с проверкой по regex и опциональным вызовом клавиатуры.
```cpp
ImGuiX::Widgets::InputTextValidated(
    "apikey##validated", "api key", value,
    true, ImGuiX::Widgets::InputValidatePolicy::OnTouch,
    R"(^[A-Za-z0-9.\\-:]+$)", valid
);
```

### InputPasswordWithToggleVK
Поле пароля с встроенной иконкой-глазом и оверлеем виртуальной клавиатуры.
```cpp
ImGuiX::Widgets::PasswordToggleConfig toggle_cfg;
ImGuiX::Widgets::InputPasswordWithToggleVK(
    "pass", "password", pwd,
    true, ImGuiX::Widgets::InputValidatePolicy::OnTouch,
    R"(^[A-Za-z0-9.\\-:]+$)", valid,
    toggle_cfg, kb_cfg
);
```

### ArrowStepper
Компактный числовой степпер со стрелками вверх/вниз и опциональным циклическим режимом.
```cpp
int minutes = 0;
ImGuiX::Widgets::ArrowStepper("mins", minutes);
```

### InputIntVerticalStepper
`InputInt` с вертикальной колонкой кнопок [+] и [−].
```cpp
int amount = 42;
ImGuiX::Widgets::InputIntVerticalStepper("Amount", &amount);
```

### VerticalStepperButtons
Отдельная колонка кнопок [+]/[−], используемая другими степперами.
```cpp
int delta = ImGuiX::Widgets::VerticalStepperButtons("vstep", ImVec2(20,40), 1, 10);
```

### VirtualKeyboard
Экранная клавиатура с несколькими локалями и необязательными кнопками буфера обмена.
```cpp
ImGuiX::Widgets::VirtualKeyboard("vk.email", email, vk_cfg);
```

### VirtualKeyboardOverlay
Центрированный оверлей для `VirtualKeyboard`.
```cpp
bool vk_visible = true;
ImGuiX::Widgets::VirtualKeyboardOverlay("vk", text, vk_cfg, &vk_visible);
```

### ListEditor
Редактор типа combo для списков строк или чисел.
```cpp
ImGuiX::Widgets::ListEditor("list.names", "Names", names);
```

## Разное

### LoadingSpinner
Анимированный спиннер-дуга с настройкой радиуса, толщины и скорости.
```cpp
ImGuiX::Widgets::LoadingSpinner("spinner", sp_cfg);
```

### Helpers центрирования текста
Утилиты для центрирования форматированного, неформатированного или перенесённого текста.
```cpp
ImGuiX::Widgets::TextCenteredFmt("Welcome, %s", user.c_str());
```

### Маркеры
Цветные бейджи и иконки подсказок для помощи, информации, предупреждений и сообщений об успехе.
```cpp
ImGuiX::Widgets::WarningMarker("Funding API rate-limit almost reached");
```

## Время

### TimePicker
Выбор абсолютного времени суток.
```cpp
ImGuiX::Widgets::TimePicker("time", seconds, tp_cfg);
```

### TimeOffsetPicker
Выбор знакового UTC-смещения или часового пояса из предопределённого списка.
```cpp
ImGuiX::Widgets::TimeOffsetPicker("offset", tz_offset, has_dst, tz_index, to_cfg);
```
`TimeOffsetPickerConfig` принимает необязательные подписи
`timezone_label`, `value_format`, `dst_suffix` и `gmt_label`. Эти строки
заимствуются только на время вызова, поэтому локализованный код должен
сохранять исходные `std::string` до возврата виджета.

### DatePicker
Выбор календарной даты по Y/M/D или timestamp.
```cpp
ImGuiX::Widgets::DatePicker("date_struct", y, m, d, dc);
```

### HoursSelector
Селектор сеткой рабочих часов.
```cpp
ImGuiX::Widgets::HoursSelector("hours", hours, hs_cfg);
```

### DaysOfWeekSelector
Виджет-сетка для выбора дней недели с быстрыми пресетами.
```cpp
ImGuiX::Widgets::DaysOfWeekSelector("days", days, ds_cfg);
```
