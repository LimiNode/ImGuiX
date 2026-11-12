#pragma once
#ifndef _IMGUIX_WIDGETS_SECTION_CARD_HPP_INCLUDED
#define _IMGUIX_WIDGETS_SECTION_CARD_HPP_INCLUDED

/// \file section_card.hpp
/// \brief Content-aware card and page composition primitives.

#include <imguix/widgets/containers/rounded_panel.hpp>

#include <imgui.h>
#include <functional>

namespace ImGuiX::Widgets {

    /// \brief Height behavior for a section card.
    enum class SectionCardHeightPolicy {
        Content,
        Fixed,
        Constrained,
    };

    /// \brief Overflow behavior for a constrained card.
    enum class SectionCardScrollPolicy {
        Never,
        Vertical,
        Both,
    };

    /// \brief Configuration shared by reusable section cards.
    struct SectionCardConfig {
        const char* title{nullptr};
        ImFont* title_font{nullptr};
        ImVec2 size{0.0f, 0.0f};
        float max_height{0.0f};
        float title_gap{-1.0f};                  ///< Negative uses style ItemSpacing.y.
        std::function<void()> header_actions{};  ///< Optional ordinary ImGui widgets.
        float header_actions_width{0.0f};  ///< Known width; zero puts actions on a separate row.
        SectionCardHeightPolicy height_policy{SectionCardHeightPolicy::Content};
        SectionCardScrollPolicy scroll_policy{SectionCardScrollPolicy::Never};
        RoundedPanelConfig panel{};
    };

    /// \brief Begin a content-aware rounded section card.
    ///
    /// Content cards use RoundedPanel with native auto-resize Y. They measure
    /// offscreen content too, leaving the page surface as the vertical scroll
    /// owner. Geometry settles on the next frame after content/width changes.
    /// Content ignores size.y and scroll_policy. Fixed requires size.y > 0;
    /// Constrained requires max_height > 0 and promotes Never to Vertical so
    /// overflowing content remains reachable. Always pair with EndSectionCard,
    /// even when this function returns false. Labels are borrowed only for
    /// this call. Callbacks must balance their own ImGui scopes.
    /// \param id Stable identifier for the card instance.
    /// \param config Card title, sizing, overflow, and visual policy.
    /// \return True when the caller should submit the card body.
    bool BeginSectionCard(const char* id, const SectionCardConfig& config = {});

    /// \brief Finish the most recently opened section card.
    void EndSectionCard();

}  // namespace ImGuiX::Widgets

#ifdef IMGUIX_HEADER_ONLY
#include "section_card.ipp"
#endif

#endif  // _IMGUIX_WIDGETS_SECTION_CARD_HPP_INCLUDED
