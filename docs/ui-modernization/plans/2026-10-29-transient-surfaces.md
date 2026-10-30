# Transient Surfaces Implementation Plan (C14)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give menus, tooltips, emoji and sticker panels, autocomplete, the reaction selector, boxes and toasts one consistent elevated look that fits Soft Panels, and decide with a prototype whether popup windows get native Acrylic.

**Architecture:** Transient surfaces come in two kinds. In-app layers (emoji panel, autocomplete, boxes, toasts) are child widgets of the main window: they get the Popup role and Soft Panels radii through app-side parallel styles. Popup windows (context menus, dropdowns, tooltips) are separate top-level windows owned by `lib_ui`: an optional, separately switched prototype in the fork applies the DWM transient backdrop to them, ending in its own go/no-go record like Phase 1.

**Tech Stack:** C++20, Qt 6.11, forked `lib_ui` (`ui/widgets/popup_menu*`, `ui/widgets/tooltip*`, `ui/layers/`, `ui/toast/`), `chat_helpers/` panels, Win32 DWM.

**Spec:** [architecture.md, C14](../architecture.md#components); exploration CSS `.details` (radius 13, 97 % fill, 0.5 px edge), `.main-menu` and `.menu-item` (radius 8); [research note on desktop Acrylic for transient UI](../research-translucent-explorer11.md#how-this-maps-to-telegram).

**Visual reference:** [C14 transient surfaces](../component-references/c14-transient-surfaces/README.md): the mockup's details popup and menu rows, every in-app layer and the context menu in day and night, and where each surface is changed.

**Depends on:** [toolkit fork](2026-10-29-toolkit-fork.md), the element plans C9–C13 (so the surfaces are judged against the finished main window), and [shared controls](2026-10-30-shared-controls.md), which already maps the fills of buttons, fields and lists inside boxes.

## Global Constraints

- All [ground rules](README.md#ground-rules), including the `lib_ui` recheck rule.
- At 100 % scale: in-app panel and box radius `12`, menu item highlight radius `8`, border `Border(Surface::Popup)`; fills use the Popup role (floor 0.97).
- Default `lib_ui` styles are not changed for all platforms without a user decision; Soft Panels values go into app-side parallel styles where the app chooses the style, and into the fork only behind a runtime flag.
- Popup Acrylic switch: `base::options` toggle id `popup-material-prototype`, scope Windows, off by default, restart required.
- Every surface keeps its keyboard navigation, dismissal (Escape, outside click), and positioning rules.

## Review Focus

- Context menus with submenus near the screen edge: the submenu still flips to the other side, with the new radius. Pinned by Task 3 Step 3 and Task 4 Step 4.
- Emoji panel dragged open from the composer while the third column (tabbed selector in the third section) is enabled: the third-section variant must match the floating one. Pinned by Task 2 Step 3.
- Boxes over material: the layer dim stays, and the box corners show the dim, not a square. Pinned by Task 2 Step 3.
- High-DPI popup menus on a second monitor with a different scale: the native backdrop follows the window to the new monitor. Pinned by Task 4 Step 4.
- Popup Acrylic with transparency effects off: popups fall back to their opaque look immediately. Pinned by Task 4 Step 4.

---

### Task 1: Inventory

**Files:**
- Create: `docs/ui-modernization/checks/transient-surfaces.md`

- [ ] **Step 1: List every transient surface**

For each: class and file, in-app layer or top-level window, style objects it takes, who chooses the style (app call site or `lib_ui` default), and current corner radius. Start from `Ui::PopupMenu`, `Ui::DropdownMenu`, `Ui::Tooltip`, `Ui::ImportantTooltip`, `Ui::Toast`, `Ui::BoxContent` / `Ui::LayerStackWidget`, `ChatHelpers::TabbedPanel`, `ChatHelpers::FieldAutocomplete`, `InlineBots::Layout` results, the reactions selector in `history/view/reactions/`, `Ui::SendButton`'s send menu and the grab audit's popup rows.

- [ ] **Step 2: Commit**

```bash
git add docs/ui-modernization/checks/transient-surfaces.md
git commit -m "Inventoried transient surfaces for Soft Panels."
```

---

### Task 2: In-app layers

**Files:**
- Modify: `Telegram/SourceFiles/chat_helpers/chat_helpers.style`, `chat_helpers/tabbed_panel.cpp`, `chat_helpers/tabbed_selector.cpp`, `chat_helpers/emoji_list_widget.cpp`, `chat_helpers/stickers_list_widget.cpp`, `chat_helpers/gifs_list_widget.cpp`, `chat_helpers/stickers_list_footer.cpp`, `chat_helpers/field_autocomplete.cpp`
- Modify: `Telegram/SourceFiles/ui/chat/chat_style_radius.h`, `chat_style_radius.cpp`
- Modify: `Telegram/SourceFiles/inline_bots/inline_results_widget.cpp`, `Telegram/SourceFiles/ui/filter_icon_panel.cpp`
- Modify: `Telegram/SourceFiles/history/view/reactions/history_view_reactions_selector.cpp`
- Modify: `Telegram/SourceFiles/boxes/boxes.style` and the app call sites that pick box styles, as the inventory lists them

**Interfaces:**
- Produces style values in `chat_helpers.style`: `softPanelsPopupRadius: 12px`; parallel styles named `<upstream name>SoftPanels` for each panel the app constructs, differing only in radius and background fields.
- Produces, in `ui/chat/chat_style_radius.h`: `[[nodiscard]] int Ui::PopupPanelRadius();` — `st::softPanelsPopupRadius` under Soft Panels, `st::emojiPanRadius` otherwise.
- Consumes: `Ui::Material::PaintSurface(p, path, Surface::Popup, solid)`.

- [ ] **Step 1: Choose parallel styles at the app call sites**

For each in-app layer whose style the app passes, pass the Soft Panels variant when `Ui::Material::SoftPanels()`. The emoji panel's radius is not a style field: `st::emojiPanRadius` is read directly in 40 places. Replace the 36 in the transient surfaces with `Ui::PopupPanelRadius()`: `tabbed_panel.cpp`, `tabbed_selector.cpp`, the three list widgets, `stickers_list_footer.cpp`, the reactions selector, `inline_results_widget.cpp` and `filter_icon_panel.cpp`. Leave the QR and IV editor boxes as they are.

- [ ] **Step 2: Paint backgrounds through the Popup role**

Paint the backgrounds with `PaintSurface(..., Surface::Popup, ...)` and stroke the border. Keep existing shadows. Each layer has its own paint sites:
- Emoji panel: `TabbedPanel::paintEvent` paints only the shadow. The background is painted by `TabbedSelector` (`paintBgRoundedPart` with the `_panelRounding` corner pixmaps, the tabs strip in `paintSlideFrame` and `paintContent`) and by each list widget, which fills `st().bg` itself (`EmojiListWidget`, `StickersListWidget`, `GifsListWidget`). Under Soft Panels the lists paint nothing, and `TabbedSelector` paints the whole rounded shape once.
- Autocomplete: `FieldAutocomplete::paintEvent` fills a square `rect()` with `_st.bg`, across `_boundings`, the full chat width. Under Soft Panels, bound it to the composer pill's width (C12) and paint a rounded shape 6 px above the pill.
- Reaction selector: it paints into `_paintBuffer`, which is filled with the opaque `_st.bg`, and caches round frames with `_st.bg` as their background color. A translucent fill needs both to start transparent. Its expanded radius follows `Ui::PopupPanelRadius()`.

- [ ] **Step 3: Build and check**

Open the emoji, sticker and GIF panel from the composer, the tabbed selector in the third column, `@` and `:` autocomplete, the reaction selector on a message, a confirmation box, the forward box and a toast. Material on and off.

- [ ] **Step 4: Commit**

```bash
git add Telegram/SourceFiles/chat_helpers Telegram/SourceFiles/ui/chat/chat_style_radius.h Telegram/SourceFiles/ui/chat/chat_style_radius.cpp Telegram/SourceFiles/inline_bots/inline_results_widget.cpp Telegram/SourceFiles/ui/filter_icon_panel.cpp Telegram/SourceFiles/history/view/reactions Telegram/SourceFiles/boxes
git commit -m "Restyled in-app transient surfaces for Soft Panels."
```

---

### Task 3: Popup menu and tooltip shape in the fork

**Files (in the `lib_ui` fork):**
- Modify: `ui/widgets/widgets.style`, `ui/widgets/popup_menu.cpp`, `ui/widgets/menu/menu_item_base.cpp` (or wherever item highlights paint; recheck)
- Modify: `Telegram/SourceFiles/window/window_soft_panels.cpp` (sets the flag)

**Interfaces:**
- Produces, in `namespace Ui` (fork): `void SetSoftMenus(bool enabled);` and `[[nodiscard]] bool SoftMenus();` — a process-wide flag read by `PopupMenu` and menu items; default `false`, so upstream look is unchanged.

- [ ] **Step 1: Recheck at the pinned commit**

Read `PopupMenu`'s background and shadow painting and the menu item's selected-background painting. Record the functions in the commit message body.

- [ ] **Step 2: Implement**

With `SoftMenus()`, item highlights paint as rounded rects with radius 8 inset by 4 px horizontally (style values in `widgets.style`), and the menu's outer radius uses the Soft Panels value. The app calls `Ui::SetSoftMenus(Ui::Material::SoftPanels())` at startup.

- [ ] **Step 3: Build, check, commit in the fork, move the pointer**

Open chat, message, link and tray-icon context menus, a submenu near the right screen edge, and the send menu. Then:

```bash
git -C Telegram/lib_ui commit -am "Added soft menu highlight shape."
git -C Telegram/lib_ui push fork HEAD
git add Telegram/lib_ui Telegram/SourceFiles/window/window_soft_panels.cpp
git commit -m "Updated lib_ui for soft menus."
```

Pushing needs the user's approval.

---

### Task 4: Popup Acrylic prototype (go/no-go)

**Files (in the `lib_ui` fork):**
- Modify: `ui/platform/win/ui_utility_win.cpp` (or the Windows popup helper the recheck finds)
- Modify: `ui/widgets/popup_menu.cpp`
**Files (tdesktop):**
- Modify: `Telegram/SourceFiles/window/window_material.cpp`, `settings/settings_experimental.cpp`
- Create: `docs/ui-modernization/decisions/popup-acrylic.md`

**Interfaces:**
- Produces, in `namespace Ui::Platform` (fork): `void SetPopupBackdrop(bool enabled);` — when enabled and on Windows, each popup menu window, after its handle exists, gets `DWMWA_SYSTEMBACKDROP_TYPE = DWMSBT_TRANSIENTWINDOW` and `DWMWA_WINDOW_CORNER_PREFERENCE = DWMWCP_ROUND`, skips its own shadow images and paints its background with `menuBg` at the Popup floor alpha.

- [ ] **Step 1: Implement behind the option**

The app calls `Ui::Platform::SetPopupBackdrop(option && Window::Material::Active())` and updates it on `ActiveValue()` changes.

- [ ] **Step 2: Build**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`

- [ ] **Step 3: Backdrop check**

With the option on, open a context menu over a bright window and run `Test-Backdrop.ps1 -Expect Transparent -Window Popup` (C16 adds `-Window`). Record pass or fail.

- [ ] **Step 4: Matrix**

Record in `decisions/popup-acrylic.md`: backdrop visible, corners and shadow correct, submenus, monitor with different DPI, transparency effects off (opaque immediately), high contrast, inactive main window, text quality. Decision: go (keep the option and plan promotion in C15), or no-go (remove the code from the fork and the app in a revert commit).

- [ ] **Step 5: Commit**

```bash
git -C Telegram/lib_ui commit -am "Added popup backdrop prototype."
git -C Telegram/lib_ui push fork HEAD
git add Telegram/lib_ui Telegram/SourceFiles/window/window_material.cpp Telegram/SourceFiles/settings/settings_experimental.cpp docs/ui-modernization/decisions/popup-acrylic.md
git commit -m "Recorded popup Acrylic prototype results."
```

---

### Task 5: Checks

- [ ] **Step 1: Run the check rows**

Solid parity, Material, Contrast (menu items, tooltips, panel tabs, box text), States (open, hover, keyboard navigation, submenu, disabled item, dismiss) for every inventoried surface, Scale and RTL. Record in `docs/ui-modernization/checks/transient-surfaces.md`.

- [ ] **Step 2: Commit**

```bash
git add docs/ui-modernization/checks/transient-surfaces.md
git commit -m "Recorded transient surface checks."
```

## Done when

- Every inventoried surface has a result and every check row passes.
- The popup Acrylic decision is recorded and the code matches it.
- With Soft Panels off, every surface is unchanged.
