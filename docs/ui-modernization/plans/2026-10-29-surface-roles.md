# Surface Roles Implementation Plan (C4)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make every background in the main window's navigation and conversation chrome paint through one role helper, so that with material active the backdrop shows through the window, the chats list, the folder rail, the chat header and the composer, and nothing changes with material off.

**Architecture:** A helper paints a role's fill with `CompositionMode_Source` (idempotent, never accumulating) or today's opaque color. A second helper drops `WA_OpaquePaintEvent` while material is active. Existing paint sites get one-line calls; an audit table records every opaque widget and whether it is in a translucent region. The Phase 1 prototype strip is removed.

**Tech Stack:** C++20, Qt 6.11 Widgets, `rpl`, `style::color`.

**Spec:** [architecture.md, Surface roles (C4)](../architecture.md#surface-roles-c4) and [Phase 2](../implementation-plan.md#phase-2-material-foundation).

**Visual reference:** [C4 surface roles](../component-references/c4-surface-roles/README.md): the role of every region in today's layout, the window with material on and off, and the repaint check for `CompositionMode_Source`.

**Depends on:** [material state](2026-10-29-material-state.md) and [material fills](2026-10-29-material-fills.md) (`Active()`, `ActiveValue()`, `Fill(Surface)`, `FillsChanged()`).

## Global Constraints

- All [ground rules](README.md#ground-rules).
- Material fills are painted with `QPainter::CompositionMode_Source`; the painter's previous mode is restored afterwards.
- With `Active()` false, each converted site paints the same `style::color` it painted before.
- In this phase the chat wallpaper stays opaque; the conversation canvas becomes translucent only through the C11 chat background option.
- Separate windows (media viewer, calls, Mini Apps, notifications) are out of scope and stay opaque.

## Review Focus

- Hover and selection in the chats list while material is active: rows must not darken cumulatively as the mouse moves. Pinned by Task 3 Step 5.
- Header animations (search slide, selection mode): `TopBarWidget::setAnimatingMode` toggles the opaque attribute; it must not re-enable it while material is active. Pinned by Task 4 Step 2.
- Palette switch while material is active: every converted widget repaints with the new fill without hover. Pinned by Task 2 Step 3.
- Chat-filter slide animation in the chats list: the slide canvas must not flash opaque. Pinned by Task 3 Step 4 (full grab handling is C5).
- Material turned off live (transparency effects off): every converted widget paints opaque again without a restart. Pinned by Task 5 Step 3.

---

### Task 1: Role painting helpers

**Files:**
- Modify: `Telegram/SourceFiles/ui/ui_material.h`, `ui_material.cpp` (`td_ui`)
- Test: `Telegram/SourceFiles/tests/test_window_material.cpp`
- Modify: `Telegram/cmake/test_window_material.cmake`
- Modify: `Telegram/SourceFiles/window/window_material_policy.{h,cpp}` (remove the prototype fill)

**Interfaces:**
- Consumes: `Ui::Material::Fill(Surface)`, `Active()`, `ActiveValue()`, `FillsChanged()`.
- Produces, in `namespace Ui::Material` (usable from both the `Telegram` and `td_ui` targets):
  - `void PaintFill(QPainter &p, const QRect &rect, const QColor &fill);` and `void PaintFill(QPainter &p, const QPainterPath &shape, const QColor &fill);` — pure, `CompositionMode_Source`, antialiased path; testable without the app.
  - `void PaintSurface(QPainter &p, const QRect &rect, Surface surface, const style::color &solid);` and the `QPainterPath` overload — `PaintFill(p, rect, Fill(surface))` when active, `p.fillRect(rect, solid)` otherwise.
  - `void SetOpaquePaint(not_null<QWidget*> widget, bool opaqueInSolidMode);` — applies `WA_OpaquePaintEvent` as `opaqueInSolidMode && !Active()` now and on every `ActiveValue()` change, for the widget's lifetime.
  - `void RepaintOnFillsChanged(not_null<QWidget*> widget);` — `widget->update()` on every `FillsChanged()`, for the widget's lifetime.

- [ ] **Step 1: Write the failing test**

```cpp
void TestPaintFillReplaces() {
	auto image = QImage(4, 4, QImage::Format_ARGB32_Premultiplied);
	image.fill(QColor(255, 0, 0));
	{
		auto p = QPainter(&image);
		PaintFill(p, QRect(0, 0, 4, 4), QColor(0, 0, 255, 128));
		PaintFill(p, QRect(0, 0, 4, 4), QColor(0, 0, 255, 128));
	}
	const auto pixel = image.pixelColor(1, 1);
	Check(pixel.alpha() == 128, "fill replaces instead of blending");
	Check(pixel.red() == 0 && pixel.blue() == 255, "fill keeps its color");
}
```

Delete `TestPrototypeFill` and its call.

- [ ] **Step 2: Build and confirm failure**

Run: `cmd /c .ai\setup\build-target.cmd test_window_material`
Expected: `'PaintFill': identifier not found`.

- [ ] **Step 3: Implement the four helpers; delete `PrototypeFill` and `kPrototypeFillAlpha` from the policy**

`PaintFill` saves the composition mode, sets `CompositionMode_Source`, fills, restores. Add `ui/ui_material.cpp` to the test target's sources, linking `desktop-app::lib_ui` as `test_text` in `cmake/tests.cmake` does.

- [ ] **Step 4: Run the tests**

Expected: `failures: 0`.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/ui/ui_material.h Telegram/SourceFiles/ui/ui_material.cpp Telegram/SourceFiles/window/window_material_policy.h Telegram/SourceFiles/window/window_material_policy.cpp Telegram/SourceFiles/tests/test_window_material.cpp Telegram/cmake/test_window_material.cmake
git commit -m "Added window material surface painting helpers."
```

---

### Task 2: Window palette and body fill

**Files:**
- Modify: `Telegram/SourceFiles/window/main_window.cpp`

**Interfaces:**
- Consumes: Task 1 helpers.

- [ ] **Step 1: Keep the window palette transparent while active**

In `MainWindow::updatePalette()`, set `QPalette::Window` to `Qt::transparent` when `Ui::Material::Active()`, else `st::windowBg->c` as now. Call `updatePalette()` on every `ActiveValue()` change.

- [ ] **Step 2: Paint the Window role on the body**

Subscribe to `_body->paintRequest()` in the `MainWindow` constructor and paint `PaintSurface(p, clip, Surface::Window, st::windowBg)` only when `Capable()`, so the non-material window is painted exactly as before. Add `RepaintOnFillsChanged(_body)`.

- [ ] **Step 3: Build and check**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`
With material active and a chat open, regions not yet converted stay opaque; any gap between widgets (for example during window resize) shows tinted backdrop, not black. Toggle night mode: no stale colors.

- [ ] **Step 4: Commit**

```bash
git add Telegram/SourceFiles/window/main_window.cpp
git commit -m "Painted the window material role behind the main window body."
```

---

### Task 3: Chats list and folder rail

**Files:**
- Modify: `Telegram/SourceFiles/dialogs/dialogs_widget.cpp`
- Modify: `Telegram/SourceFiles/dialogs/dialogs_inner_widget.cpp`
- Modify: `Telegram/SourceFiles/window/window_filters_menu.cpp`

- [ ] **Step 1: `Dialogs::Widget::paintEvent`**

Remove the Phase 1 prototype branch. Replace the three `p.fillRect(..., bg)` calls (above, between and below the scroll area) with `PaintSurface(..., Surface::Navigation, st::dialogsBg)`. The `bg` brush also takes `st::dialogsBgOver` for a highlighted state; keep that state on the opaque path (material active uses the role fill; highlight restyling is C10). Add `RepaintOnFillsChanged(this)`.

- [ ] **Step 2: `Dialogs::InnerWidget`**

Replace `setAttribute(Qt::WA_OpaquePaintEvent, true)` with `SetOpaquePaint(this, true)`. In `paintEvent`, the base `p.fillRect(rect(), context.currentBg)` becomes `PaintSurface(p, rect(), Surface::Navigation, st::dialogsBg)` when `currentBg()` is the plain list background. Change `InnerWidget::currentBg()` to return `Qt::transparent` while active, so the other `currentBg()` fills and the row painter's background fills become no-ops instead of accumulating. The badge cut-out fill `q.fillRect(cached.badge, st::dialogsBg)` is left for C10; note it in the audit.

- [ ] **Step 3: Folder rail**

In `FiltersMenu::FiltersMenu`, replace `_outer.setAttribute(Qt::WA_OpaquePaintEvent)` with `SetOpaquePaint(&_outer, true)` and paint `_outer` with `PaintSurface(..., Surface::Navigation, ...)`, keeping `st::windowFiltersButton.textBg` as the solid color. The rail and the chats list then show the same fill, as the shared navigation panel of the mockup.

- [ ] **Step 4: Chat-filter slide canvas**

In `Widget::startChatsFilterSlide`, use `SetOpaquePaint(canvas, true)` and `PaintSurface(p, canvas->rect(), Surface::Navigation, st::dialogsBg)`. `grabForChatsFilterSlide` still uses `Ui::GrabOpaque`; C5 replaces it.

- [ ] **Step 5: Build and check**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`
With material active: the rail and list show the backdrop at the Navigation fill, as in panel A of the visual reference; moving the mouse across many rows and back leaves no darkened rows; scrolling leaves no stale pixels; switching folders animates without a black frame (an opaque frame is acceptable until C5).

- [ ] **Step 6: Commit**

```bash
git add Telegram/SourceFiles/dialogs/dialogs_widget.cpp Telegram/SourceFiles/dialogs/dialogs_inner_widget.cpp Telegram/SourceFiles/window/window_filters_menu.cpp
git commit -m "Painted chats list and folder rail with the navigation material."
```

---

### Task 4: Chat header and composers

**Files:**
- Modify: `Telegram/SourceFiles/history/view/history_view_top_bar_widget.cpp`
- Modify: `Telegram/SourceFiles/history/view/controls/history_view_compose_controls.cpp`
- Modify: `Telegram/SourceFiles/history/history_widget.cpp`

- [ ] **Step 1: Header fill**

In `TopBarWidget::paintEvent`, replace the `st::topBarBg` fill with `PaintSurface(..., Surface::Conversation, st::topBarBg)`. Add `RepaintOnFillsChanged(this)`.

- [ ] **Step 2: Header opaque attribute**

Replace the constructor's `setAttribute(Qt::WA_OpaquePaintEvent)` with `SetOpaquePaint(this, true)`. In `setAnimatingMode`, set the attribute to `!_animatingMode && !Ui::Material::Active()`. Open search in a chat and select a message to enter selection mode, with material active: no stale header pixels before, during or after either animation.

- [ ] **Step 3: Composer fills**

In `ComposeControls`'s paint request, replace the `st::historyComposeAreaBg` fill with `PaintSurface(..., Surface::Composer, st::historyComposeAreaBg)`. In `HistoryWidget::drawField`, replace the `st::historyReplyBg` fill with `PaintSurface(..., Surface::Composer, st::historyReplyBg)`.

- [ ] **Step 4: Build and check**

Open an ordinary chat (`HistoryWidget`) and a topic or replies thread (`ComposeControls`), with material active: header and composer areas are translucent; replying, editing and forwarding show the reply strip in the same fill.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/history/view/history_view_top_bar_widget.cpp Telegram/SourceFiles/history/view/controls/history_view_compose_controls.cpp Telegram/SourceFiles/history/history_widget.cpp
git commit -m "Painted chat header and composers with window material roles."
```

---

### Task 5: Opaque-paint audit and checks

**Files:**
- Create: `docs/ui-modernization/checks/opaque-paint-audit.md`
- Create: `docs/ui-modernization/checks/surface-roles.md`

- [ ] **Step 1: Write the audit**

Run `git grep -n "WA_OpaquePaintEvent" -- "Telegram/SourceFiles/*.cpp"` and list every hit in a table: file, widget, region (navigation, conversation, composer, separate window, popup, other), status (converted in this plan, owned by plan C*n*, or stays opaque with a reason). Every widget in the navigation or conversation regions must have an owner plan.

- [ ] **Step 2: Backdrop check**

With material active: `Test-Backdrop.ps1 -Expect Transparent` passes and the changed pixels cover the rail, the chats list, the header and the composer area. With the setting off: `Test-Backdrop.ps1 -Expect Opaque` passes and screenshots of the chats list, a chat and the composer states match pre-plan screenshots.

- [ ] **Step 3: Live fallback**

With material active and a chat open, turn transparency effects off in Windows: every converted region becomes opaque within a second, without a restart; turn them back on: translucency returns.

- [ ] **Step 4: Commit**

```bash
git add docs/ui-modernization/checks/opaque-paint-audit.md docs/ui-modernization/checks/surface-roles.md
git commit -m "Recorded opaque paint audit and surface role checks."
```

## Done when

- `test_window_material.exe` reports `failures: 0`.
- With material active, the rail, chats list, header and composer show the backdrop; the wallpaper area stays opaque.
- With material off, screenshots match the pre-plan ones.
- Every opaque widget in a translucent region has an owner in the audit.
