#include <iostream>
#include <imguix/core.hpp>
#include <imguix/widgets/log/log_viewer.hpp>
#include <random>
#include <cmath>
#include <array>
#include <string>
#include <vector>

// === Core UI: Ввод, валидация, списки, текст ===
#include <imguix/widgets/input/validated_input.hpp>
#include <imguix/widgets/input/validated_password_input.hpp>
#include <imguix/widgets/input/input_vertical_stepper.hpp>
#include <imguix/widgets/misc/text_center.hpp>
#include <imguix/widgets/input/list_editor.hpp>

// === Авторизация и связанные панели ===
#include <imguix/widgets/auth/auth_panel.hpp>
#include <imguix/widgets/auth/auth_js_panel.hpp>
#include <imguix/widgets/auth/domain_selector.hpp>
#include <imguix/widgets/input/virtual_keyboard.hpp>
#include <imguix/widgets/input/InputTextWithIconToggle.hpp>

// === Время, дата, часы ===
#include <imguix/widgets/time/time_picker.hpp>
#include <imguix/widgets/time/date_picker.hpp>
#include <imguix/widgets/time/hours_selector.hpp>
#include <imguix/widgets/time/days_selector.hpp>
#include <imguix/widgets/time/expiry_picker.hpp>

// === Уведомления ===
#include <imguix/widgets/notify/notifications.hpp>

// === Дополнительно: прокси, спиннер, маркеры ===
#include <imguix/widgets/auth/proxy_panel.hpp>
#include <imguix/widgets/misc/loading_spinner.hpp>
#include <imguix/widgets/misc/markers.hpp>
#include <imguix/widgets/misc/theme_picker.hpp>
#include <imguix/widgets/controls/icon_combo.hpp>

// === Themes ===
#include <imguix/themes/CorporateGreyTheme.hpp>
#include <imguix/themes/DarkCharcoalTheme.hpp>
#include <imguix/themes/DarkGraphiteTheme.hpp>
#include <imguix/themes/DarkTealTheme.hpp>
#include <imguix/themes/DeepDarkTheme.hpp>
#include <imguix/themes/GoldBlackTheme.hpp>
#include <imguix/themes/GreenBlueTheme.hpp>
#include <imguix/themes/LightBlueTheme.hpp>
#include <imguix/themes/LightGreenTheme.hpp>
#include <imguix/themes/OSXTheme.hpp>
#include <imguix/themes/PearlLightTheme.hpp>
#include <imguix/themes/SlateDarkTheme.hpp>
#include <imguix/themes/VisualStudioDarkTheme.hpp>
#include <imguix/themes/Y2KTheme.hpp>
#include <imguix/themes/CyberY2KTheme.hpp>
#include <imguix/themes/CyberpunkUITheme.hpp>
#include <imguix/themes/NightOwlTheme.hpp>
#include <imguix/themes/NordTheme.hpp>
#include <imguix/themes/TokyoNightStormTheme.hpp>
#include <imguix/themes/TokyoNightTheme.hpp>

// === Графики ===
#ifdef IMGUIX_ENABLE_IMPLOT
#include <imguix/widgets/plot/PlotOHLCChart.hpp>
#include <imguix/widgets/plot/MetricsPlot.hpp>
#include <imguix/widgets/plot/MetricsPlotSet.hpp>
#include <imguix/widgets/plot/DualMetricBarsPlot.hpp>
#include <imguix/widgets/plot/DualMetricBarsSet.hpp>
#endif

#ifdef IMGUIX_DEMO
#include <imguix/widgets/external_widgets_demo.hpp>
#endif

#define IMGUIX_EMOJI_ROCKET u8"\U0001F680"

