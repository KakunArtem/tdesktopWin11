# Component plans

One implementation plan per component of the
[redesign architecture](../architecture.md). Execute them in the order below,
one at a time. Progress and open decisions are tracked in the architecture's
[Progress](../architecture.md#progress) section. Plans after Phase 1 assume its decision record ends in go or
conditional go; reread [What a Phase 1 no-go changes](../architecture.md#what-a-phase-1-no-go-changes)
before starting any of them.

| Order | Component | Plan |
| --- | --- | --- |
| 1 | C1 Native backdrop (Phase 1) | [2026-10-28-native-backdrop-prototype.md](2026-10-28-native-backdrop-prototype.md) |
| 1 | C16 Verification tooling, task 1 onwards | [2026-10-29-verification-tooling.md](2026-10-29-verification-tooling.md) |
| 2 | C2 Material state | [2026-10-29-material-state.md](2026-10-29-material-state.md) |
| 3 | C3 Material fills | [2026-10-29-material-fills.md](2026-10-29-material-fills.md) |
| 4 | C4 Surface roles | [2026-10-29-surface-roles.md](2026-10-29-surface-roles.md) |
| 5 | C5 Transparent grabs | [2026-10-29-transparent-grabs.md](2026-10-29-transparent-grabs.md) |
| any time before 7 | C6 Toolkit fork | [2026-10-29-toolkit-fork.md](2026-10-29-toolkit-fork.md) |
| 6 | C7 Shell layout | [2026-10-29-shell-layout.md](2026-10-29-shell-layout.md) |
| 7 | C8 Title bar | [2026-10-29-title-bar.md](2026-10-29-title-bar.md) |
| 8, parallel | C9 Navigation rail | [2026-10-29-navigation-rail.md](2026-10-29-navigation-rail.md) |
| 8, parallel | C10 Chat list and search | [2026-10-29-chat-list.md](2026-10-29-chat-list.md) |
| 8, parallel | C11 Conversation chrome | [2026-10-29-conversation-chrome.md](2026-10-29-conversation-chrome.md) |
| 8, parallel | C12 Composer | [2026-10-29-composer.md](2026-10-29-composer.md) |
| 8, after C11 | C13 Messages | [2026-10-29-messages.md](2026-10-29-messages.md) |
| 9 | C17 Shared controls | [2026-10-30-shared-controls.md](2026-10-30-shared-controls.md) |
| 10 | C18 Foundations | [2026-10-31-foundations.md](2026-10-31-foundations.md) |
| 11 | C14 Transient surfaces | [2026-10-29-transient-surfaces.md](2026-10-29-transient-surfaces.md) |
| 12 | C15 Info, settings and specialized screens | [2026-10-29-info-settings.md](2026-10-29-info-settings.md) |

"Parallel" plans touch disjoint directories; they can run in separate
sessions or worktrees, but each still lands as its own commits. C13 consumes
C11's chat background state, so it starts after C11 lands.

C19 Separate windows is grouped in [shared elements](../shared-elements.md)
and has no plan yet. When planned, it goes after C14.

## Ground rules

Every plan's Global Constraints include these lines.

- Read the root `AGENTS.md`, in particular `## Commits`, the comment rules,
  `## Text File Format` and `## Debug-Only Code`, before the first task.
- Source files use tabs, UTF-8 without BOM, and the line endings of the
  checkout (CRLF in the native Windows checkout).
- Commit subjects are plain past-tense lines in the style of `git log`, with
  no `[ai]` prefix for product code and no attribution trailers.
- Build Debug only, with `cmd /c .ai\setup\build-target.cmd <target>` from
  Phase 1 Task 0. Never build Release. If `out\Debug\Telegram.exe` is
  running, ask the user to close it.
- Never cite line numbers in code, comments or docs; name functions, members
  and style keys.
- Sizes are style values in `.style` files; durations are `constexpr crl::time`
  constants in the `.cpp`.
- Solid parity: with "Window transparency" off and the Soft Panels switch off,
  the app looks and behaves exactly as before the plan.
- Every background in a translucent region goes through
  `Ui::Material::PaintSurface` or paints nothing because its container
  does ([surface roles contract](../architecture.md#surface-roles-c4)).
- `lib_ui` and `lib_base` changes are committed in the forks from the
  [toolkit fork plan](2026-10-29-toolkit-fork.md), and the submodule pointer
  is updated in a separate tdesktop commit. Recheck every named `lib_ui`
  function at the pinned commit before editing: the planning survey could not
  read the submodules.
- After each task's commit, update the plan's row in
  [Progress](../architecture.md#progress) in the same commit or right after it.
- Visual checks use the tools from the
  [verification tooling plan](2026-10-29-verification-tooling.md):
  `Test-Backdrop.ps1`, `Measure-Contrast.ps1` and the screenshot matrix in
  `docs/ui-modernization/checks/`.

## Check rows used by element plans

Element plans end with a check task built from these rows. "Matrix" means the
rows of the [verification matrix](../implementation-plan.md#verification-matrix-for-every-phase).

| Row | Pass condition |
| --- | --- |
| Solid parity | Screenshots with both switches off match the pre-plan screenshots of the same state. |
| Material | `Test-Backdrop.ps1 -Expect Transparent` passes and the changed pixels lie inside the plan's regions. |
| Contrast | `Measure-Contrast.ps1` reports 0 failures (4.5:1 text, 3:1 icons and large text) on the four wallpapers, light and dark. |
| States | Every state the plan lists is shown, screenshotted and still reachable by mouse and keyboard. |
| Scale | Screenshots at 100%, 125%, 150% and 200% show no clipped text, misaligned shapes or 1 px seams. |
| RTL | The same screens with an RTL language mirror correctly. |
