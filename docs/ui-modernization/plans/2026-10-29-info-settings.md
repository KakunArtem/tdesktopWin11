# Info, Settings and Specialized Screens Implementation Plan (C15)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Bring the info pane, settings, lock and sign-in screens into Soft Panels, run the final accessibility and full-matrix pass, and turn the experimental switches into decided, user-facing settings.

**Architecture:** The info pane and settings are `Info::` sections shown in the third column, the main section or a layer. Their content widget and top bar paint `_bg` fills; under Soft Panels these become the Conversation role inside whichever panel hosts them, or the Popup role when shown as a layer. Lock and sign-in screens cover the window body and use the Window role behind a readable content card. Specialized separate windows (media viewer, calls, Mini Apps, notifications) stay opaque. The last task records the user's default decisions and promotes the Soft Panels switch from an experimental option to a setting.

**Tech Stack:** C++20, Qt 6.11 Widgets, `info/`, `settings/`, `intro/`, `window/window_lock_widgets.*`, `Core::Settings` KV prefs.

**Spec:** [architecture.md, C15](../architecture.md#components); [verification matrix](../implementation-plan.md#verification-matrix-for-every-phase); roadmap Phase 9 and decisions 3–4.

**Visual reference:** [C15 info and settings](../component-references/c15-info-settings/README.md): the info pane in the third column and as a layer, settings rows with and without their resting fill, lock and sign-in cards over the test wallpapers, and the promoted setting.

**Depends on:** [transient surfaces](2026-10-29-transient-surfaces.md), [shared controls](2026-10-30-shared-controls.md) (settings rows, peer lists and dividers), [separate windows](2026-10-31-separate-windows.md) and every earlier plan.

## Global Constraints

- All [ground rules](README.md#ground-rules).
- The info pane in the third column fills the third panel; as a layer it keeps the layer's rounded box shape with the Popup role.
- Settings rows keep their ripple; their resting fill and hover follow C17's mapping.
- Lock and sign-in screens: Window role on the body, content on a centered card with the Popup role and `softPanelsPopupRadius`.
- Media viewer, calls, group calls, Mini Apps and notification windows belong to C19, which keeps them opaque; this plan does not change them, and the check record states their state.
- Promotion keeps the user's current values: a user who had the experimental switch on keeps Soft Panels on.

## Review Focus

- Info pane opened in the third column, then the window narrowed until it moves to a layer: the background switches role without a stale panel. Pinned by Task 1 Step 4.
- Shared media tabs in a profile (`info_profile_tabs_host.cpp` slide grabs): no opaque frames. Pinned by Task 1 Step 3.
- Passcode lock with material on: the unlock field stays readable on the busiest test wallpaper. Pinned by Task 2 Step 3.
- Sign-in with a QR code: the QR stays black on white, never on material. Pinned by Task 2 Step 3.
- Upgrade from a build with the experimental switch on: Soft Panels stays on after promotion. Pinned by Task 3 Step 3.

---

### Task 1: Info and settings sections

**Files:**
- Modify: `Telegram/SourceFiles/info/info_content_widget.cpp`, `info/info_top_bar.cpp`, `info/info_wrap_widget.cpp`
- Modify: `Telegram/SourceFiles/info/profile/tabs/info_profile_tabs_host.cpp`, `info/media/info_media_grid_zoom.cpp`, `info/profile/info_profile_balance_pill.cpp`
- Modify: `Telegram/SourceFiles/settings/settings_intro.cpp`

**Interfaces:**
- Consumes: `Ui::ControlFill` and the soft controls switch from [shared controls](2026-10-30-shared-controls.md), which already maps `SettingsButton`, `PeerListContent` and `BoxContentDivider`.

- [ ] **Step 1: Content and top bar fills**

`ContentWidget::paintEvent` fills `_bg`: paint it with `PaintSurface(..., Surface::Conversation, _bg)` when the content is in a panel, or `Surface::Popup` when its wrap is a layer (`Info::Wrap::Layer`); replace its opaque attribute with `SetOpaquePaint(this, true)`. Same for `TopBar::paintEvent`'s brush fill; keep its existing `_roundRect` rule for the opaque attribute.

- [ ] **Step 2: Settings intro and rows**

`settings_intro.cpp` sets `WA_OpaquePaintEvent` on its wrap: use `SetOpaquePaint`.

Every row is a `Ui::SettingsButton`. The row styles `settingsButton` and `infoProfileButton` derive from `lib_ui`'s `defaultSettingsButton` and are used in 53 files, and each row paints its `textBg` (`windowBg`) under itself. C17 already maps that fill and the `windowBgOver` hover, and turns divider bands into the soft band. Check every top-level settings section for a row style whose `textBg` is not a surface color; such a row keeps its fill, so record it with the reason.

- [ ] **Step 3: Grabs**

The grab audit assigned `info_wrap_widget.cpp`, `info_profile_tabs_host.cpp`, `info_media_grid_zoom.cpp` and `info_profile_balance_pill.cpp` here: convert each animation grab to `Ui::Material::Grab` and paint the role under the animation, as C5 did for sections.

- [ ] **Step 4: Build and check**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`
Open a user, group and channel profile in the third column; switch shared media tabs; zoom the media grid; open Settings and walk every top-level section; narrow the window so the info pane becomes a layer and widen it back.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/info Telegram/SourceFiles/settings/settings_intro.cpp
git commit -m "Brought info and settings sections into Soft Panels."
```

---

### Task 2: Lock and sign-in screens

**Files:**
- Modify: `Telegram/SourceFiles/window/window_lock_widgets.cpp`, `window/window_lock_widgets.style`
- Modify: `Telegram/SourceFiles/intro/intro_widget.cpp`, `intro/intro.style`

**Interfaces:**
- Produces style values: `lockSoftPanelsCard: margins(24px, 22px, 24px, 20px)` and `introSoftPanelsCard: margins(24px, 22px, 24px, 20px)` — card padding around the existing centered content, from panel C of the visual reference; the content's own geometry is unchanged.

- [ ] **Step 1: Lock screens**

`LockWidget::paintEvent` delegates to `LockWidget::paintContent`, which fills `st::windowBg`. Paint the Window role there, then a Popup-role rounded card behind the centered content when `SoftPanels()`. `PasscodeLockWidget::paintContent` calls the base first and then draws the "Enter passcode" header above `_passcode` and the error line below it. The card must enclose both, plus the submit button.

- [ ] **Step 2: Sign-in**

`Intro::Widget::paintEvent` the same way. The QR code step keeps its own white square.

- [ ] **Step 3: Build and check**

Lock with a passcode, unlock, enter a wrong passcode; log out a test account and walk the sign-in steps (phone, code, QR, password, sign-up) using a test account, with material on, over the Night and Bright sky wallpapers.

- [ ] **Step 4: Commit**

```bash
git add Telegram/SourceFiles/window/window_lock_widgets.cpp Telegram/SourceFiles/window/window_lock_widgets.style Telegram/SourceFiles/intro
git commit -m "Brought lock and sign-in screens into Soft Panels."
```

---

### Task 3: Decide defaults and promote the switch

**Files:**
- Modify: `Telegram/SourceFiles/window/window_soft_panels.cpp`, `settings/settings_experimental.cpp`, `settings/sections/settings_advanced.cpp`, `Telegram/Resources/langs/lang.strings`
- Modify, if popup Acrylic is promoted: `Telegram/SourceFiles/window/window_material.cpp`
- Modify: `docs/ui-modernization/README.md`, `docs/ui-modernization/implementation-plan.md`

- [ ] **Step 1: Ask the user**

Present the roadmap's open decisions with the evidence gathered in the check records: transparency on by default or not; Soft Panels on by default or not; material chat background default; and, if `decisions/popup-acrylic.md` says go, whether to promote popup Acrylic. Record the answers in the README's direction section as user decisions with the date, and close them in [Open decisions](../architecture.md#open-decisions).

- [ ] **Step 2: Promote**

Replace the `soft-panels-layout` option with a KV pref `soft_panels_layout` and a checkbox "Use Soft Panels layout" next to "Use window transparency" (restart confirmation as in C2). On first read, if the pref is absent, take the old option's stored value; otherwise the decided default. Remove the option from experimental settings. If the user chose to promote popup Acrylic, replace `popup-material-prototype` the same way, with the KV pref `popup_material` and a checkbox "Use transparency in menus" shown only when `Window::Material::Capable()`.

- [ ] **Step 3: Check upgrade**

Start a build from before this task with the option on, quit, start this build: Soft Panels is on and the checkbox is checked. Repeat with the option off.

- [ ] **Step 4: Commit**

```bash
git add Telegram/SourceFiles/window/window_soft_panels.cpp Telegram/SourceFiles/window/window_material.cpp Telegram/SourceFiles/settings Telegram/Resources/langs/lang.strings docs/ui-modernization/README.md docs/ui-modernization/implementation-plan.md docs/ui-modernization/architecture.md
git commit -m "Promoted Soft Panels layout to a setting."
```

---

### Task 4: Final pass

**Files:**
- Create: `docs/ui-modernization/checks/final.md`

- [ ] **Step 1: Full verification matrix**

Run every row of the [verification matrix](../implementation-plan.md#verification-matrix-for-every-phase) over the main window states from every earlier check record.

- [ ] **Step 2: Accessibility**

Tab through the whole main window, settings and a box: focus is always visible. Run Narrator over the chats list, a chat and the composer: names and roles unchanged from upstream. Turn on high contrast: solid fallback everywhere. Interface scale 200 %: no clipped text.

- [ ] **Step 3: Separate windows**

Open the media viewer, a call, a Mini App and a notification: each matches `checks/separate-windows.md` (C19); note it.

- [ ] **Step 4: Commit**

```bash
git add docs/ui-modernization/checks/final.md
git commit -m "Recorded final Soft Panels verification."
```

## Done when

- Info, settings, lock and sign-in screens pass the check rows.
- The user's default decisions are recorded and implemented.
- The final record covers the whole verification matrix.