#ifdef IMGUIX_ENABLE_IMPLOT
/// \brief Generate synthetic OHLCV bars.
/// \param out Output bar container.
/// \param count Number of bars to produce.
/// \param tf_sec Timeframe in seconds.
/// \param start_time Starting timestamp.
/// \param start_price Starting price.
/// \param seed Random seed.
inline static void GenerateBars(
        std::vector<ImGuiX::Widgets::OhlcvBar>& out,
        int count, int tf_sec, std::uint64_t start_time,
        double start_price, unsigned seed
    ) {
    using BarT = ImGuiX::Widgets::OhlcvBar;
    out.clear();
    out.reserve(std::max(0, count));

    const std::uint64_t tf_ms = static_cast<std::uint64_t>(tf_sec);

    std::mt19937 rng(seed);
    std::normal_distribution<double> step(0.0, 0.002);      // ~0.2% шаг
    std::normal_distribution<double> wick(0.0, 0.0008);     // фитили
    std::lognormal_distribution<double> voln(0.0, 0.25);    // шум для объёма

    double prev_close = std::max(0.1, start_price);

    for (int i = 0; i < count; ++i) {
        BarT b;
        b.time  = start_time + static_cast<std::uint64_t>(i) * tf_sec;

        const double drift = 0.00002 * tf_sec;              // лёгкий дрейф по ТФ
        const double ch    = (drift + step(rng)) * prev_close;

        b.open  = prev_close;
        b.close = std::max(0.01, b.open + ch);

        // фитили на основе диапазона бара
        const double base_range = std::max(1e-6, std::abs(b.close - b.open));
        const double up_wick    = std::max(0.0, (base_range * 0.5) + wick(rng) * prev_close);
        const double dn_wick    = std::max(0.0, (base_range * 0.5) + wick(rng) * prev_close);

        b.high  = std::max({b.open, b.close}) + up_wick;
        b.low   = std::max(0.01, std::min({b.open, b.close}) - dn_wick);

        // объём: базовый + пропорционален диапазону
        const double range = b.high - b.low;
        b.volume = std::max(0.0, 800.0 + 50000.0 * range / std::max(1e-6, b.close) * voln(rng));

        out.push_back(b);
        prev_close = b.close;
    }
}
#endif


// Контроллер окна демо-виджетов
class WidgetsController : public ImGuiX::Controller {
public:
    WidgetsController(ImGuiX::WindowInterface& window)
        : Controller(window) {
        m_state.auth_data.host     = options().getStrOr("host", "demo.local");
        m_state.auth_data.email    = options().getStrOr("email", "guest@example.com");
        m_state.auth_data.password = options().getStrOr("password", "");
        ImGuiX::Widgets::ApplyStoredTheme(this);
    }

    void drawContent() override {
        // пространство для фонового рендера; всё демо — в drawUi()
    }

    void drawUi() override {
        ImGui::PushID(window().id());
        ImGui::PushFont(nullptr, 18.0f); // Размер шрифта (обёртка ImGuiX)
#       ifdef IMGUIX_DEMO
        ImGuiX::Widgets::DemoExternalWidgets();
        ImGui::Begin("Widgets Demo");

        ImGui::Separator();
        drawWidgetsDemo();

        ImGui::End();
#       endif
        ImGui::PopFont();
        ImGui::PopID();
    }

private:
    // --- служебные поля контроллера (пример, если пригодятся) ---
    std::string m_last_font_lang;
    int         m_items_count{1};
    int         m_notif_count{2};

    // Состояние демо: хранит конфиги и данные всех виджетов
    struct WidgetsDemoState {
        // ------------------ 1) Авторизация и вход ------------------
        ImGuiX::Widgets::AuthPanelConfig auth_cfg{};   // Конфиг панели логина
        ImGuiX::Widgets::AuthData        auth_data;    // Данные входа (email/password/host)
        ImGuiX::Widgets::AuthJsSettings  js_st{};      // Настройки JS-панели

        // ------------------ 2) Виртуальные клавиатуры ------------------
        ImGuiX::Widgets::VirtualKeyboardConfig kcfg;   // Общий конфиг экранной клавиатуры
        bool kbd_email = false; // Тогглер VK для email
        bool kbd_pass  = false; // Тогглер VK для password
        bool kbd_host  = false; // Тогглер VK для host

        // ------------------ 3) Домен и прокси ------------------
        ImGuiX::Widgets::DomainSelectorConfig dom_cfg{};      // Конфиг выбора домена
        ImGuiX::Widgets::ProxySettings        proxy{};        // Настройки прокси
        ImGuiX::Widgets::ProxyPanelConfig     proxy_cfg{};    // Конфиг панели прокси

        // ------------------ 4) Часы (рабочие периоды) ------------------
        std::vector<int>                     hours{9,10,11,12,13,14,15,16,17}; // по умолчанию 9-17
        ImGuiX::Widgets::HoursSelectorConfig hs_cfg{};                          // Конфиг выбора часов

        // ------------------ 5) Время и смещение ------------------
        int      time_of_day_sec = 18 * 3600; // 18:00:00
        int64_t  tz_offset_sec   = 2 * 3600;  // +02:00:00
        bool     has_dst_out     = false;     // DST наблюдается (для выбора TZ)
        int      tz_index_io     = 0;         // Индекс текущей TZ

