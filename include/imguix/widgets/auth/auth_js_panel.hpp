#pragma once
#ifndef _IMGUIX_WIDGETS_AUTH_JS_PANEL_HPP_INCLUDED
#define _IMGUIX_WIDGETS_AUTH_JS_PANEL_HPP_INCLUDED

/// \file auth_js_panel.hpp
/// \brief Browser auth config panel: user-agent, accept-language, and DNT flag.
/// Uses InputTextWithVKValidated (validation + on-screen keyboard trigger).

#include <imguix/widgets/input/validated_input.hpp> // InputTextWithVKValidated, KeyboardToggleConfig, InputValidatePolicy
#include <imguix/config/colors.hpp>

namespace ImGuiX::Widgets {

    /// \brief Mutable JS auth settings.
    struct AuthJsSettings {
        std::string user_agent;        ///< User-Agent header
        std::string accept_language;   ///< Accept-Language header
        bool        dnt = false;       ///< Do Not Track flag
    };

    /// \brief UI configuration for AuthJsPanel.
    struct AuthJsPanelConfig {
        // Panel
        ImVec2      panel_size = ImVec2(0.0f, 0.0f); ///< x<=0: fill available width; y<=0: auto-computed
        bool        inputs_fill_width = true;        ///< make input fields fill panel width
        bool        border            = true;       ///< draw panel border
        bool        show_separator    = true;       ///< show separator after header

        // Labels / hints
        std::string header               = u8"JS Config";
        std::string hint_user_agent      = u8"User agent";
        std::string hint_accept_language = u8"Accept language";
        std::string label_dnt            = u8"Do Not Track";

        // Validation
        bool        validate_user_agent      = true;
        bool        validate_accept_language = true;
        ImGuiX::Widgets::InputValidatePolicy policy = ImGuiX::Widgets::InputValidatePolicy::OnTouch;
        ImVec4      error_color           = IMGUIX_COLOR_ERROR;

        // Pragmatic regexes (ECMAScript for std::regex)
        // UA: printable ASCII (space through ~), at least 1 char
        const char* user_agent_regex      = u8R"(^[ -~]+$)";
        // Accept-Language: allow tokens, subtags, q-factors; permissive char class
        const char* accept_language_regex = u8R"(^(?:[A-Za-z0-9,*_\-.;=,\s])+)$)";

        // VK trigger + keyboard configs
        ImGuiX::Widgets::KeyboardToggleConfig  kb_cfg{};
        ImGuiX::Widgets::VirtualKeyboardConfig vk_cfg{};

        // Enable VK per field
        bool vk_user_agent      = true;
        bool vk_accept_language = true;

        // Extra input flags applied to both fields (e.g., AutoSelectAll)
        ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_AutoSelectAll;
    };

    /// \brief Draw browser auth settings panel.
    /// \param id Unique widget identifier.
    /// \param cfg Panel configuration.
    /// \param st Settings model to edit.
    /// \return True if any field changed this frame.
    bool AuthJsPanel(const char* id, AuthJsPanelConfig& cfg, AuthJsSettings& st);

#   ifdef IMGUIX_DEMO
    /// \brief Render demo for AuthJsPanel widget.
    inline void DemoAuthJsPanel() {
        static AuthJsPanelConfig cfg{};
        static AuthJsSettings st{};
        AuthJsPanel("js.panel", cfg, st);
    }
#   endif

} // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#   include "auth_js_panel.ipp"
#endif

#endif // _IMGUIX_WIDGETS_AUTH_JS_PANEL_HPP_INCLUDED
