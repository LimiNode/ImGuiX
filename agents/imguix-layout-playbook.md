# ImGuiX immediate-mode layout playbook

Use this playbook when changing widget rows, toolbars, tables, or other
horizontal/vertical composition in ImGuiX.

## Cursor rules

- A normal item (`Button`, `Text`, `Selectable`, `Combo`, and similar) calls
  Dear ImGui's `ItemSize()`. After the item, the cursor is already positioned
  at the next line and `IsSameLine` is cleared.
- Call `SameLine()` only when the next item is intentionally placed beside the
  previous item and the available width has been checked.
- Do not add `NewLine()` as a defensive wrap after an item that does not fit.
  Simply omit `SameLine()` and let the next item start on the normal next
  line. An extra `NewLine()` adds another line-height/spacing step and creates
  a visible vertical gap.
- Use `NewLine()` only when an intentional additional line break or vertical
  breathing room is part of the widget contract.

For a wrapping row, the normal pattern is:

```cpp
if (fits_on_current_line) {
    ImGui::SameLine();
}
ImGui::Button(label);
```

Do not use `else { ImGui::NewLine(); }` in this pattern.

## Groups and toolbars

- `EndGroup()` submits the group as one item and advances layout through
  `ItemSize()`. A following item naturally starts on the next line unless
  `SameLine()` is requested.
- A selection/status row after a button row therefore does not need an
  explicit `NewLine()` merely to avoid being appended to the buttons.
- Compute available width relative to the actual previous item/group bounds.
  Keep `SameLine()` conditional so localized labels can wrap naturally.
- Do not reserve an empty selection row when there is no selection.
- Manual cursor positioning, calculated dimensions, conditional `SameLine()`,
  style scopes spanning `BeginChild()`/`EndChild()`, and draw-list geometry
  should have a short intent comment when the reason is not obvious from the
  API. Prefer extracting named helpers over growing a monolithic draw method.
- A container that configures a child window may scope its metrics only around
  `BeginChild()` when Dear ImGui snapshots them into the child. In particular,
  `ContentSurface` must restore its padding/rounding before descendants draw so
  popups and nested children use the normal theme metrics.

## Verification checklist

- Test a wide layout where controls fit on one row.
- Test a narrow layout and a long localized label.
- Check that wrapping has no blank line between rows.
- Check that optional/hidden actions do not leave empty vertical space.
- Rebuild both static-library and `IMGUIX_HEADER_ONLY` consumers when a public
  widget header or `.ipp` implementation changes.
