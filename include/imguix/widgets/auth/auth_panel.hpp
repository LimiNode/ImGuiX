#pragma once
#ifndef _IMGUIX_WIDGETS_AUTH_PANEL_HPP_INCLUDED
#define _IMGUIX_WIDGETS_AUTH_PANEL_HPP_INCLUDED

/// \file auth_panel.hpp
/// \brief Login/auth panel: email + password (+optional host), connection state and action.

#include <imgui.h>
#include <string>
#include <functional>
#include <regex>
#include <cstring>
#include <algorithm>
#include <type_traits>

#include <imguix/config/fonts.hpp>
#include <imguix/widgets/input/validated_password_input.hpp>
#include <imguix/config/colors.hpp>
#include <imguix/core/controller/Controller.hpp>

namespace ImGuiX::Widgets {

    /// \brief Bitmask result for AuthPanel events.
    enum class AuthPanelResult : unsigned {
        None             = 0u,
        EmailChanged     = 1u << 0,
        PasswordChanged  = 1u << 1,
        HostChanged      = 1u << 2,
        TokenChanged     = 1u << 3,
        ApiKeyChanged    = 1u << 4,
        ApiSecretChanged = 1u << 5,
        ConnectClicked   = 1u << 6
    };

    /// \brief Combine two result masks.
    /// \param a Left operand.
    /// \param b Right operand.
    /// \return Bitwise OR of a and b.
    inline AuthPanelResult operator|(AuthPanelResult a, AuthPanelResult b) {
        return static_cast<AuthPanelResult>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
    }

    /// \brief Intersect two result masks.
    /// \param a Left operand.
    /// \param b Right operand.
    /// \return Bitwise AND of a and b.
    inline AuthPanelResult operator&(AuthPanelResult a, AuthPanelResult b) {
        return static_cast<AuthPanelResult>(static_cast<unsigned>(a) & static_cast<unsigned>(b));
    }

    /// \brief Add flags from b to a.
    /// \param a Mask to modify.
    /// \param b Flags to add.
    /// \return Reference to a after modification.
    inline AuthPanelResult& operator|=(AuthPanelResult& a, AuthPanelResult b) { a = a | b; return a; }

    /// \brief Check if any flag is set.
    /// \param r Result mask.
    /// \return True if r is non-zero.
    inline bool Any(ImGuiX::Widgets::AuthPanelResult r) noexcept {
        using U = std::underlying_type_t<ImGuiX::Widgets::AuthPanelResult>;
        return static_cast<U>(r) != 0u;
    }

    /// \brief Check if r contains flag f.
    /// \param r Result mask.
    /// \param f Flag mask to test.
    /// \return True if r has all bits from f.
    inline bool Has(ImGuiX::Widgets::AuthPanelResult r,
                    ImGuiX::Widgets::AuthPanelResult f) noexcept {
        using U = std::underlying_type_t<ImGuiX::Widgets::AuthPanelResult>;
        return (static_cast<U>(r) & static_cast<U>(f)) != 0u;
    }

    /// \brief Configuration for AuthPanel.
    struct AuthPanelConfig {
        ImVec2 panel_size = ImVec2(0.0f, 0.0f); ///< x<=0: fill available width; y<=0: auto-computed
        bool   inputs_fill_width = true;        ///< make input fields fill panel width
        
        // Labels (now raw pointers; caller owns lifetime)
        const char* header          = u8"Authorization";
        const char* hint_email      = u8"email";
        const char* hint_password   = u8"password";
        const char* hint_host       = u8"host";
        const char* connected_label = u8"connected";
        const char* connect_label   = u8"connect";

        // Tokens / API keys (now raw pointers)
        const char* hint_token      = u8"token";
        const char* hint_api_key    = u8"api key (public)";
        const char* hint_api_secret = u8"api secret";

        // Options
        bool show_host             = false;
        bool show_email            = true;
        bool show_password         = true;
        bool show_token            = false;
        bool show_api_keys         = false; ///< public + secret
        bool mask_api_secret       = true;  ///< show secret as password
        bool show_header           = true;  ///< show header text
        bool show_separator        = true;  ///< show separator after header
        bool show_connect_button   = true;
        bool show_connect_separator= true;  ///< show separator before connection controls
        bool show_connection_state = true;

        // Validation toggles
        bool validate_email        = true;
        bool validate_password     = true;
        bool validate_host         = true;
        bool validate_token        = true;
        bool validate_api_key      = true;
        bool validate_api_secret   = true;

