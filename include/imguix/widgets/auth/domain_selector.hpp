#pragma once
#ifndef _IMGUIX_WIDGETS_DOMAIN_SELECTOR_HPP_INCLUDED
#define _IMGUIX_WIDGETS_DOMAIN_SELECTOR_HPP_INCLUDED

/// \file domain_selector.hpp
/// \brief Domain chooser widget (combo with "Custom" fallback or validated input with VK).

#include <imguix/widgets/input/validated_input.hpp> // InputTextWithVKValidated, KeyboardToggleConfig, InputValidatePolicy
#include <imguix/config/colors.hpp>
#include <imguix/widgets/misc/markers.hpp> // HelpMarker

namespace ImGuiX::Widgets {

    /// \brief Help marker appearance for DomainSelector.
    enum class DomainSelectorHelpStyle {
        Auto,        ///< Preserve legacy behavior: text for combo, icon for plain input.
        TextQuestion,///< Render "(?)" text with tooltip.
        HelpIcon     ///< Render ImGuiX HelpMarker.
    };

    /// \brief Configuration for DomainSelector.
    struct DomainSelectorConfig {
        // Panel
        ImVec2      panel_size = ImVec2(0.0f, 0.0f); ///< x<=0: fill available width; y<=0: auto-computed
        bool        inputs_fill_width = true;        ///< make input fields fill panel width
        bool        border            = true;       ///< draw panel border
        bool        show_separator    = true;       ///< show separator after header
        
        std::string header       = u8"Domain";
        std::string hint_domain  = u8"domain";
        std::string custom_text  = u8"Custom";
        std::string default_domain;
        std::string help_text;
        bool        show_help    = false;           ///< show help marker
        DomainSelectorHelpStyle help_style = DomainSelectorHelpStyle::Auto;
        const char* help_icon = IMGUIX_ICON_HELP;   ///< icon used by HelpIcon style

        /// \brief List of predefined domains. If empty, InputTextWithVKValidated is used directly.
        std::vector<std::string> domains;

        // Validation (ECMAScript regex for std::regex)
        bool        validate_domain = true;
        ImGuiX::Widgets::InputValidatePolicy policy = ImGuiX::Widgets::InputValidatePolicy::OnTouch;
        ImVec4      error_color = IMGUIX_COLOR_ERROR;

        // Pragmatic host/domain pattern (allows subdomains and optional :port)
        const char* domain_regex = u8R"(^[A-Za-z0-9.\-:]+$)";

        // VK trigger + keyboard configs
        bool vk_enabled = true;
        ImGuiX::Widgets::KeyboardToggleConfig  kb_cfg{};
        ImGuiX::Widgets::VirtualKeyboardConfig vk_cfg{};

        // Extra input flags
        ImGuiInputTextFlags input_flags = 0; // e.g. ImGuiInputTextFlags_AutoSelectAll
    };

    /// \brief Renders domain selection UI. If current host not in list, "Custom" path is used.
    /// \param label ImGui label (must be unique).
    /// \param cfg Configuration.
    /// \param host Host string to edit.
    /// \return True if value changed this frame.
    bool DomainSelector(
            const char* label,
            DomainSelectorConfig& cfg,
            std::string& host
        );

#   ifdef IMGUIX_DEMO
    /// \brief Render demo for DomainSelector widget.
    inline void DemoDomainSelector() {
        static DomainSelectorConfig cfg{};
        static std::string host;
        DomainSelector("domain.selector", cfg, host);
    }
#   endif

} // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#   include "domain_selector.ipp"
#endif

#endif // _IMGUIX_WIDGETS_DOMAIN_SELECTOR_HPP_INCLUDED
