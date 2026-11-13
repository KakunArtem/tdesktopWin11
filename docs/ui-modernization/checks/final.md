# Final Soft Panels verification

Date: 2026-11-13. Commit: d3fad5206d. Windows: 10.0.26300. GPUs: AMD Radeon(TM)
Graphics; NVIDIA GeForce RTX 5080. One 2560×1440 display. Plan:
[info, settings and specialized screens](../plans/2026-10-29-info-settings.md),
Task 4.

This record gathers the [verification matrix](../implementation-plan.md#verification-matrix-for-every-phase)
across the component records. Rows marked "Not run" or "Pending" are not passes.

## Automated

| Check | Result | Evidence |
| --- | --- | --- |
| Material unit tests | Pass | `test_window_material`: 147 checks, 0 failures, built at d3fad5206d. |
| Shell and title, isolated native raster | Pass | [Shell layout](shell-layout.md) and [title bar](title-bar.md): light and dark, material and solid, 100% and 150%, 0 failures. Qt raster output, not desktop screenshots. |
| Fills and contrast | Pass | [Material fills](material-fills.md): 142 assertions, including day and night contrast. |
| Material state | Pass, with live rows pending | [Material state](material-state.md): the setting, capability and fallback rows pass; live transparency and high contrast toggles are pending. |
| Settings upgrade | Pass | C15 Task 3: throwaway profiles migrate stored on, stored off and unset to on, off and on; the user's profile kept its values after a restart. |

## Matrix

| Row | Result | Notes |
| --- | --- | --- |
| Light, dark and solid on the four test wallpapers | Partial | Light and dark material and solid are covered by isolated raster checks and the user's real-window approvals per component. The four-wallpaper desktop set has not been captured for the whole window. |
| Interface scale 100%, 125%, 150%, 200% | Partial | 100% and 150% pass in isolated raster checks. 200% checked by the user in a real window on 2026-11-13: no clipped text. 125% not run. |
| Monitors with different DPI | Not run | One display. |
| Normal, maximized, snapped, full-screen, inactive | Partial | Normal and inactive windows were used during the component checks; maximized, snapped and full-screen not run as a set. |
| Transparency effects off | Pass, user check | The user turned Windows transparency effects off and on on 2026-11-13: solid while off, frosted again after. Not measured; the log records the material decision only at startup. |
| High contrast | Pass, user check | The user applied a contrast theme and removed it on 2026-11-13: solid and readable. Not measured. |
| Windows 10 or older Windows 11 | Not run | No such machine. The build policy refuses material below 22621 and is covered by the unit tests. |
| Custom and cloud theme | Not run | |
| Right-to-left layout | Not run | |
| Keyboard focus | Pass, user check | The user tabbed through the main window, Settings and a box on 2026-11-13: focus always visible. |
| Narrator | Reported fine | Optional step in the user's run on 2026-11-13; the user reported all steps fine. |
| Contrast on native screenshots | Partial | Measured per component where recorded; not repeated over the whole window. |
| Behavior unchanged | Partial | No action, menu or state was removed; the changed behaviors are recorded: instant Acrylic menus, the panel content inset and C3's secondary text in separate windows. |

## Separate windows

As recorded in [separate windows](separate-windows.md): the code audit passes;
the restore shell and the side-by-side screenshots are not run.

## Known gaps

- Payments and Telegram Passport panels and the sign-in code, password and
  sign-up steps need a test bot or account.
- A profile with no signed-in account clears all settings prefs on every start,
  as upstream does, so the old experimental values apply there each time.
- The composer drop shadow from the mockup is still an open decision.
