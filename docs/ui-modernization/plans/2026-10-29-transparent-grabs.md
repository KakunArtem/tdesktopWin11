# Transparent Grabs Implementation Plan (C5)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Section changes, folder switches and other slide animations in the main window run over the material without opaque flashes, double-dense frames or black areas.

**Architecture:** While material is active, grabs for animations capture content only: a scope flag makes `PaintSurface` skip role fills during the grab, so caches hold transparent backgrounds. Each animation owner paints its role fill once, then draws the caches over it, so slides and cross-fades move content over a constant material. With material off every grab and animation is unchanged.

**Tech Stack:** C++20, Qt 6.11 Widgets, `Ui::GrabWidget`, `Window::SlideAnimation`.

**Spec:** [architecture.md, Grabs (C5)](../architecture.md#grabs-c5); roadmap risk "Opaque flashes from grabs and slide animations".

**Depends on:** [surface roles](2026-10-29-surface-roles.md) (`PaintSurface`, `PaintFill`, converted paint sites).

## Global Constraints

- All [ground rules](README.md#ground-rules).
- No `lib_ui` change in this plan: `Ui::GrabWidget`, `Ui::GrabOpaque` and `Ui::SlideAnimation` are called, not edited. Recheck their signatures at the pinned commit; the plan assumes `Ui::GrabWidget(widget, rect, background)` with a `QColor` background.
- With `Active()` false, every converted call site produces the same pixmap as before.
- Animation durations and easing are unchanged.

## Review Focus

- Back navigation (Escape from a chat to the list in one-column layout): the arriving list must not appear denser during the slide. Pinned by Task 2 Step 4.
- Cross-fade slides (`Window::SlideAnimation` with fade): overlapping caches must not darken the background at mid-animation. Pinned by Task 2 Step 4.
- Opening a forum topic list from the chats list (child list slide in `Dialogs::Widget`): no black rectangle. Pinned by Task 3 Step 3.
- Grabbing while a palette change is pending: a cache taken during a scope must never be used after material turns off. Pinned by Task 1 `TestGrabScopeNests` and Task 2 Step 2.
- Grabs of popups or separate windows (notifications, reactions selector): unaffected, because they are outside the main window's translucent regions. Pinned by Task 4's audit.

---

### Task 1: Content-only grab scope

**Files:**
- Modify: `Telegram/SourceFiles/ui/ui_material.h`, `ui_material.cpp`
- Test: `Telegram/SourceFiles/tests/test_window_material.cpp`

**Interfaces:**
- Produces, in `namespace Ui::Material` (`td_ui`):
  - `class GrabScope final { public: GrabScope(); ~GrabScope(); };` — non-copyable; increments and decrements a file-local depth counter.
  - `[[nodiscard]] bool Grabbing();` — depth counter above zero.
  - `[[nodiscard]] QPixmap Grab(not_null<QWidget*> widget, QRect rect = QRect());` — when `Active()`: a `GrabScope` around `Ui::GrabWidget(widget, rect, QColor(0, 0, 0, 0))`; otherwise `Ui::GrabWidget(widget, rect)`.
  - `[[nodiscard]] QPixmap GrabOpaque(not_null<QWidget*> widget, QRect rect, const style::color &solid);` — when `Active()`: as `Grab`; otherwise `Ui::GrabOpaque(widget, rect, solid->c)`.
  - `void PaintAnimationBackground(QPainter &p, const QRect &rect, Surface surface, const style::color &solid);` — `PaintSurface` for the area an animation paints, for owners that suppressed their normal paint.
- Changes: `PaintSurface` paints nothing while `Active() && Grabbing()`.

- [ ] **Step 1: Write the failing test**

```cpp
void TestGrabScopeNests() {
	Check(!Grabbing(), "not grabbing initially");
	{
		auto outer = GrabScope();
		Check(Grabbing(), "grabbing inside a scope");
		{
			auto inner = GrabScope();
			Check(Grabbing(), "grabbing inside nested scopes");
		}
		Check(Grabbing(), "outer scope still grabbing");
	}
	Check(!Grabbing(), "not grabbing after scopes end");
}
```

- [ ] **Step 2: Build and confirm failure**

Run: `cmd /c .ai\setup\build-target.cmd test_window_material`
Expected: `'GrabScope': identifier not found`.

- [ ] **Step 3: Implement `GrabScope`, `Grabbing`, `Grab`, `GrabOpaque`, `PaintAnimationBackground` and the `PaintSurface` change in `ui_material.cpp`**

- [ ] **Step 4: Run the tests**

Expected: `failures: 0`.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/ui/ui_material.h Telegram/SourceFiles/ui/ui_material.cpp Telegram/SourceFiles/tests/test_window_material.cpp
git commit -m "Added content-only grabs for window material animations."
```

---

### Task 2: Main section animations

**Files:**
- Modify: `Telegram/SourceFiles/mainwidget.cpp`
- Modify: `Telegram/SourceFiles/window/section_widget.cpp`
- Modify: `Telegram/SourceFiles/history/history_widget.cpp`
- Modify: `Telegram/SourceFiles/history/view/history_view_chat_section.cpp`, `history_view_pinned_section.cpp`, `history_view_scheduled_section.cpp`, `history_view_welcome_messages_section.cpp`
- Modify: `Telegram/SourceFiles/history/admin_log/history_admin_log_section.cpp`

- [ ] **Step 1: Replace the grabs**

Every `Ui::GrabWidget(...)` call in these files that feeds a show, hide or slide animation becomes `Ui::Material::Grab(...)` with the same arguments: in `MainWidget` the slide cache in the function building `SectionSlideParams`, `grabForShowAnimation` (both branches) and `showAnimated`; `SectionWidget`'s grab for show animation; `HistoryWidget`'s grab for show animation and `showAnimated`; and the `grabForShowAnimation` of each section listed. Calls that feed something else (drag area, voice record bar) are left to Task 4's audit.

- [ ] **Step 2: Drop caches when material state changes**

In `MainWidget`, `SectionWidget` and `HistoryWidget`, when `ActiveValue()` changes while `_showAnimation` exists, finish the animation immediately through the owner's existing finish path (`showFinished()` or equivalent). A content-only cache must never be drawn after material turns off.

- [ ] **Step 3: Paint the background under the animation**

Before each `_showAnimation->paintContents(p)` in `MainWidget::paintEvent`, `SectionWidget::paintEvent` and `HistoryWidget::paintShowAnimationFrame`, call `PaintAnimationBackground(p, rect(), Surface::Conversation, st::windowBg)` when `Active()`. In `MainWidget`, the navigation part of a one-column slide uses `Surface::Navigation` and `st::dialogsBg`.

- [ ] **Step 4: Build and check**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`
With material active, record the screen with Win+Alt+R while: opening a chat from the list and pressing Escape in a narrow (one-column) window; opening a chat's info in a normal window; opening pinned and scheduled messages. Step through the recording: no frame shows an opaque or black area, and the background density is constant through each slide and fade. Repeat with material off: identical animations to pre-plan recordings.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/mainwidget.cpp Telegram/SourceFiles/window/section_widget.cpp Telegram/SourceFiles/history
git commit -m "Used content-only grabs for section animations over window material."
```

---

### Task 3: Chats list animations

**Files:**
- Modify: `Telegram/SourceFiles/dialogs/dialogs_widget.cpp`
- Modify: `Telegram/SourceFiles/dialogs/ui/dialogs_suggestions.cpp`
- Modify: `Telegram/SourceFiles/dialogs/ui/dialogs_top_bar_suggestion_content.cpp`

- [ ] **Step 1: Replace the grabs**

In `Dialogs::Widget`: `grabForChatsFilterSlide` uses `Ui::Material::GrabOpaque(_scroll.data(), _scroll->rect(), st::dialogsBg)`; the other `Ui::GrabWidget` calls (show animation, child list slide, search-to-list slide) use `Ui::Material::Grab`. In `dialogs_suggestions.cpp` and `dialogs_top_bar_suggestion_content.cpp`, the slide and collapse snapshots use `Grab`.

- [ ] **Step 2: Paint the background under the animations**

Before each `paintContents(p)` / `paintFrame(...)` call in `Dialogs::Widget` (show animation, child list slide canvas, chat-filter slide canvas) and in the two suggestion widgets, call `PaintAnimationBackground(..., Surface::Navigation, st::dialogsBg)`. The chat-filter slide canvas already paints the role from C4; make sure it is painted exactly once per frame.

- [ ] **Step 3: Build and check**

Record while switching folders in the rail, opening and closing a forum's topic list, opening search and returning, and expanding and collapsing the top-bar suggestion. No opaque, black or darker frames.

- [ ] **Step 4: Commit**

```bash
git add Telegram/SourceFiles/dialogs
git commit -m "Used content-only grabs for chats list animations over window material."
```

---

### Task 4: Grab audit

**Files:**
- Create: `docs/ui-modernization/checks/grab-audit.md`

- [ ] **Step 1: Write the audit**

Run `git grep -n -E "GrabWidget|GrabOpaque|GrabWidgetToImage" -- "Telegram/SourceFiles/*.cpp"` and list every hit: file, function, what the cache is for, region (main window translucent region, popup, separate window, other) and status (converted here, owned by plan C*n* — for example the voice record bar by [composer](2026-10-29-composer.md), info tabs by [info and settings](2026-10-29-info-settings.md), the reactions selector by [transient surfaces](2026-10-29-transient-surfaces.md) — or unaffected with a reason).

- [ ] **Step 2: Commit**

```bash
git add docs/ui-modernization/checks/grab-audit.md
git commit -m "Recorded grab audit for window material."
```

## Done when

- `test_window_material.exe` reports `failures: 0`.
- Section and chats list animations show no opaque or density flashes with material active, and are unchanged with material off.
- Every grab call site has a status and owner in the audit.
