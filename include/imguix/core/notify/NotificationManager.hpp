#pragma once
#ifndef _IMGUIX_CORE_NOTIFY_NOTIFICATION_MANAGER_HPP_INCLUDED
#define _IMGUIX_CORE_NOTIFY_NOTIFICATION_MANAGER_HPP_INCLUDED

/// \file NotificationManager.hpp
/// \brief Toast notification system for ImGuiX windows.
/// Based on imgui-notify by patrickcjk (https://github.com/patrickcjk/imgui-notify)
/// \author Original implementation by TyomaVader
/// \author Modified by NewYaroslav + refactor by Nick

#include <fmt/format.h>

#include "Notification.hpp"

namespace ImGuiX::Notify {

    /// \brief Manager that queues and renders notifications.
    class NotificationManager {
    public:
        NotificationManager() = default;

        /// \brief Access mutable config.
        /// \return Reference to configuration.
        Config&       config()       { return m_cfg; }
        /// \brief Access const config.
        /// \return Reference to configuration.
        const Config& config() const { return m_cfg; }

        /// \brief Fonts configuration.
        /// \return Reference to font config.
        FontConfig&       fonts()       { return m_fonts; }
        /// \brief Fonts configuration (const).
        /// \return Reference to font config.
        const FontConfig& fonts() const { return m_fonts; }

        /// \brief Icon configuration.
        /// \return Reference to icon config.
        IconConfig&       icons()       { return m_icons; }
        /// \brief Icon configuration (const).
        /// \return Reference to icon config.
        const IconConfig& icons() const { return m_icons; }

        /// \brief Access mutable local notification style.
        NotificationStyle& style() { return m_style; }
        /// \brief Access const local notification style.
        const NotificationStyle& style() const { return m_style; }

        /// \brief Queue notification.
        /// \param toast Notification to push.
        void push(Notification toast) { m_notifications.push_back(std::move(toast)); }
        /// \brief Remove all queued notifications.
        void clear() { m_notifications.clear(); }

        /// \brief Render all active notifications.
        void render();

        /// \brief Post notification using fmt with compile-time checked format.
        /// \tparam Args Format argument types.
        /// \param type Notification type.
        /// \param dismiss_ms Auto-dismiss delay in milliseconds.
        /// \param fmt_s Format string.
        /// \param args Format arguments.
        template<class... Args>
        void postFmt(Notify::Type type, int dismiss_ms,
                     fmt::format_string<Args...> fmt_s, Args&&... args) {
            Notify::Notification n(type, dismiss_ms);
            const auto text = fmt::format(fmt_s, std::forward<Args>(args)...);
            n.setContent("%s", text.c_str());
            push(std::move(n));
        }

        /// \brief Post notification using fmt for a runtime format string.
        /// \tparam Args Format argument types.
        /// \param type Notification type.
        /// \param dismiss_ms Auto-dismiss delay in milliseconds.
        /// \param fmt_rt Runtime format string.
        /// \param args Format arguments.
        template<class... Args>
        void postFmtRuntime(Notify::Type type, int dismiss_ms,
                            fmt::string_view fmt_rt, Args&&... args) {
            Notify::Notification n(type, dismiss_ms);
            const auto text = fmt::vformat(fmt::runtime(fmt_rt),
                                           fmt::make_format_args(std::forward<Args>(args)...));
            n.setContent("%s", text.c_str());
            push(std::move(n));
        }

        /// \brief Post notification with title and content via fmt.
        /// \tparam ArgsT Title argument types.
        /// \tparam ArgsC Content argument types.
        /// \param type Notification type.
        /// \param dismiss_ms Auto-dismiss delay in milliseconds.
        /// \param title_fmt Title format string.
        /// \param title_args Title format arguments.
        /// \param content_fmt Content format string.
        /// \param content_args Content format arguments.
        template<class... ArgsT, class... ArgsC>
        void postWithTitleFmt(Notify::Type type, int dismiss_ms,
                              fmt::format_string<ArgsT...> title_fmt,   ArgsT&&... title_args,
                              fmt::format_string<ArgsC...> content_fmt, ArgsC&&... content_args) {
            Notify::Notification n(type, dismiss_ms);
            const auto t = fmt::format(title_fmt,   std::forward<ArgsT>(title_args)...);
            const auto c = fmt::format(content_fmt, std::forward<ArgsC>(content_args)...);
            n.setTitle("%s", t.c_str());
            n.setContent("%s", c.c_str());
            push(std::move(n));
        }

        /// \brief Post notification with a button via fmt.
        /// \tparam ArgsT Title argument types.
        /// \tparam ArgsC Content argument types.
        /// \param type Notification type.
        /// \param dismiss_ms Auto-dismiss delay in milliseconds.
        /// \param button_label Button label.
        /// \param on_click Callback executed on button click.
        /// \param title_fmt Title format string.
        /// \param title_args Title format arguments.
        /// \param content_fmt Content format string.
        /// \param content_args Content format arguments.
        template<class... ArgsT, class... ArgsC>
        void postWithButtonFmt(Notify::Type type, int dismiss_ms,
                               const char* button_label, std::function<void()> on_click,
                               fmt::format_string<ArgsT...> title_fmt,   ArgsT&&... title_args,
                               fmt::format_string<ArgsC...> content_fmt, ArgsC&&... content_args) {
            Notify::Notification n(type, dismiss_ms);
            const auto t = fmt::format(title_fmt,   std::forward<ArgsT>(title_args)...);
            const auto c = fmt::format(content_fmt, std::forward<ArgsC>(content_args)...);
            n.setTitle("%s", t.c_str());
            n.setContent("%s", c.c_str());
            n.setOnButtonPress(std::move(on_click));
            n.setButtonLabel("%s", button_label);
            push(std::move(n));
        }

        /// \brief Post notification using classic printf-style format.
        /// \param type Notification type.
        /// \param dismiss_ms Auto-dismiss delay in milliseconds.
        /// \param fmt_s printf-like format string.
        /// \param ... Format arguments.
        inline void postf(Notify::Type type, int dismiss_ms, const char* fmt_s, ...) {
            if (!fmt_s) return;
            va_list args; va_start(args, fmt_s);
            std::string text = ImGuiX::Utils::vformat_va(fmt_s, args);
            va_end(args);
            Notify::Notification n(type, dismiss_ms);
            n.setContent("%s", text.c_str());
            push(std::move(n));
        }

    private:
        /// \brief Remove notification at index.
        /// \param index Position in queue.
        void remove(std::size_t index) {
            m_notifications.erase(m_notifications.begin() + static_cast<std::ptrdiff_t>(index));
        }
		
        /// \brief Compute dismiss time with policy and wrapped content.
        /// \param n Notification instance.
        /// \param cfg Configuration.
        /// \param wrap_width Wrap width in pixels.
        /// \param wrapped_lines Number of wrapped lines (>=1).
        /// \return Effective dismiss time in milliseconds.
        int computeEffectiveDismissMs(
                const Notification& n,
                const Config& cfg,
                float wrap_width,
                int wrapped_lines /*>=1*/) const;

        std::vector<Notification> m_notifications;
        Config     m_cfg{};
        IconConfig m_icons{};
        FontConfig m_fonts{};
        NotificationStyle m_style{};
    };

} // namespace ImGuiX::Notify

#ifdef IMGUIX_HEADER_ONLY
#   include "NotificationManager.ipp"
#endif

#endif // _IMGUIX_CORE_NOTIFY_NOTIFICATION_MANAGER_HPP_INCLUDED
