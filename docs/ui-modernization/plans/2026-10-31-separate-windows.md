# Separate Windows Implementation Plan (C19)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep every separate window opaque, as the user decided, while the restore shell stops flashing an upstream window, and Telegram's notifications and separate panels take the popup shape, as the user chose.

**Architecture:** Separate windows are top-level windows of their own, outside the main window's panels: the media viewer, calls, separate panels (`Ui::SeparatePanel`), Instant View, Telegram's notifications and the restore shell. Their content is media, video, web views or calls, so they stay opaque and get no material. Only three things change. First, the restore shell is a placeholder shown in place of a main window while it reopens, so it takes that window's material, Window role and title painter. Second, notifications become rounded cards with the Popup role's border. Third, separate panels get the popup radius through one fork hook. Everything else is checked to be unchanged.

**Tech Stack:** C++20, Qt 6.11 Widgets, `window/window_restore_shell.*`, `window/notifications_manager_default.*`, forked `lib_ui` (`ui/widgets/separate_panel.cpp`), `Platform::Material::ApplyBackdrop`.

**Spec:** [architecture.md, C19](../architecture.md#components); [shared elements, C19](../shared-elements.md#c19-separate-windows); the user's direction that separate windows (media viewer, calls, Mini Apps, notifications) stay opaque.

**Visual reference:** [C19 separate windows](../component-references/c19-separate-windows/README.md): notifications and a Mini App panel today and in the popup shape, day and night, the restore shell before and after, and the inventory of separate windows.

**Depends on:** [material state](2026-10-29-material-state.md) (`Capable()`, `ActiveValue()`), [surface roles](2026-10-29-surface-roles.md) (`PaintSurface`), [title bar](2026-10-29-title-bar.md) (the title painter hook), [material fills](2026-10-29-material-fills.md) (`Border`), [transient surfaces](2026-10-29-transient-surfaces.md) (`Ui::PopupPanelRadius()`). Comes after C14 and before [info and settings](2026-10-29-info-settings.md), whose final pass checks these windows.

## Global Constraints

- All [ground rules](README.md#ground-rules), including the `lib_ui` recheck rule.
- No separate window gets material or a translucent fill, except the restore shell, which stands in for a main window.
- Media viewer, picture-in-picture, stories, calls, group calls and the screen-share chooser, Instant View, its editor and the markdown viewer are not changed. Their title bars keep their own look; C8's painter is set only on main windows.
- Windows native toasts stay the system's. Only Telegram's own notifications (`Window::Notifications::Default`) change, taking the popup shape (user decision 2026-10-31).
- Popup shape: radius `Ui::PopupPanelRadius()` (12 under Soft Panels), opaque `notificationBg` or `windowBg` fill, a `st::lineWidth` border in `Ui::Material::Border(Surface::Popup)`. Sizes, positions, spacing and animations stay Telegram's.
- Notifications and separate panels change only under `Ui::Material::SoftPanels()`; with Soft Panels off and on macOS and Linux they are unchanged.

## Review Focus

- A notification fading in and out (`setWindowOpacity`) after it becomes translucent: no black or white corners at any opacity. Pinned by Task 2 Step 5.
- Replying from a notification: the reply field area (`Background`) must keep the rounded bottom corners. Pinned by Task 2 Step 3 and Step 5.
- A Mini App whose web view reaches the panel's bottom edge, with no bottom bar: the web view's square corners must not stick out of a radius 12 frame. Pinned by Task 3 Step 1.
- Restoring several separate chat windows at startup with material on: no white shell flashes before each window. Pinned by Task 1 Step 3.
- Night theme notifications: today's border is `windowShadowFgFallback`, the same color as `windowBg`, so the card has no edge. Pinned by Task 2 Step 5.

---

### Task 1: Restore shell

**Files:**
- Modify: `Telegram/SourceFiles/window/window_restore_shell.cpp`

**Interfaces:**
- Consumes: `Window::Material::Capable()`, `Active()` and `ActiveValue()` (C2); `Platform::Material::ApplyBackdrop(HWND, bool)` (C1); `Ui::Material::PaintSurface` and `Surface::Window` (C4); `RpWindow::setTitleBackgroundPainter` (C8).

- [ ] **Step 1: Create the shell like a main window**

`RestoreShell` creates `Ui::RpWindow` with no arguments. When `Window::Material::Capable()`, create it translucent (`RpWindow(true)`, the constructor C1 uses for the main window) and, once the handle exists, call `ApplyBackdrop` with the night state, as `MainWindow::initHook()` does. If the shell can be created before `Capable()` is decided, recheck the order and record it in the commit body.

- [ ] **Step 2: Paint the Window role and the title**

In the content's `paintRequest()`, replace `p.fillRect(clip, st::windowBg)` with `PaintSurface(p, clip, Surface::Window, st::windowBg)`, repaint on `ActiveValue()` changes, and set the same title background painter `MainWindow::initHook()` sets (C8). The loading animation and the "unavailable" label keep their colors; the label's `windowSubTextFg` is already adjusted by C3.

- [ ] **Step 3: Build and check**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`. Open two separate chat windows, quit, and start again with material on, over the Bright sky and Night wallpapers: each shell shows the material and then the restored window without a white frame. Repeat with material off: the shell is the upstream white or night window.

- [ ] **Step 4: Commit**

```bash
git add Telegram/SourceFiles/window/window_restore_shell.cpp
git commit -m "Showed the window material in the restore shell."
```

---

### Task 2: Notifications in the popup shape

**Files:**
- Modify: `Telegram/SourceFiles/window/notifications_manager_default.h`, `notifications_manager_default.cpp`

**Interfaces:**
- Consumes: `Ui::PopupPanelRadius()` (C14), `Ui::Material::Border(Surface::Popup)` (C3), `Ui::Material::SoftPanels()` (C7).
- Produces, in `Window::Notifications::Default::internal`: `[[nodiscard]] QPainterPath CardShape(QRect rect);` — the rounded rect inset by half the border width, radius `Ui::PopupPanelRadius()`.

- [ ] **Step 1: Recheck the paint sites**

Read `Widget::Widget`, `Background::paintEvent`, `Notification::paintEvent` and its cache (`_cache`, filled with `notificationBg` in `Notification::updateNotifyDisplay`, which also draws the square `notifyBorder` lines into it), `Notification::repaintText`, and `HideAllButton::paintEvent`. Record in the commit body which of them fill the full rectangle.

- [ ] **Step 2: Translucent card window**

In `Widget::Widget`, under `SoftPanels()`, set `Qt::WA_TranslucentBackground` instead of `Qt::WA_OpaquePaintEvent`. Keep the window flags, including `Qt::NoDropShadowWindowHint`: notifications have no shadow today, and the card adds none.

- [ ] **Step 3: Paint the shape**

Under `SoftPanels()`:
- `Notification::updateNotifyDisplay` skips the square `notifyBorder` lines. `Notification::paintEvent` fills `CardShape(rect())` with `st::notificationBg`, draws `_cache` clipped to it, and strokes it with `Border(Surface::Popup)`. The cache is still built opaque; only its drawing is clipped.
- `Background::paintEvent` (the reply area) fills only its part of the shape, so its bottom corners stay rounded, and draws no border of its own.
- `HideAllButton::paintEvent` fills `CardShape(rect())` with `lightButtonBg` or `lightButtonBgOver` and strokes the same border.

- [ ] **Step 4: Build**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`

- [ ] **Step 5: Check**

With Soft Panels on, Telegram's notifications on (Settings > Notifications > Use Windows notifications off), day and night, over a dark and a bright desktop: receive one and three messages, hover (the Reply button shows), reply from the notification, click Hide all, and let them fade out. The corners stay round and clean at every opacity, the night card has a visible edge, and positions and spacing match Soft Panels off. Repeat at 150 % and 200 % scale and on a second monitor with a different DPI.

- [ ] **Step 6: Commit**

```bash
git add Telegram/SourceFiles/window/notifications_manager_default.h Telegram/SourceFiles/window/notifications_manager_default.cpp
git commit -m "Rounded Telegram's notifications under Soft Panels."
```

---

### Task 3: Separate panel radius

**Files (in the `lib_ui` fork):**
- Modify: `ui/widgets/separate_panel.h`, `ui/widgets/separate_panel.cpp`

**Files (tdesktop):**
- Modify: `Telegram/SourceFiles/window/window_soft_panels.cpp`

**Interfaces:**
- Produces, in `namespace Ui` (fork, `ui/widgets/separate_panel.h`): `void SetSeparatePanelRadius(int radius);` and `[[nodiscard]] int SeparatePanelRadius();` — process-wide; `0` (the default) means `st::callRadius`, so upstream is unchanged.

- [ ] **Step 1: Recheck the corners at the pinned commit**

`SeparatePanel` reads `st::callRadius` (6 px) in `setBottomBarHeight` (`Expects(!height || height >= st::callRadius)`), `validateBorderImage`, `paintShadowBorder`, `paintBodyBg` and `paintOpaqueBorder`. Read them, and find where a host's content (the Mini App web view in `attach_bot_webview.cpp`, the wallet, payments and passport panels) ends at the bottom when the bottom bar height is 0. If any content reaches a bottom corner, a 12 px radius would show its square corner: stop, record it, and ask the user to choose between keeping 6 px or insetting that content. Check that every `setBottomBarHeight` caller passes 0 or at least 12 px (`walletRowsHintHeight`, the Mini App footer).

- [ ] **Step 2: Add the hook and use it**

Replace each `st::callRadius` read above with `SeparatePanelRadius()`, including the `Expects`. Keep the shadow (`st::callShadow`) and the opaque fallback border.

- [ ] **Step 3: Set it at startup**

In `window_soft_panels.cpp`, next to `Ui::SetSoftControls`, call `Ui::SetSeparatePanelRadius(Ui::Material::SoftPanels() ? st::softPanelsPopupRadius : 0)`.

- [ ] **Step 4: Build and check**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`. With Soft Panels on, open a Mini App with and without its bottom button, the location picker, a payment form with a test bot, Telegram Passport and the export window, day and night. Corners are 12 px with the shadow intact and no square content corner; resizing and full screen work as before. With Soft Panels off, corners are 6 px.

- [ ] **Step 5: Commit in the fork, move the pointer, commit the app**

```bash
git -C Telegram/lib_ui commit -am "Added separate panel radius hook."
git -C Telegram/lib_ui push fork HEAD
git add Telegram/lib_ui Telegram/SourceFiles/window/window_soft_panels.cpp
git commit -m "Gave separate panels the popup radius under Soft Panels."
```

Pushing needs the user's approval.

---

### Task 4: Checks

**Files:**
- Create: `docs/ui-modernization/checks/separate-windows.md`

- [ ] **Step 1: Unchanged windows**

With Soft Panels and material on, open each window in the visual reference's panel D and compare it with a screenshot taken with both off: media viewer with a photo and a video, picture-in-picture, a story, a call and a group call with the screen-share chooser, Instant View, the Instant View editor and the markdown viewer. Each is opaque and identical, and none has C8's title painter.

- [ ] **Step 2: Notifications path**

Turn on Settings > Notifications > Use Windows notifications: native toasts are unchanged. Turn it off: Telegram's notifications have the popup shape.

- [ ] **Step 3: Run the check rows and commit**

Solid parity, Material (restore shell only), Contrast (notification title, text and Hide all), States (notification rest, hover, reply, fade), Scale and RTL. Record in `docs/ui-modernization/checks/separate-windows.md`.

```bash
git add docs/ui-modernization/checks/separate-windows.md
git commit -m "Recorded separate windows checks."
```

## Done when

- The restore shell shows the material when material is active and is unchanged otherwise.
- Notifications and separate panels are rounded with clean corners at every opacity and scale.
- Every other separate window is identical with Soft Panels on and off.
