# Material Fills Implementation Plan (C3)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Derive, from the active palette, a readable translucent fill for each surface role and a matching thin border color, so any theme stays readable on the material without new palette keys.

**Architecture:** A pure calculator (`window/window_material_fills.*`) finds the lowest alpha per role at which text keeps its contrast over black and white backdrops. A palette adjuster raises secondary text entries while material is active, through the readability mechanism Telegram already uses for bubbles, and restores them when material turns off. A cache in the material service recomputes both on palette and state changes.

**Tech Stack:** C++20, Qt 6.11, `style::palette`, `Ui::CountContrast` (`ui/color_contrast.h`), `rpl`.

**Spec:** [architecture.md, Material fills (C3)](../architecture.md#material-fills-c3); values from the [readable exploration](../explorations/README.md).

**Visual reference:** [C3 material fills](../component-references/c3-material-fills/README.md): computed fills per role over black, white and a wallpaper, the adjusted text colors, the border and the inactive state, with a script that reproduces the numbers.

**Depends on:** [material state](2026-10-29-material-state.md) (`Active()`, `ActiveValue()`).

## Global Constraints

- All [ground rules](README.md#ground-rules).
- Contrast target: 4.5:1 for text, or the text's contrast on the opaque surface when that is lower.
- Worst-case backdrops: pure black `#000000` and pure white `#ffffff`.
- Alpha search: steps of 0.01 from the role floor up to 1.0.
- Floors, light / dark: Window 0.72 / 0.74, Navigation 0.78 / 0.82, Conversation 0.79 / 0.81, Bar 0.90 / 0.90, Composer 0.96 / 0.96, Popup 0.97 / 0.97.
- Border color: the primary text color at alpha 0.10, as the exploration's `--edge`.
- No new palette keys; adjusted entries are restored exactly when material turns off.

## Review Focus

- A custom theme whose secondary text equals its background (contrast 1:1): the fill must become opaque and nothing may loop. Pinned by Task 1 `TestUnreadableTheme`.
- Switching night mode while material is active: fills and adjusted entries must follow the new palette, and the old palette's adjustments must not leak into it. Pinned by Task 3 Step 5.
- Turning material off: every adjusted entry returns to its exact original color. Pinned by Task 3 Step 4.
- A palette notification raised by the adjuster itself: it must not trigger another adjustment. Pinned by Task 3 Step 3.
- Accent-colored text on the material (unread counts excluded; they have their own badge fill): must reach 4.5:1 like secondary text. Pinned by Task 2's key list including `windowActiveTextFg`.

---

### Task 1: Fill calculator

**Files:**
- Create: `Telegram/SourceFiles/window/window_material_fills.h`
- Create: `Telegram/SourceFiles/window/window_material_fills.cpp`
- Modify: `Telegram/SourceFiles/ui/ui_material.h` (from C2)
- Modify: `Telegram/cmake/test_window_material.cmake`
- Modify: `Telegram/CMakeLists.txt`
- Test: `Telegram/SourceFiles/tests/test_window_material.cpp`

**Interfaces:**
- Produces, in `namespace Ui::Material` (`ui/ui_material.h`): `enum class Surface { Window, Navigation, Conversation, Bar, Composer, Popup };`
- Produces, in `namespace Window::Material` (`window_material_fills.h`, with `using Ui::Material::Surface;`):
  - `struct FillInputs { QColor surface; QColor text; QColor secondaryText; bool dark = false; };`
  - `[[nodiscard]] float64 Floor(Surface surface, bool dark);` — the Global Constraints values.
  - `[[nodiscard]] QColor Composite(const QColor &over, const QColor &under);` — straight alpha blend, opaque result.
  - `[[nodiscard]] QColor ComputeFill(Surface surface, const FillInputs &inputs);` — `inputs.surface` with the chosen alpha.
  - `[[nodiscard]] QColor BorderColor(const QColor &text);` — `text` at alpha 0.10.

- [ ] **Step 1: Write the failing tests**

Add to `test_window_material.cpp`, include `window/window_material_fills.h`, and call each from `main()`. Expected alphas were computed with the WCAG formula for these exact colors.

```cpp
[[nodiscard]] bool Near(float64 a, float64 b) {
	return qAbs(a - b) < 0.005;
}

void TestMockupFills() {
	const auto light = FillInputs{
		.surface = QColor(0xe9, 0xee, 0xf5),
		.text = QColor(0x13, 0x23, 0x35),
		.secondaryText = QColor(0x3a, 0x4b, 0x5f),
	};
	Check(
		Near(ComputeFill(Surface::Navigation, light).alphaF(), 0.78),
		"mockup light navigation fill is 0.78");
	Check(
		Near(ComputeFill(Surface::Window, light).alphaF(), 0.78),
		"window floor is raised until text is readable");
	const auto dark = FillInputs{
		.surface = QColor(0x1b, 0x28, 0x38),
		.text = QColor(0xf2, 0xf6, 0xfb),
		.secondaryText = QColor(0xc2, 0xcf, 0xdd),
		.dark = true,
	};
	Check(
		Near(ComputeFill(Surface::Navigation, dark).alphaF(), 0.82),
		"dark navigation fill keeps its floor");
	Check(
		ComputeFill(Surface::Navigation, dark).rgb() == dark.surface.rgb(),
		"fill keeps the surface color");
}

void TestDefaultThemeFill() {
	const auto inputs = FillInputs{
		.surface = QColor(0xff, 0xff, 0xff),
		.text = QColor(0x00, 0x00, 0x00),
		.secondaryText = QColor(0x99, 0x99, 0x99),
	};
	Check(
		Near(ComputeFill(Surface::Navigation, inputs).alphaF(), 1.0),
		"secondary text below 4.5 on solid needs an opaque fill");
}

void TestUnreadableTheme() {
	const auto inputs = FillInputs{
		.surface = QColor(0x80, 0x80, 0x80),
		.text = QColor(0x80, 0x80, 0x80),
		.secondaryText = QColor(0x80, 0x80, 0x80),
	};
	Check(
		Near(ComputeFill(Surface::Bar, inputs).alphaF(), 1.0),
		"unreadable theme falls back to an opaque fill");
}

void TestFloorsAndBorder() {
	Check(Near(Floor(Surface::Window, false), 0.72), "window light floor");
	Check(Near(Floor(Surface::Window, true), 0.74), "window dark floor");
	Check(Near(Floor(Surface::Popup, true), 0.97), "popup floor");
	const auto border = BorderColor(QColor(0x13, 0x23, 0x35));
	Check(
		border.rgb() == QColor(0x13, 0x23, 0x35).rgb()
			&& Near(border.alphaF(), 0.10),
		"border is text at 10 percent");
}
```

- [ ] **Step 2: Add the sources to the test target and confirm the build fails**

In `test_window_material.cmake`, add `window/window_material_fills.cpp`, `window/window_material_fills.h`, `ui/color_contrast.cpp` and `ui/color_contrast.h` to the sources and `desktop-app::lib_base` to the link libraries (`test_text` in `cmake/tests.cmake` links it the same way).

Run: `cmd /c .ai\setup\build-target.cmd test_window_material`
Expected: CMake reports `Cannot find source file: window/window_material_fills.cpp`.

- [ ] **Step 3: Implement the calculator**

For each alpha from `Floor(surface, inputs.dark)` upward in 0.01 steps, accept the first alpha where, for both `text` and `secondaryText`, `Ui::CountContrast(color, Composite(fill, backdrop))` is at least `std::min(4.5, Ui::CountContrast(color, opaque surface))` for both backdrops; return alpha 1.0 when none is accepted. Register both files in the `window/` block of `Telegram/CMakeLists.txt`.

- [ ] **Step 4: Run the tests**

Run: `cmd /c .ai\setup\build-target.cmd test_window_material` then `.\out\Debug\test_window_material.exe; $LASTEXITCODE`
Expected: `failures: 0` and `0`. If `TestMockupFills` reports a different alpha, print the computed value, recheck the contrast formula against `Ui::CountContrast` rather than editing the expectation; `compute-fills.py` in the visual reference prints the expected alphas.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/window/window_material_fills.h Telegram/SourceFiles/window/window_material_fills.cpp Telegram/SourceFiles/ui/ui_material.h Telegram/cmake/test_window_material.cmake Telegram/CMakeLists.txt Telegram/SourceFiles/tests/test_window_material.cpp
git commit -m "Added contrast-driven window material fill calculator."
```

---

### Task 2: Secondary text adjustment with restore

**Files:**
- Modify: `Telegram/SourceFiles/ui/chat/chat_theme_readability.h`, `chat_theme_readability.cpp`
- Modify: `Telegram/SourceFiles/window/window_material_fills.h`, `window_material_fills.cpp`
- Test: `Telegram/SourceFiles/tests/test_window_material.cpp`

**Interfaces:**
- Produces, in `namespace Ui` (`chat_theme_readability.h`): `[[nodiscard]] QColor EnsureTextContrast(const QColor &color, const std::vector<QColor> &backgrounds, float64 floor);` — the existing file-local `EnsureContrast` with `nudge = true`, made public.
- Produces, in `namespace Window::Material`:
  - `struct Adjustment { QColor original; QColor adjusted; };`
  - `[[nodiscard]] QColor ReadableOver(const QColor &text, const QColor &surface, Surface role, bool dark, float64 target = 4.5);` — `Ui::EnsureTextContrast(text, { Composite(surface at floor, black), Composite(surface at floor, white) }, target)`. Icons and large text pass `3.`.

The palette entries themselves are changed in Task 3; this task stays pure so it can be unit-tested.

- [ ] **Step 1: Write the failing tests**

```cpp
void TestReadableOver() {
	const auto surface = QColor(0xff, 0xff, 0xff);
	const auto grey = QColor(0x99, 0x99, 0x99);
	const auto readable = ReadableOver(grey, surface, Surface::Navigation, false);
	const auto floor = Floor(Surface::Navigation, false);
	auto fill = surface;
	fill.setAlphaF(floor);
	Check(
		Ui::CountContrast(readable, Composite(fill, QColor(0, 0, 0))) >= 4.5
			&& Ui::CountContrast(readable, Composite(fill, QColor(255, 255, 255))) >= 4.5,
		"adjusted secondary text reads over the navigation floor");
	Check(readable.lightness() < grey.lightness(), "light theme text darkens");
}

void TestReadableUnchanged() {
	const auto strong = QColor(0x13, 0x23, 0x35);
	Check(
		ReadableOver(strong, QColor(0xe9, 0xee, 0xf5), Surface::Navigation, false) == strong,
		"already readable text is unchanged");
}
```

Add `ui/chat/chat_theme_readability.cpp` and `.h` to the test target sources.

- [ ] **Step 2: Build and confirm failure**

Run: `cmd /c .ai\setup\build-target.cmd test_window_material`
Expected: `'ReadableOver': identifier not found`.

- [ ] **Step 3: Publish `Ui::EnsureTextContrast` and implement `ReadableOver`**

`EnsureTextContrast` forwards to the anonymous `EnsureContrast(color, backgrounds, floor, true)`; no behavior change for bubbles.

- [ ] **Step 4: Run the tests**

Expected: `failures: 0`. If `chat_theme_readability.cpp` pulls in `style_core_palette.h` and the test fails to link, add `desktop-app::lib_ui` to the test target, as `test_text` does.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/ui/chat/chat_theme_readability.h Telegram/SourceFiles/ui/chat/chat_theme_readability.cpp Telegram/SourceFiles/window/window_material_fills.h Telegram/SourceFiles/window/window_material_fills.cpp Telegram/cmake/test_window_material.cmake Telegram/SourceFiles/tests/test_window_material.cpp
git commit -m "Added readable text adjustment for window material."
```

---

### Task 3: Palette cache in the material service

**Files:**
- Modify: `Telegram/SourceFiles/window/window_material.h`, `window_material.cpp`
- Modify: `Telegram/SourceFiles/ui/ui_material.h`, `ui_material.cpp`

**Interfaces:**
- Consumes: `ComputeFill`, `ReadableOver`, `BorderColor` (Tasks 1–2); `ActiveValue()` (C2).
- Produces, in `namespace Ui::Material` (`td_ui`, read by both targets):
  - `void SetFills(base::flat_map<Surface, QColor> fills, base::flat_map<Surface, QColor> borders);` — called only by `Window::Material`; fires `FillsChanged()`.
  - `[[nodiscard]] QColor Fill(Surface surface);` — the published fill; the opaque role color when `Active()` is false.
  - `[[nodiscard]] QColor Border(Surface surface);` — `BorderColor` of the role's `text`, also while `Active()` is false.
  - `[[nodiscard]] rpl::producer<> FillsChanged();` — fires after every recomputation, so widgets can repaint.
- `Window::Material` computes the maps below and publishes them with `Ui::Material::SetFills`.

Role inputs, all `st::` palette colors (recheck each name in `lib_ui/ui/colors.palette` at the pinned commit; drop and note any that are missing):

| Role | `surface` | `text` | `secondaryText` |
| --- | --- | --- | --- |
| Window | `windowBg` | `windowFg` | `windowSubTextFg` |
| Navigation | `dialogsBg` | `dialogsNameFg` | `dialogsTextFg` |
| Conversation | `windowBg` | `windowFg` | `windowSubTextFg` |
| Bar | `topBarBg` | `windowFg` | `windowSubTextFg` |
| Composer | `historyComposeAreaBg` | `windowFg` | `historyComposeAreaFgService` |
| Popup | `menuBg` | `menuFg` | `windowSubTextFg` |

Entries adjusted while active, each against the Navigation and Conversation floors at 4.5:1: `windowSubTextFg`, `dialogsTextFg`, `dialogsDateFg`, `dialogsTextFgService`, `windowActiveTextFg`, `historyComposeAreaFgService`. Keep them in a file-local table of `{ const style::color *entry; std::vector<Surface> roles; float64 target; }`, so later plans append rows instead of changing code; the full list is in [Adjusted palette entries](../architecture.md#adjusted-palette-entries).

- [ ] **Step 1: Implement the cache**

On `ActiveValue()` becoming true: save each listed entry's color into a file-local `base::flat_map<const style::color*, Adjustment>`, set each to `ReadableOver` the strictest floor among its roles at its target (both backdrops), then compute fills. On `false`: write every saved `original` back, clear the map, compute opaque fills. Change colors through the same setter `chat_theme_readability.cpp` uses; then call `style::NotifyPaletteChanged()` once.

- [ ] **Step 2: Recompute on theme changes**

Subscribe to `style::PaletteChanged()`. When a notification arrives while the file-local `bool adjusting` is false: forget saved originals (the new palette replaced them), re-run Step 1's activation path if active, and fire `FillsChanged()`.

- [ ] **Step 3: Guard against self-notification**

Set `adjusting = true` around the adjuster's own `NotifyPaletteChanged()` call. Verify with a temporary log in `PaletteChanged()`'s handler: one theme switch produces exactly one recomputation.

- [ ] **Step 4: Check restore**

Build. With material active, open Settings > Chat settings and screenshot the chat list; turn transparency effects off in Windows (material becomes inactive live) and compare the subtitle color with a screenshot taken with the setting off: identical pixels.

- [ ] **Step 5: Check night mode**

With material active, toggle night mode twice. Log `Fill(Surface::Navigation)` after each switch: the light values repeat exactly after returning.

- [ ] **Step 6: Commit**

```bash
git add Telegram/SourceFiles/window/window_material.h Telegram/SourceFiles/window/window_material.cpp Telegram/SourceFiles/ui/ui_material.h Telegram/SourceFiles/ui/ui_material.cpp
git commit -m "Applied window material fills and text adjustments to the palette."
```

## Done when

- `test_window_material.exe` reports `failures: 0`.
- `Fill()` and `Border()` follow palette, night mode and material state changes.
- Turning material off restores every adjusted entry exactly.