        ImGuiX::Widgets::TimePickerConfig       tp_cfg{}; // Конфиг выбора времени
        ImGuiX::Widgets::TimeOffsetPickerConfig to_cfg{}; // Конфиг выбора смещения

        // ------------------ 6) Дата ------------------
        int64_t                                  date_ts = 0; // Unix-эпоха (сек)
        ImGuiX::Widgets::DatePickerConfig        dp_cfg{};    // Конфиг выбора даты

        // ------------------ 7) Разное ------------------
        ImGuiX::Widgets::LoadingSpinnerConfig sp_cfg{}; // Конфиг спиннера
        std::vector<std::string>              names{"Alice", "Bob"};
        std::vector<int>                      numbers{1, 2, 3};

        std::array<ImGuiX::Widgets::LogViewerLevel, 6> log_levels{{
            {0, "TRACE"}, {1, "DEBUG"}, {2, "INFO"},
            {3, "WARN"}, {4, "ERROR"}, {5, "FATAL"}}};
        std::vector<ImGuiX::Widgets::LogViewerEntry> log_entries;
        ImGuiX::Widgets::LogViewerState log_state{2};
        
#       ifdef IMGUIX_ENABLE_IMPLOT
        // ------------------ 8) OHLC Bars Plot (demo) ------------------
        int  tf_index = 0;            // 0=M1, 1=M30, 2=H1
        int  bars_count = 300;        // сколько баров генерить
        unsigned rng_seed = 42;       // зерно генератора
        double start_price = 100.0;   // базовая цена
        // Данные для трёх ТФ (время — миллисекунды)
        std::vector<ImGuiX::Widgets::OhlcvBar> bars_m1;
        std::vector<ImGuiX::Widgets::OhlcvBar> bars_m30;
        std::vector<ImGuiX::Widgets::OhlcvBar> bars_h1;

        ImGuiX::Widgets::OHLCChartConfig ohlc_cfg{}; // конфиг виджета графика
#       endif

        // ------------------ Конструктор: дефолты ------------------
        WidgetsDemoState() {
            // --- Auth panel defaults ---
            auth_cfg.header                = "Login";
            auth_cfg.hint_email            = "email";
            auth_cfg.hint_password         = "password";
            auth_cfg.hint_host             = "host";
            auth_cfg.connected_label       = "connected";
            auth_cfg.connect_label         = "connect";
            auth_cfg.show_host             = true;
            auth_cfg.show_connection_state = true;
            auth_cfg.validate_email        = true;
            auth_cfg.init                  = true;
            auth_cfg.connected             = false;

            // Виртуальная клавиатура для полей логина
            auth_cfg.vk_host               = true;
            auth_cfg.vk_email              = true;
            auth_cfg.vk_password           = true;

            auth_cfg.vk_cfg.submit_on_enter     = true;
            auth_cfg.vk_cfg.capture_hw_enter    = true;    // захват физического Enter
            auth_cfg.vk_cfg.shift_enter_newline = true;    // Shift+Enter = перенос строки
            auth_cfg.vk_cfg.border              = false;
            auth_cfg.vk_cfg.on_submit = [](const std::string& /*text*/) {
                // Пользовательский submit (опционально)
            };

            // Кнопка "connect" (демо: просто переключает флаг)
            auth_cfg.on_connect = [this]() {
                auth_cfg.connected = !auth_cfg.connected;
            };

            // --- Auth data defaults ---
            auth_data.email    = "guest@example.com";
            auth_data.password = "";
            auth_data.host     = "demo.local";

            // --- VK config defaults ---
            kcfg.size               = ImVec2(0, 0); // авторазмер
            kcfg.locales            = {
                "en", "ru", "es", "pt-BR", "id",
                "uk", "be", "pt", "de", "fr", "it", "pl", "tr",
                "vi", "th", "ar", "hi", "ur"
            };
            kcfg.start_locale_index = 0;
            kcfg.show_locale_combo  = true;
            kcfg.key_size           = ImVec2(26, 26);
            kcfg.spacing            = 4.0f;
            kcfg.show_top_preview   = false;

            // --- Domain selector defaults ---
            dom_cfg.header         = "Server";
            dom_cfg.hint_domain    = "domain";
            dom_cfg.custom_text    = "Custom";
            dom_cfg.default_domain = auth_data.host; // при переходе к "Custom" сохраняем текущий
            dom_cfg.show_help      = true;
            dom_cfg.help_text      = "Choose predefined server or type custom host.";
            dom_cfg.domains        = { "demo.local", "test.example.com" };

            // --- Proxy defaults ---
            proxy.use_proxy = false;
            proxy.ip        = "127.0.0.1";
            proxy.port      = 8080;
            proxy.type      = ImGuiX::Widgets::ProxyType::HTTP;

            proxy_cfg.header          = "Proxy";
            proxy_cfg.label_use_proxy = "use proxy";
            proxy_cfg.button_check    = "check proxy";
            proxy_cfg.label_checked   = "checked";
            proxy_cfg.show_type       = true;
            proxy_cfg.show_check      = true;
            proxy_cfg.on_check = [this]() {
                // демо-поведение: отмечаем как "проверено" и чередуем успех/провал
                proxy_cfg.checked  = true;
                proxy_cfg.check_ok = !proxy_cfg.check_ok;
            };

            // --- Hours selector defaults ---
            hs_cfg.label       = "Hours";
            hs_cfg.empty_hint  = "none";
            // hs_cfg.popup_size  = ImVec2(260, 110);
            // hs_cfg.cell_size   = ImVec2(18, 18);
            // hs_cfg.rows        = 3;
            // hs_cfg.cols        = 8;
            // hs_cfg.combo_width = 260.0f;

            // --- Time pickers defaults ---
            tp_cfg.label = "Time of day";
            tp_cfg.desc  = "HH:MM:SS";

            to_cfg.label    = "GMT offset";
            to_cfg.desc     = "±HH:MM:SS";
            to_cfg.show_gmt = true;

            for (std::uint64_t index = 0; index < 48; ++index) {
                const int rank = static_cast<int>(index % log_levels.size());
                std::string message = "Synthetic log entry " + std::to_string(index);
                if (index % 7 == 0) {
                    message +=
                        " with a deliberately long message that wraps across the message column";
                }
                log_entries.push_back({
                    index + 1,
                    rank,
                    std::string("2026-10-07T12:00:") + (index < 10 ? "0" : "") +
                        std::to_string(index) + ".000Z",
                    log_levels[rank].label,
                    std::move(message)});
            }

            // dp_cfg (конфиг даты) можно настроить в UI-секции Date Picker
        }
    };

