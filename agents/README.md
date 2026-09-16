# agents/

Agent-focused playbooks for this ImGuiX submodule.

Files:

- `imguix-smoke-build.md` - Known-good commands to configure and rebuild smoke examples with MinGW.
- `imguix-fonts-i18n-playbook.md` - Quick operational checklist for documenting and validating fonts + i18n behavior.
- `imguix-windowing-playbook.md` - Practical checklist for `WindowInstance` / `ImGuiFramedWindow` changes and docs sync.
- `imguix-styling-playbook.md` - RAII style guards, theme ownership, and styling review checks.
- `imguix-table-playbook.md` - Table surfaces, bordered data panels, filtering, and range-selection rules.

When to use `imguix-fonts-i18n-playbook.md`:

- You update `README*`, `docs/FONTS-GUIDE*`, `docs/I18N-GUIDE*`, or agent docs touching fonts/i18n behavior.
- You need to verify docs against current APIs (`FontManager`, `LangStore`, `WindowInterface`, `Controller`).
- You run a docs-only QA pass and need fast anti-pattern checks before commit.

Expected outputs:

- API-aligned snippets for font role switching and i18n language flow.
- No stale signatures/roles in docs.
- EN/RU guides consistent in behavior and terminology.

When to use `imguix-windowing-playbook.md`:

- You touch `include/imguix/core/window/*` or `include/imguix/windows/*`.
- You update corner/classic smoke examples (`corner_icon_area_*`) or framed-window behavior docs.
- You need backend-aware verification (SFML vs GLFW/SDL2) before finalizing windowing changes.

Expected outputs from `imguix-windowing-playbook.md`:

- Consistent behavior statements for classic/corner layouts and config semantics.
- Backend-aware notes (SFML vs GLFW/SDL2) without over-generalization.
- At least one representative smoke build check tied to the changed windowing scenario.

Toolchain policy:

- Use CMake generator `MinGW Makefiles`.
- Use MinGW compilers `gcc` and `g++`.
- Do not use `Ninja`.
- Do not use `NMake` / `nmake`.

Template placement policy:

- Public template APIs must remain visible in both header-only and static-lib modes.
- Put consumer-facing template definitions in `.hpp` or in `.tpp` files that are always included from `.hpp`.
- Use `.ipp` only for non-template implementation or header-only/backend-specific implementation details.