        // Virtual keyboard configs (shared across fields)
        VirtualKeyboardConfig      vk_cfg{};   ///< behavior/locale/etc of VK
        KeyboardToggleConfig       kb_cfg{};   ///< visuals of the VK trigger button
        
        // Inline password eye config
        PasswordToggleConfig toggle_cfg{};     ///< password eye config

        // Per-field VK enable switches
        bool vk_host        = false;
        bool vk_email       = false;
        bool vk_password    = false;
        bool vk_token       = false;
        bool vk_api_key     = false;
        bool vk_api_secret  = false;

        // Connection state visuals
        bool   init      = false;
        bool   connected = false;

        // Optional email state pill on the right
        bool        use_email_state   = false;
        bool        email_state       = false;
        ImVec4      email_state_color = ImVec4(0,0,0,1);
        std::string email_state_label;

        // Callbacks
        std::function<void()> on_connect;

        // Regexes (basic)
        const char* email_regex      = u8R"(.+@.+\.\w+)";                    ///< 
        const char* password_regex   = u8R"(^\S+$)";                         ///< 
        const char* host_regex       = u8R"(^[A-Za-z0-9.\-:]+$)";            ///< host:port / domain
        const char* token_regex      = u8R"(^(?:[A-Za-z0-9._\-]{16,256})$)"; ///< 
        const char* api_key_regex    = u8R"(^(?:[A-Za-z0-9._\-]{16,256})$)"; ///< 
        const char* api_secret_regex = u8R"(^(?:[A-Za-z0-9._\-]{16,256})$)"; ///< 

        ImVec4      error_color      = IMGUIX_COLOR_ERROR;
    };
    
    /// \brief All auth-related fields passed in/out by reference.
    struct AuthData {
        std::string email;       ///< in/out
        std::string password;    ///< in/out
        std::string host;        ///< in/out, used if cfg.show_host
        std::string token;       ///< in/out, used if cfg.show_token
        std::string api_key;     ///< in/out, used if cfg.show_api_keys
        std::string api_secret;  ///< in/out, used if cfg.show_api_keys

        bool email_valid     = true;  ///< out (set by widget if cfg.validate_email)
        bool password_valid  = true;  ///< out
        bool host_valid      = true;  ///< out
        bool token_valid     = true;  ///< out
        bool api_key_valid   = true;  ///< out
        bool api_secret_valid= true;  ///< out
    };

    /// \brief Renders login form. Optionally includes host input.
    /// \param id Unique widget ID.
    /// \param cfg Panel configuration.
    /// \param data All input/output fields and validity flags (set inside).
    /// \return Bitmask of events happened this frame.
    AuthPanelResult AuthPanel(
            const char* id,
            AuthPanelConfig& cfg,
            AuthData& data
        );

#   ifdef IMGUIX_DEMO
    /// \brief Render demo for AuthPanel widget.
    /// \param ctrl Controller to access options storage.
    inline void DemoAuthPanel(ImGuiX::Controller* ctrl) {
        static AuthPanelConfig auth_cfg{};
        static AuthData auth_data{};
        static bool is_once = false;
        
        if (!is_once) {
            is_once = true;
            auth_data.host     = ctrl->options().getStrOr("host", "demo.local");
            auth_data.email    = ctrl->options().getStrOr("email", "guest@example.com");
            auth_data.password = ctrl->options().getStrOr("password", "");
        }
        
        auth_cfg.vk_email            = true;
        auth_cfg.vk_password         = true;
        AuthPanel("login.panel", auth_cfg, auth_data);

        AuthPanelConfig cfg_api{};
        cfg_api.header              = u8"Authorization (API key)";
        cfg_api.show_api_keys       = true;
        cfg_api.vk_email            = true;
        cfg_api.vk_password         = true;
        cfg_api.vk_api_key          = true;
        cfg_api.vk_api_secret       = true;
        cfg_api.show_connect_button = false;
        const auto res = AuthPanel("login.api", cfg_api, auth_data);
        using E = AuthPanelResult;
        if (ctrl) {
            if (Has(res, E::HostChanged) && auth_data.host_valid) {
                ctrl->options().setStr("host", auth_data.host);
            }
            if (Has(res, E::EmailChanged) && auth_data.email_valid) {
                ctrl->options().setStr("email", auth_data.email);
            }
            if (Has(res, E::PasswordChanged)) {
                ctrl->options().setStr("password", auth_data.password);
            }
        }
    }
#   endif

} // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#   include "auth_panel.ipp"
#endif

#endif // _IMGUIX_WIDGETS_AUTH_PANEL_HPP_INCLUDED
