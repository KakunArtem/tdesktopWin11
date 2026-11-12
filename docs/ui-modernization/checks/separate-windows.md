# C19 separate windows checks

Date: 2026-11-12. Tasks 1–3 are committed. Task 4 is partial: the code audit is done,
and the side-by-side screenshots with Soft Panels and material off have not been taken.
Plan: [separate windows](../plans/2026-10-31-separate-windows.md). Reference:
[C19 visual reference](../component-references/c19-separate-windows/README.md).

## Inventory

| Window | Code | C19 | Result |
| --- | --- | --- | --- |
| Media viewer, picture-in-picture, stories | `media/view/`, `media/stories/` | Unchanged | Code audit passes, with the secondary text finding below. Not compared on screen. |
| Calls, group calls, screen-share chooser | `calls/`, `calls/group/` | Unchanged | Code audit passes, with the secondary text finding below. Not compared on screen. |
| Instant View, its editor, markdown viewer | `iv/` | Unchanged | Code audit passes, with the secondary text finding below. Not compared on screen. |
| Mini Apps, payments, Passport, wallet, location picker, export | `Ui::SeparatePanel` users | 12 px under Soft Panels | Task 3. The user approved the corners in a real window on 2026-11-12. Payments and Passport were not opened. |
| Boxes over a Mini App | `Ui::StandaloneLayerStack` | 12 px under Soft Panels | Same panel as the Mini App. Not opened. |
| Telegram's notifications and Hide all | `window/notifications_manager_default.*` | Popup shape | Task 2. The user approved the cards in a real window on 2026-11-08. |
| Windows native toasts | `platform/win/notifications_manager_win.*` | Unchanged | The user saw them on 2026-11-08 with "Use Windows notifications" on: drawn by Windows, unchanged. |
| Restore shell | `window/window_restore_shell.*` | Material, Window role, title painter | Task 1. Built. Not seen on screen: it shows only while saved separate windows reopen at startup. |

## Code audit

- No file in `media/view/`, `calls/` or `iv/` reads `Ui::Material`, `Window::Material`,
  `SoftPanels()`, `SetupWindowMaterial`, `setTitleBackgroundPainter`,
  `PopupPanelRadius` or `SeparatePanelRadius`. These windows stay opaque and have no C8
  title painter.
- `Ui::SoftControls()` is read only by `lib_ui`'s popup menu and menu item background,
  so it changes context menus inside these windows (C14), not the windows.
- The `SeparatePanel` users are export, Passport, payments, the Mini App and its
  standalone layer stack, the location picker, the wallet and the two test panels. Task 3
  changes all of them on purpose.
- The `lib_ui` `WM_PAINT` fix (`aecae93`) posts an update request only for windows with
  `WS_EX_NOREDIRECTIONBITMAP`, the translucent Direct3D ones. It can extra-sync such a
  window after an expose; it draws nothing different.

## Findings

- **Secondary text in separate windows follows C3.** While material is active, C3's
  readability pass sets process-wide palette entries such as `windowSubTextFg`. That
  entry is read by `media_view_overlay_widget.cpp`, `calls_group_common.cpp`,
  `iv_editor_box.cpp`, `iv_editor_math_box.cpp` and 26 style entries in `media/view/`,
  `calls/` and `iv/`. In light themes that text is darker than upstream, so these
  windows are not pixel-identical with material on. It reads better, not worse. The user
  chose on 2026-11-12 to keep it rather than scope the adjustment to the main window,
  which would need per-window palettes.
- **Panels inset their content.** The location picker filled down to the bottom
  corners, so with a radius above 6 px a panel without a bottom bar ends its content
  12 px above the edge. The user chose this over keeping 6 px on 2026-11-12. Panel
  windows keep their size, so that content is 12 px shorter.

## Rows

| Row | Result |
| --- | --- |
| Solid parity | Not run: no screenshots with Soft Panels and material off. |
| Material (restore shell only) | Not run on screen. |
| Contrast (notification title, text and Hide all) | Not measured. |
| States (notification rest, hover, reply, fade) | Seen by the user on 2026-11-08: rest, hover and the Reply field. Fade not checked. Not measured. |
| Scale | Not run. |
| RTL | Not run. |
