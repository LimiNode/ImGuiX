# ImGuiX styling playbook

Use this playbook when adding or reviewing temporary styling, theme integration,
or panel layout code in an ImGuiX consumer.

## Scoped style overrides

Use the RAII guards from `imguix/extensions/scoped_style.hpp` for temporary
style changes:

```cpp
{
    const ImGuiX::Extensions::ScopedStyleVar padding(
        ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 20.0f));
    const ImGuiX::Extensions::ScopedStyleColor border(
        ImGuiCol_Border, ImGui::GetStyle().Colors[ImGuiCol_Border]);

    const bool content_visible = ImGui::BeginChild("panel");
    if (content_visible) {
        draw_content();
    }
    ImGui::EndChild();
}
```

The guard must cover the `Begin*` call and any drawing that consumes the style
for a window, child, popup, table, or widget. Follow the API-specific cleanup
rule: `End`/`EndChild` is unconditional, while `EndTable`/`EndPopup`/`EndCombo`
is called exactly once only when its `Begin*` call returns `true`.

`ScopedStyleVar` and `ScopedStyleColor` are deliberately non-copyable and
non-movable. Create named local guards; do not return them, store them in a
container, or manually pair their stack operations elsewhere.

## Theme ownership

Prefer the active theme for values that express the shared design language:

```cpp
const ImGuiStyle& style = ImGui::GetStyle();
const ImVec4 selected = style.Colors[ImGuiCol_NavHighlight];
```

Use `ThemeManager` custom values/colors for reusable tokens that are not Dear
ImGui style roles. Keep screen-specific geometry local until it is repeated in
more than one screen. Do not add a large JSON token catalogue for one-off
coordinates.

Product colors must not be hardcoded in a reusable ImGuiX widget. If a
consumer needs a product-specific role, define it in the consumer theme and
resolve it at draw time.

## Review checklist

- Search consumer code for raw `PushStyleVar`, `PushStyleColor`, and matching
  `PopStyle*` calls. Replace temporary overrides with RAII guards.
- Verify every `Begin*` has an unconditional matching `End*`.
- Confirm style guards remain alive for the widget/window that consumes them.
- Use `docs/THEMES.md` for the complete `ThemeManager` and custom-token API.
- Build at least one relevant smoke example after changing style or theme code.
