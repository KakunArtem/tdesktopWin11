# Material fills verification

Date: 2026-11-04. Code commit: `e367656b34`. Windows: 26300.9711.
Qt: 6.11.2. GPUs: AMD Radeon Graphics and NVIDIA GeForce RTX 5080.
Display: 2560 x 1440, DPI 96. Application scenarios used an isolated,
unauthorized fixture with the production session gate; no user account data.

| Row | How | Result | Evidence |
| --- | --- | --- | --- |
| Calculator | Standalone test_window_material | Pass | 49 checks, zero failures; mockup values, every floor, borders, alpha text, readable adjustment and UI cache publication |
| Service | Native Debug application scenario | Pass | 142 assertions: all six fills/borders, day/night contrast, exact restoration, repeated day values, native-frame and reported-failure fallback |
| Original ownership | Partial palette change, then deactivate | Pass | Reproduced six failures before fix; all original text colors now restore exactly |
| Theme snapshots | Actual night preview/revert, canonical save and per-chat copy | Pass | Reproduced six rollback failures; canonical and copied palettes contain originals, live palette unchanged |
| Replacement provenance | Edited palette intentionally reuses old adjusted colors | Pass | All six intentional originals retained after deactivation |
| Original unreadable text | Secondary-only 1:1, black primary on white | Pass | Three roles failed before fix; original unreadable roles now remain opaque |
| Dense custom material | Grey surface/accent with black primary/secondary | Pass | Four ratios failed near 1.15 before fix; final accent reaches at least 4.51 over both backdrops |
| Impossible shared target | Incompatible opaque backgrounds | Pass | Terminates with opaque affected roles and original accent, without looping |
| Regression suite | Account-free application overlay | Pass | 79 assertions; original four cases went RED with 19 failures, then GREEN; no second review after the prescribed TDD fix pass |
| Notification guard | No-op and real theme changes | Pass | No-op recomputes zero times; night/custom changes recompute once |
| Text restoration | Live native-frame fallback and native-failure report | Pass | Entire palette byte-for-byte equal to inactive baseline |
| Normal build | Restore no-op overlay and build Telegram Debug | Pass | Normal executable rebuilt; no task instrumentation retained |
| Review/lint | One fresh whole-branch reviewer, then one fix pass | Pass | Four Important findings fixed; zero Critical or Minor; Qt lint zero findings |
| Native OS/visual matrix | Real OS toggles, screenshot fidelity, scaled displays and RTL | Pending separately | Existing C2/C16 acceptance remains open; these color/state tests do not claim pixel acceptance |
| Editor interaction | Signed-in editor save gesture | Not performed | Persistence uses the tested canonical snapshot helper; no user theme was edited |
| Other platforms | Build/runtime checks | Not performed | Windows-first checkout; inactive paths source-reviewed |

Full evidence is outside the repository at
`%LOCALAPPDATA%\Packages\OpenAI.Codex_2p2nqsd0c76g0\LocalCache\Local\tdesktop-ui-checks\material-fills\`:
`review-red`, `review-green`, `review-full-suite` and `review-extended`.
The disposable overlay is preserved there as `verification-overlay.cpp`;
the repository's scenario is the original no-op. No private chat captures
were created or committed.

C3's calculator, publication, palette ownership and restoration contracts are
implemented. Native OS and visual acceptance remains tracked separately in
[C2](material-state.md) and [C16](verification-tooling.md). C4 and later
components have not been started.

## Native Acrylic re-measure

Date: 2026-11-06. The window looked far less translucent than the mockup. The
[native measurement](../decisions/2026-10-native-backdrop.md#native-measurement)
showed why: fills assumed raw black and white behind the window, but Acrylic
only spans 54 to 146 in dark and 135 to 227 in light over those backdrops.

`AcrylicRange()` now supplies the contrast backdrops: 54 to 169 in dark, because
two-channel colors such as cyan peak near 169, and 132 to 255 in light. Saturated
rows in that measurement partly lagged, so 169 is inferred from the rows that
settled, and is not a full sweep of every color.

| Role | Floor before (light / dark) | Floor now (light / dark) |
| --- | --- | --- |
| Window | 0.72 / 0.74 | 0.45 / 0.50 |
| Navigation | 0.78 / 0.82 | 0.52 / 0.60 |
| Conversation | 0.79 / 0.81 | 0.52 / 0.60 |

Bar, Composer and Popup keep their floors. Mockup text computes to 0.52 light
and 0.63 dark on Navigation. Over the darkest and brightest measured areas of the
four wallpapers it stays between 4.85:1 and 12.97:1. In the real day and night
themes the Window fill published 0.50 and 0.60 alpha.

A real isolated window confirms the composition. Its title reads 195 over black,
241 over white, 255, 189, 189 over red and 194, 194, 255 over blue while
focused, and a flat 233 unfocused, matching a 0.50 white fill over the measured
Acrylic. That window showed the sign-in screen with an empty profile, so the
chats list and panels at these fills have not been seen by this check. The
standalone suite passes after 9 failures against the old floors.

These numbers come from one machine, Windows build 26300. A later Windows build
that changes Acrylic's tint would need a new measurement.

## Visible scope at the C3 checkpoint

C3 publishes its six fills and borders, but no production painter consumes
`Ui::Material::Fill()` or `Border()` yet. Existing panel painters keep their
upstream fills. The C1 strip still uses `PrototypeFill()`. C4 connects the
material cache to surface painting; later components change the layout.

The current visible effect is limited to the six adjusted foreground entries.
Normal chat previews, dates, service text and accent text can change, while
selected and hovered variants outside that table keep their existing colors.

Follow-up on 2026-11-04: a read-only symbol-based inspection of the running
signed-in Debug client confirmed adjusted dark-theme colors, including
`dialogsTextFg #a8bbcf`, `dialogsDateFg #abbbce` and
`windowActiveTextFg #79c0ff`. Startup material was accepted and all native
backdrop calls succeeded. This checks live palette values, not rendered glyph
pixels. The inspection did not restart or suspend the client, change settings,
read chat content or modify production code.

