# Shell Layout Implementation Plan (C7)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Lay out the main window as Soft Panels: a navigation panel (folder rail plus chats list) flush with the left edge, a rounded conversation panel and an optional rounded third column, separated by thin gaps over the window material, in one-, two- and three-column layouts.

**Architecture:** A pure layout function computes panel rectangles from the existing column widths. An experimental switch decides whether `MainWindow` and `MainWidget` apply the frame inset and gaps. A mouse-transparent `PanelFrame` overlay per panel paints the corners outside the rounded shape with the Window role and draws the thin border, so no panel child needs clipping. Existing column resizing, animations and adaptive rules are reused unchanged.

**Tech Stack:** C++20, Qt 6.11 Widgets, `.style` codegen, `base::options`.

**Spec:** [architecture.md, Shell geometry (C7)](../architecture.md#shell-geometry-c7); exploration CSS: `.workspace` (gap 4 px, padding 0 6 6 0), `.nav` (radius 0 12 12 0), `.conversation` (radius 14, 0.5 px edge at 10 %).

**Depends on:** [surface roles](2026-10-29-surface-roles.md) and [transparent grabs](2026-10-29-transparent-grabs.md).

## Global Constraints

- All [ground rules](README.md#ground-rules).
- Switch: `base::options` toggle id `soft-panels-layout`, name "Soft Panels layout", scope Windows, off by default, restart required (the window body and columns are built once; a live switch is not worth the risk).
- Sizes at 100 % scale: frame margins `0, 0, 6, 6`; gap `4`; conversation and third radius `14`; navigation right-corner radius `12`; border width `1`.
- Border color: `Ui::Material::Border(Surface::Window)` when material is active, else `st::shadowFg`.
- The column widths, minimum widths and the one/two/three-column thresholds stay Telegram's (`SessionController::computeColumnLayout`); only the frame and gaps are subtracted.
- Column resize handles keep working and sit in the gaps.

## Review Focus

- Window narrowed across the one/two-column threshold while a chat is open: no overlapping panels, no gap left at the old place. Pinned by Task 4 Step 2.
- Dragging the chats list width (and the third column width): the gap and the rounded frame follow every step of the drag. Pinned by Task 4 Step 3.
- The call, export and player top bars: they stay inside the conversation panel's top, with rounded top corners preserved. Pinned by Task 3 Step 3.
- Layers and boxes (settings, media preview): they keep covering the whole body, above the panel frames. Pinned by Task 3 Step 5.
- Maximized and full-screen windows: the frame inset stays, so panels never touch the screen edge at the right or bottom. Pinned by Task 4 Step 4.

---

### Task 1: Switch, style values and pure layout

**Files:**
- Create: `Telegram/SourceFiles/window/window_soft_panels.h`, `window_soft_panels.cpp`, `window_soft_panels.style`
- Create: `Telegram/SourceFiles/window/window_soft_panels_layout.h`, `window_soft_panels_layout.cpp` (pure)
- Modify: `Telegram/SourceFiles/ui/ui_material.h`, `ui_material.cpp`
- Modify: `Telegram/SourceFiles/settings/settings_experimental.cpp`
- Modify: `Telegram/cmake/td_ui.cmake`, `Telegram/CMakeLists.txt`, `Telegram/cmake/test_window_material.cmake`
- Test: `Telegram/SourceFiles/tests/test_window_material.cpp`

**Interfaces:**
- Produces, in `namespace Window::SoftPanels` (`window_soft_panels_layout.h`): `LayoutArgs`, `Geometry` and `[[nodiscard]] Geometry Layout(const LayoutArgs &args);` exactly as in the architecture contract.
- Produces, in `namespace Ui::Material` (`ui/ui_material.h`): `[[nodiscard]] bool SoftPanels();`, `void SetSoftPanels(bool enabled);` (called at startup from `window_soft_panels.cpp`) and and `[[nodiscard]] rpl::producer<bool> SoftPanelsValue();` — the option value on Windows, `false` elsewhere.
- Produces style values in `window_soft_panels.style`: `softPanelsFrame: margins(0px, 0px, 6px, 6px)`, `softPanelsGap: 4px`, `softPanelsRadius: 14px`, `softPanelsNavigationRadius: 12px`, `softPanelsBorderWidth: 1px`.

- [ ] **Step 1: Write the failing tests**

```cpp
using namespace Window::SoftPanels;

void TestTwoColumns() {
	const auto result = Layout({
		.main = QRect(0, 0, 928, 694),
		.dialogsWidth = 300,
		.gap = 4,
	});
	Check(result.dialogs == QRect(0, 0, 300, 694), "dialogs at the left");
	Check(
		result.conversation == QRect(304, 0, 624, 694),
		"conversation after the gap");
	Check(result.third.isEmpty(), "no third column");
}

void TestThreeColumns() {
	const auto result = Layout({
		.main = QRect(0, 0, 1200, 694),
		.dialogsWidth = 300,
		.thirdWidth = 280,
		.gap = 4,
	});
	Check(result.third == QRect(920, 0, 280, 694), "third at the right");
	Check(
		result.conversation == QRect(304, 0, 612, 694),
		"conversation between two gaps");
}

void TestOneColumn() {
	const auto result = Layout({
		.main = QRect(0, 0, 500, 694),
		.dialogsWidth = 500,
		.oneColumn = true,
		.gap = 4,
	});
	Check(result.dialogs == QRect(0, 0, 500, 694), "one column fills");
	Check(result.conversation == result.dialogs, "chat shares the column");
}
```

- [ ] **Step 2: Add sources to the test target, build, confirm failure**

Run: `cmd /c .ai\setup\build-target.cmd test_window_material`
Expected: CMake cannot find `window/window_soft_panels_layout.cpp`.

- [ ] **Step 3: Implement `Layout`; add the switch, the style file and registrations**

Register `window/window_soft_panels.style` in the style list of `cmake/td_ui.cmake` next to `window/window.style`, and the `.cpp`/`.h` files in `Telegram/CMakeLists.txt`. Define the option like the Phase 1 one in `window_soft_panels.cpp`, with `.restartRequired = true`, and call `Ui::Material::SetSoftPanels(option value)` once during startup, before the main window is created (from `Window::Material`'s first-use initialization), and list it in `settings_experimental.cpp` next to `Window::kOptionNewWindowsSizeAsFirst`.

- [ ] **Step 4: Run the tests and build the app**

Expected: `failures: 0`; `Telegram.exe` builds; Experimental settings lists "Soft Panels layout".

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/window Telegram/SourceFiles/ui/ui_material.h Telegram/SourceFiles/ui/ui_material.cpp Telegram/SourceFiles/settings/settings_experimental.cpp Telegram/cmake Telegram/CMakeLists.txt Telegram/SourceFiles/tests/test_window_material.cpp
git commit -m "Added Soft Panels layout switch and geometry."
```

---

### Task 2: Panel frame overlay

**Files:**
- Modify: `Telegram/SourceFiles/window/window_soft_panels.h`, `window_soft_panels.cpp`

**Interfaces:**
- Produces, in `namespace Window::SoftPanels`:
  - `enum class Corners { All, Right };`
  - `[[nodiscard]] QPainterPath PanelShape(QRectF rect, float64 radius, Corners corners);` — rounded rectangle, square left corners for `Right` (pure; add a test that `PanelShape(QRectF(0, 0, 100, 100), 14, Corners::Right)` contains `QPointF(1, 1)` and does not contain `QPointF(99, 1)`).
  - `class PanelFrame final : public Ui::RpWidget { public: PanelFrame(not_null<QWidget*> parent, Corners corners); void setPanelGeometry(QRect panel); };` — sets `WA_TransparentForMouseEvents`, covers the panel rect, paints `PaintSurface(p, outside, Surface::Window, st::windowBg)` where `outside` is the rect minus `PanelShape`, then strokes `PanelShape` with the border color and `st::softPanelsBorderWidth`. Repaints on `FillsChanged()` and `style::PaletteChanged()`.

- [ ] **Step 1: Write the `PanelShape` test, confirm it fails, implement `PanelShape`, run tests**

- [ ] **Step 2: Implement `PanelFrame`**

Paint only within the event's clip; skip all work when the clip lies inside the shape's inner rect (rect shrunk by the radius), so ordinary child repaints cost one rectangle test.

- [ ] **Step 3: Commit**

```bash
git add Telegram/SourceFiles/window/window_soft_panels.h Telegram/SourceFiles/window/window_soft_panels.cpp Telegram/SourceFiles/tests/test_window_material.cpp
git commit -m "Added Soft Panels frame overlay."
```

---

### Task 3: Apply the layout

**Files:**
- Modify: `Telegram/SourceFiles/mainwindow.cpp` (`MainWindow::updateControlsGeometry`)
- Modify: `Telegram/SourceFiles/window/window_filters_menu.cpp`
- Modify: `Telegram/SourceFiles/mainwidget.h`, `mainwidget.cpp`
- Modify: `Telegram/SourceFiles/window/window_session_controller.cpp` (`computeColumnLayout`)

- [ ] **Step 1: Outer frame**

In `MainWindow::updateControlsGeometry` (root `mainwindow.cpp`), when `SoftPanels()`, shrink `_main`'s geometry by `st::softPanelsFrame` (right and bottom); leave `_layer`, `_intro`, `_mediaPreview` and locks on the full body. Subscribe to `SoftPanelsValue()` to re-run geometry. In `FiltersMenu`, subtract `st::softPanelsFrame.bottom()` from the rail height when `SoftPanels()`.

- [ ] **Step 2: Column math**

In `SessionController::computeColumnLayout`, subtract the frame's horizontal margins and one `softPanelsGap` per visible gap from `bodyWidth` when `SoftPanels()`, so the thresholds compare the space panels actually get.

- [ ] **Step 3: Panels in `MainWidget::updateControlsGeometry`**

When `SoftPanels()`: compute `Layout({ .main = rect(), .dialogsWidth, .thirdWidth, .oneColumn = isOneColumn(), .gap = st::softPanelsGap })` and place `_dialogs`, the main section (`_history` or `_mainSection`) and `_thirdSection` at its rectangles; place `_callTopBar`, `_exportTopBar` and `_player` at the top of `conversation` (their existing order); hide `_sideShadow` and `_thirdShadow`; put the first and third column resize areas in the gaps. Otherwise the function is unchanged.

- [ ] **Step 4: Frames**

Add `base::unique_qptr<Window::SoftPanels::PanelFrame>` members for the navigation (`Corners::Right`, around `dialogs`), conversation and third panels (`Corners::All`). Create them when `SoftPanels()` turns on, destroy them when it turns off. Raise them in the function that raises shadows and resize areas, after the panels and before `_connecting`, the float player and `_hider`.

- [ ] **Step 5: Build and check**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`
Turn on "Soft Panels layout". With and without material: panels match the exploration's layout; gaps show the window fill; boxes, settings layers and media preview still cover the whole window; a call bar sits inside the conversation panel's rounded top.

- [ ] **Step 6: Commit**

```bash
git add Telegram/SourceFiles/mainwindow.cpp Telegram/SourceFiles/window/window_filters_menu.cpp Telegram/SourceFiles/mainwidget.h Telegram/SourceFiles/mainwidget.cpp Telegram/SourceFiles/window/window_session_controller.cpp
git commit -m "Applied Soft Panels layout to the main window."
```

---

### Task 4: Layout checks

**Files:**
- Create: `docs/ui-modernization/checks/shell-layout.md`

- [ ] **Step 1: Screenshots**

For one, two and three columns, light and dark, material on and off, at 100 % and 150 % scale, save screenshots with `Capture-Matrix.ps1` (verification tooling) and list them in the record.

- [ ] **Step 2: Threshold**

Narrow the window slowly across the one/two-column threshold with a chat open and back: no overlaps, no stale gaps.

- [ ] **Step 3: Resizing columns**

Drag the chats list edge and the third column edge: the frames follow continuously.

- [ ] **Step 4: Window states**

Maximized, snapped left, full screen, inactive: frame inset present; `Test-Backdrop.ps1 -Expect Transparent` passes in the gaps and panels with material on.

- [ ] **Step 5: Scroll cost**

Scroll a long chat and the chats list quickly with the switch on and off; record whether frame rate visibly differs. If it does, replace the full-panel overlay with eight corner and edge pieces and repeat.

- [ ] **Step 6: Commit**

```bash
git add docs/ui-modernization/checks/shell-layout.md
git commit -m "Recorded Soft Panels layout checks."
```

## Done when

- `test_window_material.exe` reports `failures: 0`.
- With the switch on, all three adaptive layouts show Soft Panels in solid and material modes.
- With the switch off, layout and screenshots match the pre-plan ones.
