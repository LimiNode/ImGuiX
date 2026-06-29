#include <algorithm>

#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE void ToggleButton(const char* id, bool* state, const ToggleButtonConfig& config) {
        // Geometry
        const float default_h = ImGui::GetFrameHeight();
        const float h = config.size.y > 0.0f ? config.size.y :
            (config.height > 0.0f ? config.height : default_h);
        const float w = config.size.x > 0.0f ? config.size.x : h * 1.55f;
        const float r = h * 0.50f;
        const bool has_offset = config.offset.x != 0.0f || config.offset.y != 0.0f;

        const ImVec2 cursor_pos = ImGui::GetCursorPos();
        if (has_offset) {
            ImGui::SetCursorPos(ImVec2(cursor_pos.x + config.offset.x, cursor_pos.y + config.offset.y));
        }

        const ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // Click area
        ImGui::InvisibleButton(id, ImVec2(w, h));
        if (has_offset && config.restore_cursor_after_offset) {
            const ImVec2 cursor_after_item = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2(cursor_after_item.x - config.offset.x, cursor_after_item.y - config.offset.y));
        }
        if (ImGui::IsItemClicked() && state) *state = !*state;

        // --- Animation (public API) -----------------------------------------
        // Store per-widget animation progress in window storage.
        ImGuiStorage* st = ImGui::GetStateStorage();
        ImGui::PushID(id);
        const ImGuiID key_anim = ImGui::GetID("anim");
        ImGui::PopID();

        const bool is_on = state && *state;
        float anim = st->GetFloat(key_anim, (is_on ? 1.0f : 0.0f));
        const float target = (is_on ? 1.0f : 0.0f);

        // Linear approach to target with ~0.08s time to settle (like old kAnimSpeed)
        const float dt = ImGui::GetIO().DeltaTime;
        const float step = (dt > 0 ? dt / 0.08f : 1.0f); // 0.08s to travel 0->1
        if (target > anim) {
            anim = std::min(target, anim + step);
        } else if (target < anim) {
            anim = std::max(target, anim - step);
        }

        st->SetFloat(key_anim, anim);
        const float t = anim; // 0..1

        // Small helpers (no ImLerp/ImSaturate from internals)
        auto lerp4 = [](const ImVec4& a, const ImVec4& b, float k) {
            if (k < 0.0f) k = 0.0f; else if (k > 1.0f) k = 1.0f;
            return ImVec4(a.x + (b.x - a.x) * k,
                          a.y + (b.y - a.y) * k,
                          a.z + (b.z - a.z) * k,
                          a.w + (b.w - a.w) * k);
        };

        // Colors
        const bool hovered = ImGui::IsItemHovered();
        const ImU32 col_bg = ImGui::GetColorU32(
                hovered ?
                    lerp4(config.off_hovered_color, config.on_hovered_color, t) :
                    lerp4(config.off_color, config.on_color, t)
        );

        // Track background
        dl->AddRectFilled(
            pos,
            ImVec2(pos.x + w, pos.y + h),
            col_bg,
            h * 0.5f
        );

        // Knob
        const float x0 = pos.x + r;
        const float x1 = pos.x + (w - r);
        const float x  = x0 + (x1 - x0) * t;

        dl->AddCircleFilled(
            ImVec2(x, pos.y + r),
            std::max(0.0f, r - 1.5f),
            ImGui::GetColorU32(config.knob_color)
        );
    }

} // namespace ImGuiX::Widgets
