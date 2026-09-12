# ImGuiX table playbook

Use this playbook for scrollable tables, selectable rows, filtering, and
composed data panels. It documents reusable Dear ImGui invariants; individual
applications remain responsible for their own panel composition and visual
language.

## Separate table responsibilities

Give each visual mechanism one responsibility:

| Mechanism | Responsibility |
| --- | --- |
| `ImGuiTableFlags_RowBg` | Zebra/background rows |
| `Selectable` | Hover, pressed, and selected interaction surface |
| `ImGuiTableFlags_BordersInnerH/V` | Row and column separators |
| `TableSetupScrollFreeze(0, 1)` | Keep the header visible while scrolling |

Do not draw the selected state twice by combining a selected `Selectable` with
`TableSetBgColor(ImGuiTableBgTarget_RowBg0, ...)`. Do not zero the theme's
vertical `CellPadding` and compensate with a fake spacer row. Keep the active
theme padding and adjust a local widget style only when there is a demonstrated
layout requirement.

## Optional bordered data-panel pattern

When a consumer wants controls and a dataset to read as one component, it may
keep them inside one bordered child:

```cpp
const bool panel_visible = ImGui::BeginChild(
    "##data_panel", ImVec2(0.0f, 0.0f),
    ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);

if (panel_visible) {
    draw_toolbar();
    ImGui::Separator();

    if (ImGui::BeginTable(
            "##data", column_count,
            ImGuiTableFlags_RowBg |
                ImGuiTableFlags_BordersInnerH |
                ImGuiTableFlags_BordersInnerV |
                ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        draw_header();
        draw_rows();
        ImGui::EndTable();
    }
}
ImGui::EndChild();
```

The child owns the outer border and padding; the table owns inner separators.
This is a composition recipe, not a required application layout. Avoid adding
a second outer table border unless the consuming design explicitly calls for it.

## Filtering and range selection

Render and interact with a visible projection rather than raw backing indices:

```text
backing collection -> visible projection -> render / range selection / copy
```

Shift-range selection must use positions in the visible projection. Copy and
export must use the same projection and selection rules. Otherwise hidden rows
can be selected and copied after a filter is applied.

The application owns the policy for hidden selections when a filter changes
(prune them, preserve them, or ask the user). Document that policy in the
feature's model/controller guide.

## Review checklist

- Keep `CellPadding` inherited from the active theme unless a local override is
  justified.
- Use one selected/hover surface, one separator mechanism, and one zebra
  mechanism.
- Freeze the header for scrollable data tables.
- Call `EndTable` exactly once when `BeginTable` returns `true`; do not call it
  on a `false` return.
- Keep `EndChild` unconditional after `BeginChild`.
- Test filtering followed by Shift-selection and copy/export.
