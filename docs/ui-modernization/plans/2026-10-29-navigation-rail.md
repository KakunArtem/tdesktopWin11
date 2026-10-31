# Navigation Rail Implementation Plan (C9)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restyle the folder rail and the main menu drawer to the exploration: rounded selected folder with an accent marker, quiet hover, readable labels and badges on the shared navigation material, and a rounded drawer, with every folder, menu and badge behavior kept.

**Architecture:** The rail keeps `FiltersMenu` and `Ui::SideBarButton`. Under Soft Panels the buttons get a parallel style with transparent backgrounds, and `FiltersMenu` paints the selected pill, hover pill and marker on its container, under the buttons. The main menu keeps `Window::MainMenu` in the layer stack and paints a rounded Popup-role shape instead of a full rectangle.

**Tech Stack:** C++20, Qt 6.11 Widgets, `.style` files, `Ui::SideBarButton` (lib_ui, used unchanged if its style exposes the needed fields).

**Spec:** [architecture.md, Element styling](../architecture.md#element-styling-c9c15); exploration CSS `.rail`, `.rail button`, `.folder-badge`, `.main-menu`, `.menu-item`; the [exploration's main menu notes](../explorations/README.md#proposed-changes).

**Visual reference:** [C9 navigation rail](../component-references/c9-navigation-rail/README.md): the mockup rail and drawer, and every button state in light and dark. It was drawn at 72 px, before the 64 px width was adopted.

**Depends on:** [shell layout](2026-10-29-shell-layout.md) (`SoftPanels()`, navigation panel), [surface roles](2026-10-29-surface-roles.md).

## Global Constraints

- All [ground rules](README.md#ground-rules).
- Rail width under Soft Panels: `windowFiltersSoftPanelsWidth` 64 px, the exploration's (user decision 2026-10-31); upstream keeps `windowFiltersWidth` (72 px).
- At 100 % scale: buttons `54` px minimum height (icons-only and main menu already use it), corner radius `10`, `4` px vertical gap, container padding `4, 6, 8` (top, sides, bottom); marker `3 × 20` px, radius `2`, at the rail's left edge, accent color.
- Hover fill: primary text at alpha `0.06` (light) / `0.07` (dark), the exploration's `--hover`.
- Light rail (user decision 2026-10-31): under Soft Panels the rail is part of the navigation panel, so its solid color is `dialogsBg` and its parallel styles take the chats list's colors: labels `windowSubTextFg`, icons `menuIconFg`, and the selected label and icon `windowActiveTextFg`. Each is already adjusted by C3 or C18 while material is active.
- Selected fill: `Ui::Material::SelectedFill()` ([element styling](../architecture.md#element-styling-c9c15)), opaque in solid and material mode, so the adjusted label and icon colors keep their contrast on it. `sideBarBgActive` (`#17212b` in the day theme) is not used under Soft Panels.
- Without Soft Panels the rail keeps its upstream colors; while material is active they are raised by C3 (`sideBarTextFg`, `sideBarTextFgActive`, added here) and C18 (the icons).
- Main menu: width stays `mainMenuWidth`; right corner radius `14`; row radius `8`; Popup role.
- Every behavior stays: folder click, drag reorder in edit mode, context menu, unread counters, muted badges, "Edit" entry, premium lock, tab and keyboard order.

## Review Focus

- Reordering folders by drag (Edit folders mode): the selected pill and marker must follow the moved button during the drag. Pinned by Task 1 Step 6.
- Text-only and icons-only rail modes (`windowFiltersButtonTextOnly`, `windowFiltersButtonIconsOnly`): both need their parallel styles. Pinned by Task 1 Step 2.
- A folder with unread muted chats only: grey badge plus the existing muted icon, never accent. Pinned by Task 2 Step 2.
- Keyboard focus moving through the rail: a visible focus ring on the pill shape. Pinned by Task 1 Step 6.
- Opening the main menu over material: the layer dim must still show outside the drawer's rounded corners. Pinned by Task 3 Step 3.

---

### Task 1: Rail buttons and selection

**Files:**
- Modify: `Telegram/SourceFiles/window/window.style`
- Modify: `Telegram/SourceFiles/window/window_filters_menu.h`, `window_filters_menu.cpp`
- Modify: `Telegram/SourceFiles/window/window_filters_favorite.cpp`
- Modify: `Telegram/SourceFiles/window/window_session_controller.cpp`
- Modify: `Telegram/SourceFiles/window/window_material.cpp` (C3's table)

**Interfaces:**
- Produces style objects: `windowFiltersButtonSoftPanels`, `windowFiltersButtonTextOnlySoftPanels`, `windowFiltersButtonIconsOnlySoftPanels`, `windowFiltersMainMenuSoftPanels` (each inheriting its upstream counterpart with background fields set to `transparent`), and `windowFiltersSoftPanelsRadius: 10px`, `windowFiltersSoftPanelsMarker: size(3px, 20px)`, `windowFiltersSoftPanelsPadding: margins(6px, 4px, 6px, 8px)`, `windowFiltersSoftPanelsSkip: 4px`.
- Produces, in `namespace Window` (`window_filters_menu.h`): `[[nodiscard]] int FiltersMenuWidth();` — `st::windowFiltersSoftPanelsWidth` under Soft Panels, `st::windowFiltersWidth` otherwise.
- Consumes: `Ui::Material::SelectedFill()` from [shared controls](2026-10-30-shared-controls.md).
- Produces, in `FiltersMenu`: `[[nodiscard]] const style::SideBarButton &buttonStyle(ChatsFiltersTabsMode mode) const;` — returns the parallel style when `SoftPanels()`.

- [ ] **Step 1: Recheck `SideBarButton` style fields**

Read `SideBarButton` in `lib_ui/ui/widgets/widgets.style` and its `paintEvent` at the pinned commit. App code already uses `textBg`, `textFg`, `textTop`, `textSkip`, `style`, `minHeight`, `minTextWidth` and `iconPosition`. If the active background is not a style field, add a transparent-background flag in the [fork](2026-10-29-toolkit-fork.md) instead of painting over it, and note it in the commit.

- [ ] **Step 2: Add the parallel styles and choose them in one place**

Every place `FiltersMenu` and `FolderFavoriteButton` pick a `SideBarButton` style goes through `buttonStyle()`. Rebuild buttons when `SoftPanelsValue()` changes, the way the menu already rebuilds on tab mode changes.

- [ ] **Step 3: Paint pills and marker on the container**

In the `_container` paint (add a `paintRequest()` subscription), when `SoftPanels()`: for the hovered button paint the hover fill, for the active folder paint the selected fill, both as rounded rects inset by the padding; paint the marker at x = 0 centered on the active button. `_outer` keeps painting the Navigation role from C4. `FolderFavoriteButton` uses `SetOpaquePaint(this, !SoftPanels())`.

- [ ] **Step 4: Spacing**

When `SoftPanels()`, lay out buttons with `windowFiltersSoftPanelsPadding` and `windowFiltersSoftPanelsSkip`; otherwise unchanged. Replace the three reads of `st::windowFiltersWidth` (`SessionController::filtersWidth`, the width check next to it, and the button width in `window_filters_menu.cpp`) with `FiltersMenuWidth()`. Long folder names elide as they do today; check "Personal" and a long custom name.

- [ ] **Step 5: Light rail colors**

Set the parallel styles' label, icon and selected colors from the light rail constraint and append `sideBarTextFg` and `sideBarTextFgActive` (Navigation, 4.5:1) to C3's table for the rail without Soft Panels. Expected day values over the material: `#485563` and `#005788`.

- [ ] **Step 6: Build and check**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`
Switch folders, hover, open Edit folders and drag a folder, Tab through the rail, switch text-only and icons-only modes in folder settings. Pills and marker follow every change; focus is visible. Compare with panel B of the visual reference.

- [ ] **Step 7: Commit**

```bash
git add Telegram/SourceFiles/window/window.style Telegram/SourceFiles/window/window_filters_menu.h Telegram/SourceFiles/window/window_filters_menu.cpp Telegram/SourceFiles/window/window_filters_favorite.cpp Telegram/SourceFiles/window/window_session_controller.cpp Telegram/SourceFiles/window/window_material.cpp
git commit -m "Restyled folder rail for Soft Panels."
```

---

### Task 2: Badges

**Files:**
- Modify: `Telegram/SourceFiles/window/window.style`

- [ ] **Step 1: Badge ring**

In the parallel styles, keep `badgeHeight` 17 and `badgeStroke` 2. The stroke paints with the button background color; with a transparent background it must use the Navigation fill instead. If `SideBarButton` strokes with its `textBg`, set the stroke color through the style field the recheck in Task 1 found, or draw the ring in the container paint.

- [ ] **Step 2: Muted badges**

Verify that folders whose unread chats are all muted use `sideBarBadgeBgMuted` (existing) and keep the existing muted main-menu icon `windowFiltersMainMenuUnreadMuted`. Screenshot a muted-only folder and an unmuted folder side by side, light and dark.

- [ ] **Step 3: Commit**

```bash
git add Telegram/SourceFiles/window/window.style
git commit -m "Adjusted folder rail badges for Soft Panels."
```

---

### Task 3: Main menu drawer

**Files:**
- Modify: `Telegram/SourceFiles/window/window_main_menu.cpp`
- Modify: `Telegram/SourceFiles/window/window_main_menu.style`

**Interfaces:**
- Produces style values: `mainMenuSoftPanelsRadius: 14px`, `mainMenuSoftPanels: Menu(mainMenu)` with item corner radius `8px` where the `Menu` style supports it (recheck in `widgets.style`).

- [ ] **Step 1: Rounded shape**

In `MainMenu::paintEvent`, when `SoftPanels()`, paint `PaintSurface(p, shape, Surface::Popup, st::mainMenuBg)` where `shape` is `Window::SoftPanels::PanelShape(rect(), st::mainMenuSoftPanelsRadius, Corners::Right)`; otherwise fill as now. In the constructor, use `SetOpaquePaint(this, !SoftPanels())`. Apply the same shape to the cover fill in the cover paint.

- [ ] **Step 2: Rows**

Pass `st::mainMenuSoftPanels` to the menu when `SoftPanels()`. If the `Menu` style has no item radius, paint the row hover as a rounded rect in `MainMenu`'s own row painting, or leave rows square and record the gap.

- [ ] **Step 3: Build and check**

Open the main menu from the rail with material on and off: rounded right corners, layer dim visible outside them, account switching, night mode toggle, Contacts, Calls, Saved Messages and Settings all work; Escape and outside clicks close it.

- [ ] **Step 4: Commit**

```bash
git add Telegram/SourceFiles/window/window_main_menu.cpp Telegram/SourceFiles/window/window_main_menu.style
git commit -m "Rounded the main menu drawer for Soft Panels."
```

---

### Task 4: Checks

**Files:**
- Create: `docs/ui-modernization/checks/navigation-rail.md`

- [ ] **Step 1: Run the check rows**

Solid parity, Material, Contrast (rail labels, badges, main menu rows), States (default, hover, selected, focused, dragging, locked premium folder, muted and unmuted badges), Scale and RTL from the [check rows](README.md#check-rows-used-by-element-plans). Record each result.

- [ ] **Step 2: Commit**

```bash
git add docs/ui-modernization/checks/navigation-rail.md
git commit -m "Recorded navigation rail checks."
```

## Done when

- Every check row passes and is recorded.
- With Soft Panels off, the rail and main menu are unchanged.