## Implementation decisions and limits

- Continue in-place on codex/c3-material-fills from C16 — preserves the native configured build and established checkout preference — cost if wrong: less filesystem isolation than a worktree.
- Use native PowerShell bookkeeping rather than Bash helper scripts — this is a Windows checkout — cost if wrong: manual ledger errors need checking against git.
- Collect both backdrop composites for every listed role when adjusting shared text — custom Navigation and Conversation backgrounds can differ, so one strictest floor alone cannot represent them — cost if wrong: adjustment may be more conservative than a single-role shortcut.
- Force opaque when any text has contrast 1:1 on its opaque surface — Task 1's explicit unreadable-theme test/review focus overrides the general min-target loop, which otherwise accepts any alpha at target 1 — cost if wrong: a deliberately unreadable custom theme loses translucency.
- Composite alpha-bearing text before measuring contrast — preserves the existing readability helper's behavior for custom translucent text instead of treating it as fully opaque — cost if wrong: some custom themes may get more opaque fills.
- C2/C16 manual OS, native screenshot and scaled-display acceptance stay open during this requested continuation — user confirmed visible transparency and asked to proceed — cost if wrong: later integration may expose a defect those pending rows would have caught.
- Supply a minimal test-target PCH and explicit base/assertion.h — existing color_contrast.cpp relies on app PCH for float64/array/algorithm and the standalone target had none — cost if wrong: additional test PCH maintenance; product PCH unchanged.
- Ignore byte-identical palette notifications — retaining original colors avoids treating adjusted entries as their own originals, and avoids redundant recomputation — cost if wrong: an external notification without changed palette data cannot force a repaint through FillsChanged; widgets also have their palette subscription.
- Observe recompute counts only in disposable test overlay — root AGENTS prohibits temporary production debug machinery — cost if wrong: no permanent diagnostic count is available.
- Exact byte restoration through live native-frame fallback covers the automated restoration path; OS transparency/high-contrast screenshots stay pending — user settings are untouched and pixel acceptance needs the exposed client — cost if wrong: an OS-specific visual issue can remain despite correct service state.
- Add explicit replacement provenance and canonical snapshots at all app theme load/apply/reset/save boundaries — color equality alone cannot distinguish replacement from partial update — cost if wrong: a future new boundary must adopt these calls to preserve ownership.
- Increase implicated role coverages together before retrying all adjusted text — fixes infeasible floor adjustments with finite progress — cost if wrong: a custom role may be slightly more opaque than a globally optimized joint solution.
- Preserve original colors and opaque affected roles if even opaque backgrounds cannot share a target foreground — avoids loops and does not worsen the original theme — cost if wrong: inherently unreadable custom themes remain unreadable.
- Exact whole-palette restoration through real native-frame and failure fallbacks replaces C3's manual OS-toggle pixel comparison — directly proves its color ownership contract while respecting untouched user OS settings — cost if wrong: native visual composition acceptance remains with C2/C16.
- Native screenshot contrast/fidelity remains separate acceptance — only color/state paths are certified here — cost if wrong: an unobserved raster defect could remain.
- Actual OS effects/high-contrast broadcasts remain in C2 acceptance — service fallback branches are tested, not those broadcasts — cost if wrong: OS-specific event handling may remain unverified.
- Scale and RTL rendering remain native-matrix acceptance — C3 role calculation has no geometry — cost if wrong: later painting may reveal clipping or mapping issues.
- Non-Windows parity is source-reviewed only — Windows checkout and no corresponding toolchains — cost if wrong: a platform build or runtime issue may remain.
- C4 painting and later adoption stay outside this change — user explicitly stops after C3 — cost if wrong: cached fills are not yet the final redesigned panels.
- Later-component keys and unread badges remain excluded — use the current explicit table as specified — cost if wrong: their readability needs later component checks.
- Canonical per-chat copy construction is now exercised, while complete signed-in per-chat lifecycle remains a later integration check — avoids persisting adjusted global originals — cost if wrong: per-chat visual integration may need a follow-up.
- Newly added reproduction overlays are accepted via direct RED/GREEN evidence — 19 failures became zero and 79 extended checks pass — cost if wrong: author-written tests can miss additional cases.
- Post-review fixes use the single TDD fix pass, not another reviewer — required executing-plans workflow — cost if wrong: fixes lack a second independent inspection.
- Keep the local topic branch after finalization — user asked to finish C3 and stop, with no integration request — cost if wrong: integration into dev remains a separate action.
