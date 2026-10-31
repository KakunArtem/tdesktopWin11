# Shared Controls Implementation Plan (C17)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let buttons, text fields, list rows and settings rows sit on the role fill of the panel or box that holds them, without opaque patches over the material, and keep every control's marks and text readable on that fill.

**Architecture:** A control paints over a region its container already fills. Some controls also fill their own rectangle with the surface color (`windowBg` or `boxBg`). In solid mode that fill matches the container and adds nothing. On the material it shows as an opaque patch. Their hover fill (`windowBgOver`) is opaque too, unlike the translucent hover of the chats list (C10) and rail (C9). One process-wide switch in the `lib_ui` fork, set from `SoftPanels()` at startup, maps those two kinds of fill. A fill equal to the surface color is skipped, and a fill equal to `windowBgOver` becomes the shared translucent hover. Every other fill, ripple, border, mark and text is unchanged, so filled buttons, accent marks and error states keep their look. App-side lists use the same mapping. Three state colors join C3's adjusted entries.

**Tech Stack:** C++20, Qt 6.11 Widgets, forked `lib_ui` (`ui/widgets/buttons.cpp`, `ui/widgets/fields/input_field.cpp`), `boxes/peer_list_box.cpp`, `ui/widgets/multi_select.cpp`, `style::color`.

