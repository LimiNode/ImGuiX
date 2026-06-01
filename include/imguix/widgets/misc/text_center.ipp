#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>
#include <imguix/config/build.hpp>

namespace ImGuiX::Widgets {

    IMGUIX_IMPL_INLINE void TextCenteredFmt(const char* fmt, ...) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        std::vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        const float avail_w = ImGui::GetContentRegionAvail().x;
        const float text_w  = ImGui::CalcTextSize(buf).x;

        const float x0 = ImGui::GetCursorPosX();
        const float x  = (text_w < avail_w) ? x0 + (avail_w - text_w) * 0.5f : x0;
        ImGui::SetCursorPosX(x);
        ImGui::TextUnformatted(buf);
    }

    IMGUIX_IMPL_INLINE void TextUnformattedCentered(const char* text) {
        if (!text) return;
        const float avail_w = ImGui::GetContentRegionAvail().x;
        const float text_w  = ImGui::CalcTextSize(text).x;

        const float x0 = ImGui::GetCursorPosX();
        const float x  = (text_w < avail_w) ? x0 + (avail_w - text_w) * 0.5f : x0;
        ImGui::SetCursorPosX(x);
        ImGui::TextUnformatted(text);
    }

    IMGUIX_IMPL_INLINE void TextWrappedCentered(const char* text, float wrap_width) {
        if (!text) return;

        const float avail_w = ImGui::GetContentRegionAvail().x;
        const float block_w = (wrap_width > 0.0f) ? std::min(wrap_width, avail_w) : avail_w;

        const float x0 = ImGui::GetCursorPosX();
        const float x  = x0 + (avail_w - block_w) * 0.5f; // центрируем «контейнер»

        ImGui::SetCursorPosX(x);
        // wrap позиция задаётся в ЛОКАЛЬНЫХ координатах текущего окна:
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + block_w);
        ImGui::TextUnformatted(text);
        ImGui::PopTextWrapPos();
    }

    IMGUIX_IMPL_INLINE void TextWrappedLinesCentered(const char* text, float wrap_width) {
        if (!text) return;

        const float avail_w = ImGui::GetContentRegionAvail().x;
        const float block_w = (wrap_width > 0.0f) ? std::min(wrap_width, avail_w) : avail_w;
        const float x0 = ImGui::GetCursorPosX();
        const float block_x = x0 + (avail_w - block_w) * 0.5f;

        std::vector<std::string> lines;
        std::string current;
        std::string word;

        auto flush_word = [&]() {
            if (word.empty()) {
                return;
            }

            const std::string candidate = current.empty() ? word : current + " " + word;
            if (!current.empty() && ImGui::CalcTextSize(candidate.c_str()).x > block_w) {
                lines.push_back(current);
                current = word;
            } else {
                current = candidate;
            }
            word.clear();
        };

        auto flush_line = [&]() {
            flush_word();
            lines.push_back(current);
            current.clear();
        };

        for (const char* p = text; *p != '\0'; ++p) {
            if (*p == '\n') {
                flush_line();
            } else if (*p == ' ' || *p == '\t') {
                flush_word();
            } else {
                word.push_back(*p);
            }
        }
        flush_word();
        if (!current.empty() || lines.empty()) {
            lines.push_back(current);
        }

        for (const std::string& line : lines) {
            const float line_w = ImGui::CalcTextSize(line.c_str()).x;
            const float line_x = (line_w < block_w) ? block_x + (block_w - line_w) * 0.5f : block_x;
            ImGui::SetCursorPosX(line_x);
            ImGui::TextUnformatted(line.c_str());
        }
    }

} // namespace ImGuiX::Widgets
