#pragma once
#ifndef IMGUIX_CORE_NOTIFY_NOTIFICATION_HPP_INCLUDED
#define IMGUIX_CORE_NOTIFY_NOTIFICATION_HPP_INCLUDED

/// \file Notification.hpp
/// \brief Toast notification entity (data + timing helpers).

#include <string>
#include <chrono>
#include <functional>
#include <cstdarg>
#include <optional>

#include <imguix/utils/vformat.hpp> // ImGuiX::Utils::vformat_va / vformat

#include "NotificationTypes.hpp"

namespace ImGuiX::Notify {

    /// \brief Toast notification entity.
    class Notification {
    public:
        /// \brief Construct empty toast.
        /// \param type Notification type.
        /// \param dismiss_ms Auto-dismiss delay in milliseconds.
        explicit Notification(Type type, int dismiss_ms = 0)
            : m_flags(0), m_type(type), m_dismiss_ms(dismiss_ms),
              m_created(std::chrono::steady_clock::now()) {}

        /// \brief Construct with printf-style content.
        /// \param type Notification type.
        /// \param fmt Format string.
        /// \param ... Format arguments.
        Notification(Type type, const char* fmt, ...) : Notification(type) {
            va_list ap; va_start(ap, fmt);
            m_content = ImGuiX::Utils::vformat_va(fmt, ap);
            va_end(ap);
        }

        /// \brief Construct with printf-style content and dismiss time.
        /// \param type Notification type.
        /// \param dismiss_ms Auto-dismiss delay in milliseconds.
        /// \param fmt Format string.
        /// \param ... Format arguments.
        Notification(Type type, int dismiss_ms, const char* fmt, ...) : Notification(type, dismiss_ms) {
            va_list ap; va_start(ap, fmt);
            m_content = ImGuiX::Utils::vformat_va(fmt, ap);
            va_end(ap);
        }

        /// \brief Set title (printf-style).
        /// \param fmt Format string.
        /// \param ... Format arguments.
        void setTitle(const char* fmt, ...) {
            va_list ap; va_start(ap, fmt);
            m_title = ImGuiX::Utils::vformat_va(fmt, ap);
            va_end(ap);
        }

        /// \brief Set content (printf-style).
        /// \param fmt Format string.
        /// \param ... Format arguments.
        void setContent(const char* fmt, ...) {
            va_list ap; va_start(ap, fmt);
            m_content = ImGuiX::Utils::vformat_va(fmt, ap);
            va_end(ap);
        }

        /// \brief Set button label (printf-style).
        /// \param fmt Format string.
        /// \param ... Format arguments.
        void setButtonLabel(const char* fmt, ...) {
            va_list ap; va_start(ap, fmt);
            m_button_label = ImGuiX::Utils::vformat_va(fmt, ap);
            va_end(ap);
        }

        /// \brief Set notification type.
        /// \param t New type.
        void setType(Type t) { m_type = t; }
        /// \brief Override the default color for this notification.
        /// \param color Custom RGBA color.
        void setColor(const ImVec4& color) { m_color_override = color; }
        /// \brief Restore the color associated with the notification type.
        void clearColorOverride() { m_color_override.reset(); }
        /// \brief Check whether this notification has an explicit color.
        bool hasColorOverride() const noexcept { return m_color_override.has_value(); }
        /// \brief Set window flags.
        /// \param flags Window flags to merge.
        void setWindowFlags(ImGuiWindowFlags flags) { m_flags = flags; }
        /// \brief Set button callback.
        /// \param fn Callback executed on button press.
        void setOnButtonPress(std::function<void()> fn) { m_on_button = std::move(fn); }

        /// \brief Get title.
        /// \return Title C-string.
        const char* title() const { return m_title.c_str(); }
        /// \brief Get content.
        /// \return Content C-string.
        const char* content() const { return m_content.c_str(); }
        /// \brief Get button label.
        /// \return Button label C-string.
        const char* buttonLabel() const { return m_button_label.c_str(); }
        /// \brief Get button callback.
        /// \return Callback executed on button press.
        const std::function<void()>& onButtonPress() const { return m_on_button; }
        /// \brief Get notification type.
        /// \return Notification type.
        Type type() const { return m_type; }

        /// \brief Default title inferred from type if title is empty.
        /// \return Title string or null.
        const char* defaultTitle() const {
            if (!m_title.empty()) return m_title.c_str();
            switch (m_type) {
                case Type::Success: return "Success";
                case Type::Warning: return "Warning";
                case Type::Error:   return "Error";
                case Type::Info:    return "Info";
                default:            return nullptr;
            }
        }

        /// \brief Color inferred from type (uses IMGUIX_NOTIFY_COLOR_*).
        /// \return RGBA color.
        ImVec4 color() const {
            if (m_color_override.has_value()) {
                return *m_color_override;
            }
            switch (m_type) {
                case Type::Success: return IMGUIX_NOTIFY_COLOR_SUCCESS;
                case Type::Warning: return IMGUIX_NOTIFY_COLOR_WARNING;
                case Type::Error:   return IMGUIX_NOTIFY_COLOR_ERROR;
                case Type::Info:    return IMGUIX_NOTIFY_COLOR_INFO;
                default:            return IMGUIX_NOTIFY_COLOR_DEFAULT;
            }
        }

