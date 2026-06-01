#pragma once
#ifndef _IMGUIX_WIDGETS_PROXY_ENDPOINT_FIELDS_HPP_INCLUDED
#define _IMGUIX_WIDGETS_PROXY_ENDPOINT_FIELDS_HPP_INCLUDED

/// \file proxy_endpoint_fields.hpp
/// \brief Proxy endpoint fields: host, port, and protocol type.

#include <imgui.h>

#include <algorithm>
#include <functional>
#include <string>

#include <imguix/config/colors.hpp>
#include <imguix/widgets/input/validated_input.hpp>
#include <imguix/widgets/input/validated_password_input.hpp>
#include <imguix/widgets/auth/proxy_panel.hpp>

namespace ImGuiX::Widgets {

    /// \brief Mutable model of proxy endpoint settings.
    struct ProxyEndpointSettings {
        std::string ip;                             ///< host or IPv4 string
        int         port       = 0;                 ///< 0..65535
        ProxyType   type       = ProxyType::HTTP;   ///< proxy protocol type

        bool ip_valid          = true;              ///< out (set by widget if cfg.validate_ip)
        bool port_valid        = true;              ///< out
    };

    /// \brief UI configuration for ProxyEndpointFields.
    struct ProxyEndpointFieldsConfig {
        ImVec2 panel_size             = ImVec2(0, 0); ///< 0->auto width/height
        bool   inputs_fill_width      = true;         ///< make input fields fill panel width
        bool   border                 = true;         ///< draw panel border
        bool   show_header            = true;         ///< draw header and separator

        // Labels
        const char* header            = u8"Proxy settings";
        const char* hint_ip           = u8"ip";
        const char* hint_port         = u8"port";
        const char* button_check      = u8"check proxy";
        const char* label_checked     = u8"checked";

        // Options
        bool show_check               = true;         ///< show "check proxy" button + status
        bool checked                  = false;        ///< result of last check (visual only)
        bool check_ok                 = false;        ///< last check OK (green) vs fail (red)

        // Virtual keyboard configs (shared across fields)
        VirtualKeyboardConfig      vk_cfg{};          ///< behavior/locale/etc of VK
        KeyboardToggleConfig       kb_cfg{};          ///< visuals of the VK trigger button

        bool vk_ip                    = true;         ///< show virtual keyboard toggle for IP
        bool vk_port                  = true;         ///< show virtual keyboard toggle for port

        // Validation toggles
        bool validate_ip              = true;         ///< validate IP/host field
        bool validate_port            = true;         ///< validate port field

        const char* ip_regex          = u8R"(^\d{1,3}(\.\d{1,3}){3}$)";
        const char* port_regex        = u8R"(^[A-Za-z0-9.\-:]+$)";

        // Callbacks
        std::function<void()> on_check = nullptr;

        // Layout
        float field_width_host        = 0.0f;
        float field_width_port        = 0.0f;
        float field_width_type        = 0.0f;

        // Style
        ImVec4 error_color = IMGUIX_COLOR_ERROR;
    };

    /// \brief Draw proxy endpoint fields.
    /// \param id Unique widget identifier.
    /// \param cfg Field configuration.
    /// \param st Proxy endpoint model to edit.
    /// \return True if any settings changed this frame.
    bool ProxyEndpointFields(const char* id, ProxyEndpointFieldsConfig& cfg, ProxyEndpointSettings& st);

    /// \brief Draw proxy endpoint fields backed by full proxy settings.
    /// \param id Unique widget identifier.
    /// \param cfg Field configuration.
    /// \param st Full proxy settings model to edit.
    /// \return True if any endpoint settings changed this frame.
    bool ProxyEndpointFields(const char* id, ProxyEndpointFieldsConfig& cfg, ProxySettings& st);

} // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#   include "proxy_endpoint_fields.ipp"
#endif

#endif // _IMGUIX_WIDGETS_PROXY_ENDPOINT_FIELDS_HPP_INCLUDED
