#pragma once
#ifndef _IMGUIX_WIDGETS_FORM_SECTION_HPP_INCLUDED
#define _IMGUIX_WIDGETS_FORM_SECTION_HPP_INCLUDED

/// \file form_section.hpp
/// \brief Neutral collapsible sections for form content.

#include <imgui.h>

namespace ImGuiX::Widgets {

    /// \brief Configuration for a neutral form-section header.
    struct FormSectionConfig {
        const char* title{nullptr};
        bool default_open{true};
        bool draw_separator{true};
        float header_height{0.0f};
        float indent{0.0f};
    };

    /// \brief Begin a neutral collapsible form section.
    /// \param id Stable state and widget identity.
    /// \param config Header title and layout policy.
    /// \return True when the section body should be submitted.
    bool BeginFormSection(const char* id, const FormSectionConfig& config = {});

    /// \brief Finish the most recently opened form section.
    void EndFormSection();

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#include "form_section.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_FORM_SECTION_HPP_INCLUDED