**Spec:** [architecture.md, C17](../architecture.md#components); [shared elements, C17](../shared-elements.md#c17-shared-controls); the readable exploration's menu switch, composer field and `--hover`.

**Visual reference:** [C17 shared controls](../component-references/c17-shared-controls/README.md): the mockup's switch and field, every control group on a panel and in a box in day and night, before and after, and the contrast of the control colors on the role fills.

**Depends on:** [material fills](2026-10-29-material-fills.md) (adjusted entries), [surface roles](2026-10-29-surface-roles.md), [toolkit fork](2026-10-29-toolkit-fork.md), [shell layout](2026-10-29-shell-layout.md) (`SoftPanels()`). Comes after the [title bar](2026-10-29-title-bar.md) and before the element plans C9–C13, which build their rows, fields and hovers on it; must finish before [transient surfaces](2026-10-29-transient-surfaces.md) and [info and settings](2026-10-29-info-settings.md), which use it too.

## Global Constraints

- All [ground rules](README.md#ground-rules), including the `lib_ui` recheck rule.
- The mapping, under Soft Panels only: a fill whose RGBA equals `windowBg` or `boxBg` is skipped; a fill whose RGBA equals `windowBgOver` becomes the soft hover; every other fill is painted as before.
- Soft hover: `windowFg` at alpha `0.06` in light palettes and `0.07` in dark ones, the hover of C9 and C10 and the exploration's `--hover`. It is recomputed on every palette change.
- The switch is set once at startup from `Ui::Material::SoftPanels()` and defaults to off, so upstream behavior is unchanged with Soft Panels off and on macOS and Linux.
- Shapes, sizes, radii and durations of every control stay Telegram's.
- The toggle knob keeps its `windowBg` fill, as the mockup's switch keeps its knob. The avatar selection ring (`RoundCheckbox` border) stays: it sits on the avatar, and its boxes have the 0.97 Popup fill.
- Adjusted entries at the roles Navigation, Conversation and Popup: `checkboxFg` and `activeLineFg` at 3:1, `lightButtonFg` at 4.5:1.
- Field bottom lines (`inputBorderFg`) and slider tracks (`sliderBgInactive`) keep their colors until the user decides; they miss 3:1 in today's solid look too.
- Divider bands (`boxDividerBg`, which is `windowBgOver` in both default themes) go through the same mapping, so they become a soft band of the same tint as the hover.

## Review Focus

- A button or field whose surface-colored fill sits on something that is not a role fill, such as the chat wallpaper or a colored header: it would lose its background. Pinned by Task 4 Step 1's audit.
- A text field typed into, scrolled and resized over the material: `InputField` stops being opaque, so no stale pixels may appear. Pinned by Task 1 Step 6.
- A disabled settings option (`settingsOptionDisabled` sets `textBgOver: windowBg` to turn off hover): hovering shows nothing. Pinned by Task 1 `TestControlFill` and Step 6.
- A peer list row with a status icon, which is cached as an image: no opaque square behind the icon. Pinned by Task 2 Step 2.
- Night mode switched while a settings page is open: the soft hover follows the new palette. Pinned by Task 1 Step 6.

---

### Task 1: Soft controls mapping in the fork

**Files (in the `lib_ui` fork):**
- Create: `ui/widgets/soft_controls.h`, `ui/widgets/soft_controls.cpp`; register both in `lib_ui`'s `CMakeLists.txt` next to `ui/widgets/buttons.cpp`
- Modify: `ui/widgets/buttons.cpp`, `ui/widgets/fields/input_field.cpp`, `ui/widgets/box_content_divider.cpp`

**Files (tdesktop):**
- Modify: `Telegram/SourceFiles/window/window_soft_panels.cpp` (sets the switch and the hover)
- Modify: `Telegram/cmake/test_window_material.cmake`
- Test: `Telegram/SourceFiles/tests/test_window_material.cpp`

**Interfaces:**
- Produces, in `namespace Ui` (fork, `ui/widgets/soft_controls.h`):
  - `struct SoftControlColors { QColor windowBg; QColor boxBg; QColor windowBgOver; QColor hover; };`
  - `[[nodiscard]] QColor ResolveControlFill(const QColor &fill, const SoftControlColors &colors);` — pure: a fully transparent color when `fill` is opaque and equals `windowBg` or `boxBg`; `colors.hover` when it equals `windowBgOver`; `fill` otherwise.
  - `void SetSoftControls(bool enabled);`, `[[nodiscard]] bool SoftControls();` and `void SetSoftControlHover(QColor hover);` — process-wide; off by default.
  - `[[nodiscard]] QColor ControlFill(const style::color &fill);` — `fill->c` while off; otherwise `ResolveControlFill` with `st::windowBg`, `st::boxBg`, `st::windowBgOver` and the hover.

- [ ] **Step 1: Write the failing test**

Add `ui/widgets/soft_controls.cpp` to the test target's sources and call the test from `main()`:

```cpp
void TestControlFill() {
	const auto colors = Ui::SoftControlColors{
		.windowBg = QColor(0xff, 0xff, 0xff),
		.boxBg = QColor(0xf4, 0xf6, 0xf8),
		.windowBgOver = QColor(0xf1, 0xf1, 0xf1),
		.hover = QColor(0, 0, 0, 15),
	};
	Check(
		Ui::ResolveControlFill(colors.windowBg, colors).alpha() == 0,
		"window color is skipped");
	Check(
		Ui::ResolveControlFill(colors.boxBg, colors).alpha() == 0,
		"box color is skipped");
	Check(
		Ui::ResolveControlFill(colors.windowBgOver, colors) == colors.hover,
		"opaque hover becomes the soft hover");
	const auto filled = QColor(0x40, 0xa7, 0xe3);
	Check(
		Ui::ResolveControlFill(filled, colors) == filled,
		"a filled button keeps its fill");
	const auto translucent = QColor(0xff, 0xff, 0xff, 0x80);
	Check(
		Ui::ResolveControlFill(translucent, colors) == translucent,
		"a translucent fill is unchanged");
}
```

- [ ] **Step 2: Build and confirm failure**

Run: `cmd /c .ai\setup\build-target.cmd test_window_material`
Expected: CMake cannot find `ui/widgets/soft_controls.cpp`.

- [ ] **Step 3: Recheck the paint sites at the pinned commit**

Read `RoundButton::paintEvent`, `FlatButton::paintEvent`, `SettingsButton::paintBg`, the `InputField` constructor's `WA_OpaquePaintEvent` rule, `InputField::paintFlatSurrounding`, `InputField::paintRoundSurrounding` and `BoxContentDivider::paintEvent`. The planning survey read them at `79c846fd`; name any difference in the fork commit's body.

- [ ] **Step 4: Implement the mapping and apply it**

Implement the functions. Then paint `ControlFill(color)` where each site painted `color`, skipping the paint when its alpha is 0:
- `RoundButton::paintEvent`: for `_roundRect` (`textBg`) and `_roundRectOver` (`textBgOver`). When `ControlFill` differs from the style color, draw the rounded rect with it directly, as the `_brushOverride` path does, because the cached `RoundRect` holds the style color. Paths with `_brushOverride` are unchanged.
- `FlatButton::paintEvent`: the chosen `bgColor` or `overBgColor`.
- `SettingsButton::paintBg`: the chosen `textBg` or `textBgOver`.
- `InputField`: do not set `WA_OpaquePaintEvent` when `ControlFill(_st.textBg)` is transparent; in `paintFlatSurrounding` paint `ControlFill(_st.textBg)`; in `paintRoundSurrounding` use `Qt::NoBrush` when both `textBg` and `textBgActive` map to transparent, and keep the border pen.
- `BoxContentDivider::paintEvent`: the `_st.bg` fill; the top and bottom shadow icons stay.

- [ ] **Step 5: Set the switch and the hover at startup**

In `window_soft_panels.cpp`, where `Ui::Material::SetSoftPanels` is called before the main window exists, call `Ui::SetSoftControls(Ui::Material::SoftPanels())`. When it is on, set `Ui::SetSoftControlHover` to `st::windowFg` with alpha 0.06, or 0.07 when `Ui::IsDarkPalette(*style::main_palette::get())` is true, now and on every `style::PaletteChanged()`. C14 adds `Ui::SetSoftMenus` next to this call.

- [ ] **Step 6: Run the tests, build and check**

Run: `cmd /c .ai\setup\build-target.cmd test_window_material` then `.\out\Debug\test_window_material.exe; $LASTEXITCODE`
Expected: `failures: 0` and `0`.

Run: `cmd /c .ai\setup\build-target.cmd Telegram`. With Soft Panels and material on: open Settings > Chat settings and Settings > Advanced; open a box with a light button and a field (Settings > My account > Username). No white rectangle sits behind light buttons, fields or rows, and hovered rows show the same tint as hovered chats. Type, select text and scroll a long field: no stale pixels. Hover a disabled option: nothing. Toggle night mode with a settings page open and hover a row. Turn Soft Panels off: screenshots match the pre-plan ones.

- [ ] **Step 7: Commit in the fork, move the pointer, commit the app**

```bash
git -C Telegram/lib_ui add ui/widgets/soft_controls.h ui/widgets/soft_controls.cpp
git -C Telegram/lib_ui commit -am "Added soft controls fill mapping."
git -C Telegram/lib_ui push fork HEAD
git add Telegram/lib_ui Telegram/SourceFiles/window/window_soft_panels.cpp Telegram/cmake/test_window_material.cmake Telegram/SourceFiles/tests/test_window_material.cpp
git commit -m "Let shared controls rest on the Soft Panels surfaces."
```

Pushing needs the user's approval.

---

### Task 2: Lists and multi-select in the app

**Files:**
- Modify: `Telegram/SourceFiles/boxes/peer_list_box.cpp`
- Modify: `Telegram/SourceFiles/ui/widgets/multi_select.cpp`

**Interfaces:**
- Consumes: `Ui::ControlFill` (Task 1).

- [ ] **Step 1: Peer list fills**

In `PeerListContent::paintEvent`, paint the area outside the rows with `ControlFill(_st.item.button.textBg)`. In `PeerListContent::paintRowContent`, paint the row's `bg` (`textBg` or `textBgOver`) through `ControlFill`; with `defaultPeerListButton` that skips the resting fill and turns the `windowBgOver` hover into the soft hover.

- [ ] **Step 2: Cached rows**

Where a row is cached into an image and its status icon area is cleared with `q.fillRect(statusRect, st.button.textBg)`, clear it with `Qt::transparent` in `QPainter::CompositionMode_Source` when `ControlFill` is transparent, so the cached image has no opaque square.

- [ ] **Step 3: Multi-select field**

`MultiSelect` fills its background with `_st.bg` (`boxSearchBg`, which is `boxBg`): paint `ControlFill(_st.bg)`. Its inner `InputField` already follows Task 1.

- [ ] **Step 4: Build and check**

Open the share box, Add members, Contacts and a group's members in the info pane, with Soft Panels and material on. Hover rows, select rows (the round checkboxes), scroll fast, and type in the multi-select field. No row, status icon or field has an opaque rectangle, and hover shows the soft hover.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/boxes/peer_list_box.cpp Telegram/SourceFiles/ui/widgets/multi_select.cpp
git commit -m "Let peer lists rest on the Soft Panels surfaces."
```

---

### Task 3: Readable control colors

**Files:**
- Modify: `Telegram/SourceFiles/window/window_material.cpp` (C3's adjusted-entry table)

- [ ] **Step 1: Add the entries**

Append to C3's table, with roles Navigation, Conversation and Popup: `checkboxFg` at 3:1 (unchecked checkbox and radio marks), `activeLineFg` at 3:1 (the focused field line), `lightButtonFg` at 4.5:1 (light button and tab text). Themes store `lightButtonFg` as its own value, so C3's `windowActiveTextFg` entry does not cover it. The rows are already in [Adjusted palette entries](../architecture.md#adjusted-palette-entries).

- [ ] **Step 2: Build and check the values**

With material active, log the three adjusted colors. For Telegram's default themes they match the visual reference's panel D: day `checkboxFg` `#6e6e6e`, `activeLineFg` `#0075ab`, `lightButtonFg` `#005888`; night `checkboxFg` `#8499ae`, `activeLineFg` `#679bd0`, `lightButtonFg` `#77bfff`. A difference of one step in a channel is rounding; a larger one means the roles or targets differ from this step.

- [ ] **Step 3: Commit**

```bash
git add Telegram/SourceFiles/window/window_material.cpp
git commit -m "Kept checkbox, focus line and light button colors readable on the material."
```

---

### Task 4: Audit and checks

**Files:**
- Create: `docs/ui-modernization/checks/shared-controls.md`

- [ ] **Step 1: Audit surface fills outside the roles**

List every style in `Telegram/SourceFiles/**/*.style` whose `textBg`, `textBgOver`, `bgColor` or `overBgColor` resolves to `windowBg`, `boxBg` or `windowBgOver`, through `lightButtonBg` and the other palette aliases. For each, record where the control sits: on a role fill (fine), or on something else such as the wallpaper, a media preview or a colored header. Give each "something else" an owner plan, or a parallel style with a color the mapping leaves alone.

- [ ] **Step 2: Controls with no fill of their own**

With material on, check that these already work and record the result: the folder tabs strip above the chats list (`chat_filters_tabs_strip.cpp`, ripples and an accent bar only), checkbox and radio marks, toggles (the knob stays filled), sliders and tabs (bars only), scroll bars (translucent black or white) and their top and bottom shadow lines inside rounded panels.

- [ ] **Step 3: Run the check rows**

Solid parity, Material, Contrast (checkbox marks, focused field line, light button text, tab labels), States (rest, hover, pressed, focused, disabled, checked, error for fields) for each group in the visual reference, Scale and RTL. Record in `docs/ui-modernization/checks/shared-controls.md`.

- [ ] **Step 4: Commit**

```bash
git add docs/ui-modernization/checks/shared-controls.md
git commit -m "Recorded shared controls checks."
```

## Done when

- `test_window_material.exe` reports `failures: 0`.
- With Soft Panels and material on, no light button, flat button, field, settings row, peer list row or multi-select shows an opaque patch over a role fill, and their hover matches the chats list.
- With Soft Panels off, every control is unchanged.
- The audit gives every surface fill outside a role an owner.
