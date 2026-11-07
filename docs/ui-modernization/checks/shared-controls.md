# C17 shared controls checks

Date: 2026-11-07. Tasks 1 to 3 are implemented. Task 4 is partial: the audit and the
automated checks are done, and two resting boxes have been seen in a real window.
Plan:
[shared controls](../plans/2026-10-30-shared-controls.md).

## Implemented behavior

Under Soft Panels, a control's fill in the surface color (`windowBg`, `boxBg`) is
skipped, so the control shows the panel or box behind it. A fill in `windowBgOver`,
which covers hover and divider bands, becomes `windowFg` at 6% in light palettes and
7% in dark ones. Every other fill, ripple, border, mark and text is unchanged.

This applies to `RoundButton`, `FlatButton`, `SettingsButton`, `InputField`,
`MaskedInputField`, `BoxContentDivider`, `MultiSelect`, and peer lists and their
boxes.

`checkboxFg` and `activeLineFg` are raised to 3:1 and `lightButtonFg` to 4.5:1 on the
Navigation, Conversation and Popup fills while material is active. Like C3's other
adjusted entries, this does not depend on Soft Panels.

`Ui::Material::SelectedFill()` is added for C9 and C10. Nothing uses it yet.

## Changes from the plans

- The bars below a chat keep their fill. `FlatButton` is used almost only for these
  join, unblock and search bars, which sit on the wallpaper, and the plan's rule would
  have left bare text there. `ControlFill` leaves `historyComposeButtonBg` and
  `historyComposeButtonBgOver` alone, matched by palette entry, not by color value.
  The chat preview's mark read button, which shares that color, keeps its fill too.
- `MaskedInputField` is added. It has the same fill and opaque rule as `InputField`,
  and the plan's mapping list left it out.
- `MultiSelect` also drops its opaque attribute, which the plan left out, and
  `PeerListBox::paintEvent` also maps its fill.
- `RowsScrollCache` gains a transparent image format under Soft Panels. It used
  `Format_RGB32` and never cleared, so skipping the row fill would have shown
  uninitialized pixels while scrolling. The plan's transparent clear of the status
  icon area assumed an image with alpha.
- `test_window_material` already links `lib_ui`, so its CMake file is unchanged.
- `SelectedFill()` lives in `ui_material_bars.cpp`, not `ui_material.cpp`, because the
  unit test compiles the latter without generated styles.
- `RefreshSoftControls()` sets the switch and the hover, at startup and on palette
  changes, so a test can drive it.

## Automated evidence

A disposable `-testagent` overlay builds real controls with an empty profile, over an
opaque backdrop, in day and night, with Soft Panels on and off, material on and off,
at 100% and 150%. It passes 95 checks per material run and 59 per solid run, 308 in
all:

- Soft Panels on: the light button, row, field, masked field and multi-select show the
  backdrop and are no longer opaque. A hovered row and the divider show the soft tint.
  The filled button and the chat bar keep their fills.
- The mapping's values, including the chat bar's exemption, and `SelectedFill()`.
- The three raised colors reach their targets on both ends of the measured Acrylic
  range for all three roles.
- Soft Panels off, in solid mode: all captures are byte-identical to a baseline taken
  before any change.
- Soft Panels off, in material mode: captures differ only inside the light button's
  label, from the raised text color.

Regressions, on this change:

- [Composer checks](composer.md#automated-evidence): 192 pass.
- [Bar test](conversation-chrome.md#automated-evidence): 532 pass, all 32 captures
  byte-identical.
- [Title bar matrix](title-bar.md#automated-evidence): 1024 pass, all 80 captures
  byte-identical.
- `test_window_material`: 142 pass, 6 of them new. These were not seen failing first:
  without the new header the test does not compile.

Those regression overlays do not turn soft controls on, so they do not show that the
header, bars and composer look right with the mapping active.

## Audit

A script resolved each style's `textBg`, `textBgOver`, `bgColor` and `overBgColor`
through inheritance and palette aliases. 154 styles map: 33 `RoundButton`, 58
`InputField`, 56 `SettingsButton` and 7 `FlatButton`. Their uses were then read by
file. Most sit in boxes, settings and panels, on a role fill. It does not cover
styles built in code, and it read where a style is used, not each widget's parent.

Those that sit on something else:

| Control | Sits on | Owner |
| --- | --- | --- |
| Chat bars: join, unblock, bot start, mute, report, search list, pinned clear, admin log filter | Wallpaper | Exempt; keep their fill |
| `infoProfileTopBarShowLastSeen` | The profile's colored header | C15. It loses its white pill under Soft Panels until then, a known regression |
| `themePreviewCancelButton` | The theme preview's bar in the media viewer, painted in `themePreviewBg` | Not seen; shows that bar's fill |
| Passport, IV and intro fields and buttons | Their own window's `windowBg` | No visible change expected; not seen |

The top bar search field (`dialogsFilter`) keeps its opaque fill for now, because its
rounded path only drops the brush when both states map to transparent. C10 owns it.

## User screenshots

On 2026-11-07 the user shared four signed-in screenshots, without comment, in the day
theme with Soft Panels and material on. They show no opaque patches in these states:

- The Advanced settings box at rest, with one hovered row, its section bands, toggles
  and checkboxes.
- The Members box with a single peer list row.

## Not verified

There is no test-server account on this machine, and these need a real window:

- Other settings pages and the info pane. Scrolling a long peer list: the overlay
  does not build peer lists, so the scroll cache change has only been compiled.
- Peer list hover, selection and status icons, and a box's multi-select field.
- Typing, selecting and scrolling a long field over the material, for stale pixels.
- Hovering a disabled option, pressed states, focus and error states.
- Night mode switched with a settings page open.
- Task 4 Step 2: the folder tabs, checkboxes, radios, toggles, sliders and scroll bars.
- Contrast over a real backdrop, Scale beyond 150%, and RTL.

Also open:

- Field lines and slider tracks keep their colors below 3:1, as the user decided.
- `custom_field_object` still blends quote colors with the field's `textBg` value.
