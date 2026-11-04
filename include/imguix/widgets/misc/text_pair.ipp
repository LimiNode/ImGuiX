#include <imguix/config/build.hpp>

#include <algorithm>

namespace ImGuiX::Widgets {

    namespace {

        struct MeasuredText final {
            ImVec2 size{};
            ImFont* font = nullptr;
            float font_size = 0.0f;
        };

        MeasuredText measure_text(const TextPart& part) {
            ImFont* font = part.font != nullptr ? part.font : ImGui::GetFont();
            // A null part font means "use the active font and size". An
            // explicitly supplied font uses its native registration size when
            // no override is requested.
            const float font_size = part.font_size > 0.0f
                                        ? part.font_size
                                        : (part.font != nullptr && font != nullptr
                                               ? font->LegacySize
                                               : 0.0f);

            ImGui::PushFont(font, font_size);
            const char* begin = part.text.data();
            const char* end = begin != nullptr ? begin + part.text.size() : nullptr;
            const ImVec2 size = begin != nullptr
                                    ? ImGui::CalcTextSize(begin, end, false, -1.0f)
                                    : ImVec2{};
            ImGui::PopFont();
            return MeasuredText{size, font, font_size};
        }

        void draw_text(const TextPart& part, const MeasuredText& measured,
                       const ImVec2& position) {
            if (part.text.empty() || measured.font == nullptr) {
                return;
            }

            const char* begin = part.text.data();
            const char* end = begin + part.text.size();
            ImGui::PushFont(measured.font, measured.font_size);
            ImGui::GetWindowDrawList()->AddText(
                position, ImGui::GetColorU32(part.color), begin, end);
            ImGui::PopFont();
        }

    }  // namespace

    IMGUIX_IMPL_INLINE ImVec2 TextPair(const TextPairConfig& config) {
        const MeasuredText left = measure_text(config.left);
        const MeasuredText right = measure_text(config.right);
        const float spacing = (!config.left.text.empty() && !config.right.text.empty())
                                  ? std::max(0.0f, config.spacing)
                                  : 0.0f;
        const float right_y = config.right_offset_y;
        const float width = left.size.x + spacing + right.size.x;
        const float height = std::max(left.size.y, right_y + right.size.y);
        const ImVec2 origin = ImGui::GetCursorScreenPos();

        draw_text(config.left, left, origin);
        draw_text(config.right, right, ImVec2(origin.x + left.size.x + spacing,
                                              origin.y + right_y));

        ImGui::Dummy(ImVec2(width, std::max(0.0f, height)));
        return ImVec2(width, std::max(0.0f, height));
    }

}  // namespace ImGuiX::Widgets
