# Foundations Implementation Plan (C18)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep icons readable on the role fills, make the default font the user's decision, and confirm that Soft Panels adds no motion of its own, without changing Telegram's icon shapes, type scale or durations.

**Architecture:** Icons are monochrome masks recolored by palette entries, so their readability is a color problem that C3 already solves for text. This plan appends the grey icon entries to C3's adjusted-entry table: resting icons at 3:1, hover icons at 4.5:1 so hover stays stronger than rest. Typography keeps Telegram's sizes. The font family already has a Default / System / custom setting, and `lib_ui` sizes a system or custom family to Open Sans's x-height. If the user chooses the system font, one pure function makes "Default" mean the system font under Soft Panels, used at startup and in the font box preview. Motion needs no code: the last task checks that every Soft Panels path keeps Telegram's durations and honors power saving.

**Tech Stack:** C++20, Qt 6.11 Widgets, `window/window_material.cpp` (C3's table), `ui/ui_material.*` (C7's `SoftPanels()`), `core/application.cpp`, `ui/boxes/choose_font_box.cpp`, `style::SetCustomFont`, `PowerSaving`.

**Spec:** [architecture.md, C18](../architecture.md#components); [shared elements, C18](../shared-elements.md#c18-foundations); the readable exploration's 13 px type, 600 weight and 120 ms transitions.

**Visual reference:** [C18 foundations](../component-references/c18-foundations/README.md): the mockup's icons, every grey icon group on its role today and raised, day and night, the entry table with contrast before and after, the type scale and the motion values.

**Depends on:** [material fills](2026-10-29-material-fills.md) (the adjusted-entry table and `ReadableOver`), [shell layout](2026-10-29-shell-layout.md) (`SoftPanels()`). Comes after [shared controls](2026-10-30-shared-controls.md), whose entries it appends to the same table, and before the element plans C9–C13: their contrast checks include these icons, and the font decision changes every screenshot after it.

## Global Constraints

- All [ground rules](README.md#ground-rules).
- Icon shapes, sizes and positions stay Telegram's. Only palette entries change, only while material is active, and C3 restores them exactly when it turns off.
- Each entry is listed on its own, including the ones the palette derives from `menuIconFg` or `cancelIconFg`: a theme stores its own value for each.
- Resting entries target 3:1, the icon target. Hover (`...Over`) entries target 4.5:1, so a raised resting icon never ends up stronger than its hover.
- Entries and roles are the ones in [Adjusted palette entries](../architecture.md#adjusted-palette-entries) marked C18.
- Not adjusted, with the reason: `menuIconColor` and `infoIconFg` (both `windowBoldFg`, already strong); `windowSubTextFg`, `windowActiveTextFg` and `lightButtonFg` (C3 and C17 adjust them); `historyToDownFg` (drawn on its own opaque round button); icons on accent or media fills (`activeButtonFg`, `historyFileInIconFg` and similar); separate windows (C19).
- Type sizes stay Telegram's: `fsize` 13 px, box text 14 px (`boxFontSize`), box titles 16 px semibold (`boxTitleFont`). They match the mockup's 13 px base and 600 weight.
- The font family default changes only if the user chooses the system font (Task 2 Step 1). No new strings; changing the family still restarts.
- Motion: no new animation. Durations stay `lib_ui`'s (`universalDuration` 120, `slideDuration` 240, `fadeWrapDuration` 200, `slideWrapDuration` 150). Anything that animates honors `PowerSaving::kAnimations`.

## Review Focus

- A theme whose icon colors are already strong, or set to an accent: unchanged by the adjustment. Pinned by Task 3 Step 1.
- Hovering a raised icon in the day theme: the hover color must be darker than the resting one, not lighter. Pinned by Task 1 `TestIconHoverStronger`.
- A user who picked "Default" before turning Soft Panels on, then picks Open Sans from the list: Open Sans stays after restart. Pinned by Task 2 `TestDefaultFontFamily` and Step 6.
- The font box preview for the Default entry under Soft Panels: it must show the font that will be applied. Pinned by Task 2 Step 6.
- Interface scale at 200 % with the system font: no clipped names, dates or settings labels. Pinned by Task 2 Step 6 and Task 3 Step 2.

---

### Task 1: Readable icon colors

**Files:**
- Modify: `Telegram/SourceFiles/window/window_material.cpp` (C3's adjusted-entry table)
- Test: `Telegram/SourceFiles/tests/test_window_material.cpp`

**Interfaces:**
- Consumes: `Ui::Material::ReadableOver(text, surface, role, dark, target)` and the file-local table of `{ entry, roles, target }` from [material fills](2026-10-29-material-fills.md); `Ui::CountContrast`.

- [ ] **Step 1: Pin the hover order**

Add to the test file and call it from `main()`:

```cpp
void TestIconHoverStronger() {
	using Ui::Material::Surface;
	const auto white = QColor(0xff, 0xff, 0xff);
	const auto rest = Ui::Material::ReadableOver(
		QColor(0x99, 0x99, 0x99), white, Surface::Navigation, false, 3.);
	const auto over = Ui::Material::ReadableOver(
		QColor(0x8a, 0x8a, 0x8a), white, Surface::Navigation, false, 4.5);
	Check(
		Ui::CountContrast(over, white) > Ui::CountContrast(rest, white),
		"a raised hover icon stays stronger than its resting icon");
}
```

It guards the targets: with both at 3:1, `#999999` and `#8a8a8a` reach the same grey and hover would stop showing on the icon.

- [ ] **Step 2: Run it**

Run: `cmd /c .ai\setup\build-target.cmd test_window_material` then `.\out\Debug\test_window_material.exe; $LASTEXITCODE`
Expected: `failures: 0` and `0` (C3's `ReadableOver` already exists).

- [ ] **Step 3: Add the entries**

Append the C18 rows of [Adjusted palette entries](../architecture.md#adjusted-palette-entries) to C3's table, one table row per entry, with its roles and target.

- [ ] **Step 4: Build and check the values**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`. With material active, log the adjusted colors. For Telegram's default themes they match panel C of the visual reference:

| Entry | Day | Night |
| --- | --- | --- |
| `menuIconFg`, `cancelIconFg`, `boxTitleCloseFg` | `#6e6e6e` | `#8c99a4`, `#8c98a1`, `#8c98a1` |
| `menuIconFgOver`, `cancelIconFgOver`, `boxTitleCloseFgOver` | `#535353` | unchanged `#dcdcdc` |
| `dialogsMenuIconFg` / `...Over` | `#6e6e6e` / `#535353` | `#8996a1` / unchanged |
| `sideBarIconFg`, `sideBarIconFgActive` | `#606f7f`, `#0972b0` | `#8097a9`, unchanged `#5eb5f7` |
| `historyComposeIconFg`, `historyReplyCancelFg` / their `...Over` | `#8e8e8e` / `#6f6f6f` | unchanged |
| `emojiIconFg`, `smallCloseIconFg` / `smallCloseIconFgOver` | `#8e8e8e`, `#8f8f8f` / `#707070` | unchanged, `#66727e` / unchanged |

A difference of one step in a channel is rounding; a larger one means the roles or targets differ from Step 3.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/window/window_material.cpp Telegram/SourceFiles/tests/test_window_material.cpp
git commit -m "Kept grey icons readable on the window material."
```

---

### Task 2: Default font family under Soft Panels

**Files:**
- Modify: `Telegram/SourceFiles/ui/ui_material.h`, `ui/ui_material.cpp`
- Modify: `Telegram/SourceFiles/core/application.cpp`, `ui/boxes/choose_font_box.cpp`
- Modify: `docs/ui-modernization/README.md`, `docs/ui-modernization/architecture.md`
- Test: `Telegram/SourceFiles/tests/test_window_material.cpp`

**Interfaces:**
- Produces, in `namespace Ui::Material` (`ui/ui_material.h`):
  - `[[nodiscard]] QString DefaultFontFamily(const QString &chosen, bool softPanels);` — pure: `style::SystemFontTag()` when `chosen` is empty and `softPanels` is true; `chosen` otherwise.
  - `[[nodiscard]] QString FontFamily(const QString &chosen);` — `DefaultFontFamily(chosen, SoftPanels())`.

- [ ] **Step 1: Ask the user**

On Windows 11, take two screenshots of the main window and Settings > Chat settings: Font family set to Default (Open Sans), then to System font (Segoe UI), at 100 % and 150 %. Present them with panel D of the visual reference and ask whether Soft Panels should default to the system font. Record the answer in the README's direction section as a user decision with the date, and close the row in [Open decisions](../architecture.md#open-decisions). If the answer is to keep Telegram's font, commit the docs, mark this task done with no code in the Progress row, and skip to Task 3.

- [ ] **Step 2: Write the failing test**

```cpp
void TestDefaultFontFamily() {
	using Ui::Material::DefaultFontFamily;
	Check(
		DefaultFontFamily(QString(), true) == style::SystemFontTag(),
		"Default is the system font under Soft Panels");
	Check(
		DefaultFontFamily(QString(), false).isEmpty(),
		"Default stays Open Sans without Soft Panels");
	Check(
		DefaultFontFamily(u"Open Sans"_q, true) == u"Open Sans"_q,
		"an explicit family wins");
	Check(
		DefaultFontFamily(style::SystemFontTag(), false)
			== style::SystemFontTag(),
		"an explicit System stays System");
}
```

- [ ] **Step 3: Run it to verify it fails**

Run: `cmd /c .ai\setup\build-target.cmd test_window_material`
Expected: `'DefaultFontFamily': is not a member of 'Ui::Material'`.

- [ ] **Step 4: Implement and apply**

Implement both functions. In `Application::run`, pass `Ui::Material::FontFamily(settings().customFontFamily())` to `style::SetCustomFont`. `SoftPanels()` must already hold the option value there: `base::options` are read in `Launcher` before `Application::run`, so if C7's startup call runs later, call `Ui::Material::SetSoftPanels` with the option value just above `SetCustomFont`. In `choose_font_box.cpp`, `PrepareRequest` sets `.family = Ui::Material::FontFamily(family)`, so the Default entry previews the font it applies. The Settings label keeps showing "Default".

- [ ] **Step 5: Run the tests**

Run: `cmd /c .ai\setup\build-target.cmd test_window_material` then `.\out\Debug\test_window_material.exe; $LASTEXITCODE`
Expected: `failures: 0` and `0`.

- [ ] **Step 6: Build and check**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`. With Soft Panels on and Font family on Default: the interface is Segoe UI, and the font box's Default preview matches it. Choose Open Sans in the list and restart: Open Sans. Choose Default again, turn Soft Panels off and restart: Open Sans. At 100 %, 150 % and 200 %, no name, date, header or settings label is clipped.

- [ ] **Step 7: Commit**

```bash
git add Telegram/SourceFiles/ui/ui_material.h Telegram/SourceFiles/ui/ui_material.cpp Telegram/SourceFiles/core/application.cpp Telegram/SourceFiles/ui/boxes/choose_font_box.cpp Telegram/SourceFiles/tests/test_window_material.cpp docs/ui-modernization/README.md docs/ui-modernization/architecture.md
git commit -m "Used the system font as the Soft Panels default."
```

---

### Task 3: Checks

**Files:**
- Create: `docs/ui-modernization/checks/foundations.md`

- [ ] **Step 1: Icons**

With material on, day and night, over the Night and Bright sky test wallpapers: the chats list header (menu, lock, search cancel), the folder rail, the chat header (search, call, profile), the info pane's top bar, a box's close button, the composer (attach, emoji, bot commands, silent), the emoji panel's category tabs and the reply bar's cancel. Each icon at rest and on hover, measured with the contrast tool. Then load a custom theme whose `menuIconFg` is an accent color: unchanged. Turn material off: every entry is the theme's own value again.

- [ ] **Step 2: Text rendering**

Text on a translucent surface is drawn without ClearType subpixel rendering. Compare chats list names, the chat header title and settings rows on the material against solid mode, at 100 % and 150 %, and with the font Task 2 left as the default. Record whether the grayscale text reads as well; a proposed fix goes to the user, not into this plan.

- [ ] **Step 3: Motion**

List every animation the earlier plans touch: section and folder slides (C5), rail and chat row hover (C9, C10), bar show and hide (C11), menu and popup open (C14), control ripples and toggles (C17). Confirm each keeps its upstream duration and that none is new. Then turn off Settings > Advanced > Battery and Animations > Interface animations: each one jumps to its end state, as upstream does.

- [ ] **Step 4: Run the check rows and commit**

Solid parity, Material, Contrast (every icon group in Step 1), States (rest, hover, pressed, disabled), Scale and RTL. Record in `docs/ui-modernization/checks/foundations.md`.

```bash
git add docs/ui-modernization/checks/foundations.md
git commit -m "Recorded foundations checks."
```

## Done when

- `test_window_material.exe` reports `failures: 0`.
- With material on, every icon group in Task 3 Step 1 reaches 3:1 at rest and hover stays stronger than rest; with material off, every entry is unchanged.
- The font decision is recorded and, if the user chose the system font, implemented.
- The motion check finds no new animation and no animation that ignores power saving.