    WidgetsDemoState m_state{};

#   ifdef IMGUIX_DEMO
    void demoTheme() {
        if (ImGui::CollapsingHeader("Theme", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGuiX::Widgets::ThemePicker("demo.theme", this);
        }
    }

    void demoAuthorization() {
        if (ImGui::CollapsingHeader("Authorization", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGuiX::Widgets::DemoAuthPanel(this);
            ImGuiX::Widgets::DemoInputTextWithIconToggle();
        }

        if (ImGui::CollapsingHeader("Validated Inputs")) {
            ImGuiX::Widgets::DemoValidatedPasswordInputs();
        }

        if (ImGui::CollapsingHeader("Status Info")) {
            ImGui::Text("Connection: %s", m_state.auth_cfg.connected ? "connected" : "disconnected");
            ImGui::Text("Email Valid: %s", m_state.auth_data.email_valid ? "yes" : "no");
            ImGui::Text("API Key: %s", m_state.auth_data.api_key.c_str());
            ImGui::Text("API Secret: %s", m_state.auth_data.api_secret.c_str());
        }

        if (ImGui::CollapsingHeader("Virtual Keyboards")) {
            ImGuiX::Widgets::DemoVirtualKeyboard();
        }

        if (ImGui::CollapsingHeader("Domain Selector")) {
            ImGuiX::Widgets::DemoDomainSelector();
        }

        if (ImGui::CollapsingHeader("JS Panel")) {
            ImGuiX::Widgets::DemoAuthJsPanel();
        }

        if (ImGui::CollapsingHeader("Proxy Settings")) {
            if (ImGuiX::Widgets::ProxyPanel("proxy.panel", m_state.proxy_cfg, m_state.proxy)) {
                // применить настройки к сетевому слою при необходимости
            }
        }
    }

    void demoTime() {
        if (ImGui::CollapsingHeader("Hours Selector")) {
            ImGuiX::Widgets::DemoHoursSelector();
        }

        if (ImGui::CollapsingHeader("WeekDays Selector")) {
            ImGuiX::Widgets::DemoDaysOfWeekSelector();
        }

        if (ImGui::CollapsingHeader("Time & Offset")) {
            ImGuiX::Widgets::DemoTimePicker();
        }

        if (ImGui::CollapsingHeader("Date Picker")) {
            ImGuiX::Widgets::DemoDatePicker();
        }

        if (ImGui::CollapsingHeader("Expiry Picker")) {
            ImGuiX::Widgets::DemoExpiryPicker();
        }
    }

    void demoMisc() {
        if (ImGui::CollapsingHeader("Markers")) {
            ImGuiX::Widgets::DemoMarkers();
        }

        if (ImGui::CollapsingHeader("Loading Spinner")) {
            ImGuiX::Widgets::DemoLoadingSpinner(m_state.sp_cfg);
        }

        if (ImGui::CollapsingHeader("Text Centering")) {
            ImGuiX::Widgets::DemoTextCenter();
        }

        if (ImGui::CollapsingHeader("List Editors")) {
            ImGuiX::Widgets::DemoListEditor();
        }

        if (ImGui::CollapsingHeader("Icon Combo")) {
            static int sel_idx = 0;
            const char* items[] = {u8"One", u8"Two", u8"Three"};
            const char* icons[] = {u8"\ue430", u8"\ue51c", u8"\ue885"};
            if (ImGuiX::Widgets::BeginIconCombo("demo icon combo", items[sel_idx], icons[sel_idx])) {
                for (int i = 0; i < IM_ARRAYSIZE(items); ++i) {
                    bool selected = (i == sel_idx);
                    if (ImGui::Selectable(items[i], selected)) {
                        sel_idx = i;
                    }
                }
                ImGuiX::Widgets::EndIconCombo();
            }
        }
    }

    void demoNotifications() {
        if (ImGui::CollapsingHeader("Notifications", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGuiX::Widgets::DemoNotifications(this);
        }
    }

    void demoLogViewer() {
        if (ImGui::CollapsingHeader("Log Viewer", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGuiX::Widgets::LogViewerConfig config;
            config.size = ImVec2(0.0f, 360.0f);
            config.levels = m_state.log_levels.data();
            config.level_count = m_state.log_levels.size();
            config.actions.on_refresh = []() {};
            config.actions.on_clear = []() {};
            config.actions.on_open_folder = []() {};
            ImGuiX::Widgets::LogViewer(
                "demo.log_viewer", m_state.log_entries.data(), m_state.log_entries.size(),
                m_state.log_state, config);
        }
    }

#   ifdef IMGUIX_ENABLE_IMPLOT
    void demoPlot() {
        if (ImGui::CollapsingHeader(u8"OHLC Bars / Plot")) {
            static const int TFs[] = {60, 30*60, 60*60};
            static const char* TF_NAMES[] = {"M1", "M30", "H1"};

            ImGui::PushItemWidth(160);
            ImGui::Combo("Timeframe", &m_state.tf_index, TF_NAMES, IM_ARRAYSIZE(TF_NAMES));
            ImGui::SameLine();
            ImGui::TextDisabled("(%d)", TFs[m_state.tf_index]);
            ImGui::PopItemWidth();

            ImGui::SliderInt("Bars", &m_state.bars_count, 50, 2000);
            double min_price = 1.0, max_price = 10000.0;
            ImGui::SliderScalar("Start Price", ImGuiDataType_Double, &m_state.start_price, &min_price, &max_price, "%.2f");
            ImGui::InputScalar("Seed", ImGuiDataType_U32, &m_state.rng_seed);

            if (ImGui::Button("Regenerate")) {
                const std::uint64_t start_time = 1700000000ULL;
                GenerateBars(m_state.bars_m1,  m_state.bars_count, TFs[0], start_time, m_state.start_price, m_state.rng_seed);
                GenerateBars(m_state.bars_m30, m_state.bars_count, TFs[1], start_time, m_state.start_price, m_state.rng_seed + 1);
                GenerateBars(m_state.bars_h1,  m_state.bars_count, TFs[2], start_time, m_state.start_price, m_state.rng_seed + 2);
            }
            ImGui::SameLine();
            if (ImGui::Button("Shuffle Seed")) {
                m_state.rng_seed = (m_state.rng_seed * 1664525u + 1013904223u);
            }

            const int tf_sec = TFs[m_state.tf_index];
            const std::vector<ImGuiX::Widgets::OhlcvBar>& bars =
                (m_state.tf_index == 0 ? m_state.bars_m1
                 : m_state.tf_index == 1 ? m_state.bars_m30
                                         : m_state.bars_h1);

            std::string title = std::string("Bars (") + TF_NAMES[m_state.tf_index] + ")";

            ImGuiX::Widgets::PlotOHLCChart<ImGuiX::Widgets::OhlcvBar>(title.c_str(), bars, m_state.ohlc_cfg);
        }
        if (ImGui::CollapsingHeader(u8"Metrics / Plot")) {
            ImGuiX::Widgets::DemoMetricsPlot();
            ImGuiX::Widgets::DemoMetricsPlotSet();
            ImGuiX::Widgets::DemoDualMetricBarsPlot();
            ImGuiX::Widgets::DemoDualMetricBarsPlotSet();
        }
    }
#   endif

    void drawWidgetsDemo() {
        ImGui::SeparatorText("Widgets Demo");
        demoTheme();
#       ifdef IMGUIX_ENABLE_IMPLOT
        demoPlot();
#       endif
        demoAuthorization();
        demoTime();
        demoMisc();
        demoNotifications();
        demoLogViewer();
    }
#   endif

    void postLanguageChange(const std::string& lang, bool apply_to_all = true) {
        notify(ImGuiX::Events::LangChangeEvent(lang, apply_to_all, window().id()));
    }
};

// Окно-приложение: шрифты, размеры, инициализация контроллера
class ExampleWindow : public ImGuiX::WindowInstance {
public:
    ExampleWindow(int id,
               ImGuiX::ApplicationContext& app,
               std::string name)
        : WindowInstance(id, app, std::move(name)) {}

    void onInit() override {
        createController<WidgetsController>();
        create(id() == 0 ? 800 : 640, id() == 0 ? 600 : 480);
        setWindowIcon("data/resources/icons/icon.png");

        auto& tm = themeManager();
        ImGuiX::Themes::registerCorporateGreyTheme(tm);
        ImGuiX::Themes::registerDarkCharcoalTheme(tm);
        ImGuiX::Themes::registerDarkGraphiteTheme(tm);
        ImGuiX::Themes::registerDarkTealTheme(tm);
        ImGuiX::Themes::registerDeepDarkTheme(tm);
        ImGuiX::Themes::registerGoldBlackTheme(tm);
        ImGuiX::Themes::registerGreenBlueTheme(tm);
        ImGuiX::Themes::registerLightBlueTheme(tm);
        ImGuiX::Themes::registerLightGreenTheme(tm);
        ImGuiX::Themes::registerOSXTheme(tm);
        ImGuiX::Themes::registerPearlLightTheme(tm);
        ImGuiX::Themes::registerSlateDarkTheme(tm);
        ImGuiX::Themes::registerVisualStudioDarkTheme(tm);
        ImGuiX::Themes::registerY2KTheme(tm);
        ImGuiX::Themes::registerCyberY2KTheme(tm);
        ImGuiX::Themes::registerCyberpunkUITheme(tm);
        ImGuiX::Themes::registerNightOwlTheme(tm);
        ImGuiX::Themes::registerNordTheme(tm);
        ImGuiX::Themes::registerTokyoNightStormTheme(tm);
        ImGuiX::Themes::registerTokyoNightTheme(tm);
        ImGuiX::Widgets::ApplyStoredTheme(this);

        // Шрифты (ручная сборка)
        fontsBeginManual();
        fontsSetRangesPreset("Default+Cyrillic+Vietnamese+Punct+PUA+EmojiTGCore+LatinExtA");
        fontsAddBody({ "NotoSans-Regular.ttf", 18.0f });
        fontsAddMerge(ImGuiX::Fonts::FontRole::Icons, { "MaterialIcons-Regular.ttf", 18.0f, 3.0f, true });
        fontsAddMerge(ImGuiX::Fonts::FontRole::Emoji, { "NotoSansSymbols2-Regular.ttf", 18.0f, 0.0f, true });
        fontsAddMerge(ImGuiX::Fonts::FontRole::Emoji, { "NotoEmoji-Regular.ttf", 18.0f, 0.0f, true });
        fontsAddHeadline(ImGuiX::Fonts::FontRole::H1, { "NotoSans-Bold.ttf", 24.0f });
        fontsBuildNow();
    }
};

int main() {
#   ifdef IMGUIX_ENABLE_IMPLOT
    std::cout << "IMGUIX_ENABLE_IMPLOT" << std::endl;
#   endif
#   ifdef IMGUI_USE_WCHAR32
    std::cout << "IMGUI_USE_WCHAR32" << std::endl;
#   endif
    ImGuiX::Application app;
    app.createWindow<ExampleWindow>("Main Window");
    app.run();
    return 0;
}