        /// \brief Phase based on elapsed time and config.
        /// \param cfg Notification configuration.
        /// \return Current phase.
        Phase phase(const Config& cfg) const {
            using namespace std::chrono;
            const auto elapsed = duration_cast<milliseconds>(steady_clock::now() - m_created).count();
            const int dismiss_ms = m_dismiss_ms > 0 ? m_dismiss_ms : cfg.default_dismiss_ms;
            if (elapsed > cfg.fade_in_out_ms + dismiss_ms + cfg.fade_in_out_ms) return Phase::Expired;
            if (elapsed > cfg.fade_in_out_ms + dismiss_ms)                      return Phase::FadeOut;
            if (elapsed > cfg.fade_in_out_ms)                                   return Phase::Wait;
            return Phase::FadeIn;
        }
        
        /// \brief Phase using explicit dismiss time.
        /// \param cfg Notification configuration.
        /// \param effective_dismiss_ms Effective dismiss time in milliseconds.
        /// \return Current phase.
        Phase phase(const Config& cfg, int effective_dismiss_ms) const {
            using namespace std::chrono;
            const auto elapsed = elapsedMs();
            //const auto elapsed = duration_cast<milliseconds>(steady_clock::now() - m_created).count();
            if (elapsed > cfg.fade_in_out_ms + effective_dismiss_ms + cfg.fade_in_out_ms) return Phase::Expired;
            if (elapsed > cfg.fade_in_out_ms + effective_dismiss_ms)                      return Phase::FadeOut;
            if (elapsed > cfg.fade_in_out_ms)                                             return Phase::Wait;
            return Phase::FadeIn;
        }

        /// \brief Fade factor 0..opacity according to phase.
        /// \param cfg Notification configuration.
        /// \return Fade multiplier.
        float fadePercent(const Config& cfg) const {
            using namespace std::chrono;
            const float elapsed = static_cast<float>(duration_cast<milliseconds>(steady_clock::now() - m_created).count());
            const Phase ph = phase(cfg);
            const int dismiss_ms = m_dismiss_ms > 0 ? m_dismiss_ms : cfg.default_dismiss_ms;
            if (ph == Phase::FadeIn)  return (elapsed / cfg.fade_in_out_ms) * cfg.opacity;
            if (ph == Phase::FadeOut) return (1.0f - (elapsed - cfg.fade_in_out_ms - dismiss_ms) / cfg.fade_in_out_ms) * cfg.opacity;
            return cfg.opacity;
        }
        
        /// \brief Fade factor with explicit dismiss time.
        /// \param cfg Notification configuration.
        /// \param effective_dismiss_ms Effective dismiss time in milliseconds.
        /// \return Fade multiplier.
        float fadePercent(const Config& cfg, int effective_dismiss_ms) const {
            using namespace std::chrono;
            const float elapsed = static_cast<float>(duration_cast<milliseconds>(steady_clock::now() - m_created).count());
            const Phase ph = phase(cfg, effective_dismiss_ms);
            if (ph == Phase::FadeIn)  return (elapsed / cfg.fade_in_out_ms) * cfg.opacity;
            if (ph == Phase::FadeOut) return (1.0f - (elapsed - cfg.fade_in_out_ms - effective_dismiss_ms) / cfg.fade_in_out_ms) * cfg.opacity;
            return cfg.opacity;
        }
        
        /// \brief Update pause state when hovered.
        /// \param hovered True if notification is hovered.
        inline void updateHover(bool hovered) {
            using clock = std::chrono::steady_clock;
            const auto now = clock::now();
            if (hovered) {
                if (!m_paused) { m_paused = true; m_pause_begin = now; }
            } else {
                if (m_paused) { m_paused = false; m_pause_accum +=
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - m_pause_begin); }
            }
        }

        /// \brief Effective window flags (instance flags override base config; tooltip layer optional).
        /// \param cfg Notification configuration.
        /// \return Combined window flags.
        ImGuiWindowFlags windowFlags(const Config& cfg) const {
            ImGuiWindowFlags flags = m_flags ? m_flags : cfg.base_window_flags;
            if (cfg.use_tooltip_layer) flags |= ImGuiWindowFlags_Tooltip;
            return flags;
        }

        /// \brief User-specified dismiss override.
        /// \return Delay in milliseconds.
        int userDismissMs() const noexcept { return m_dismiss_ms; }

        /// \brief Content string length.
        /// \return Number of characters.
        int contentLength() const noexcept { return static_cast<int>(m_content.size()); }
        
        /// \brief Elapsed time excluding paused intervals.
        /// \return Milliseconds since creation.
        inline std::int64_t elapsedMs() const {
            using clock = std::chrono::steady_clock;
            using ms    = std::chrono::milliseconds;
            const auto now = clock::now();
            ms paused = m_pause_accum;
            if (m_paused) paused += std::chrono::duration_cast<ms>(now - m_pause_begin);
            return std::chrono::duration_cast<ms>(now - m_created).count() - paused.count();
        }

    private:
        ImGuiWindowFlags m_flags = 0;                 ///< 0 => use Config::base_window_flags
        Type m_type = Type::None;
        std::optional<ImVec4> m_color_override;
        std::string m_title;
        std::string m_content;
        int m_dismiss_ms = 0;
        std::chrono::steady_clock::time_point m_created;
        std::function<void()> m_on_button;
        std::string m_button_label;
        
        bool m_paused = false;
        std::chrono::steady_clock::time_point m_pause_begin{};
        std::chrono::milliseconds m_pause_accum{0};
    };

} // namespace ImGuiX::Notify

#endif // IMGUIX_CORE_NOTIFY_NOTIFICATION_HPP_INCLUDED
