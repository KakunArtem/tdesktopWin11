# Popup Acrylic prototype results

Date: 2026-11-10. Component: C14 Task 4 of the
[transient surfaces plan](../plans/2026-10-29-transient-surfaces.md). Build:
Debug, on top of `5f482724e9` with the prototype uncommitted at the time of
the run. Windows: `Microsoft Windows [Version 10.0.26300.9711]`, one
2560×1440 display. GPUs: AMD Radeon(TM) Graphics; NVIDIA GeForce RTX 5080.

Checks ran from scripts that right-click a chat row in the running Debug
Telegram, measure the menu window and run `tools/Test-Backdrop.ps1` on it.
Captures show a real account and are kept out of the repository.

## What the prototype does

- `Ui::Platform::SetPopupBackdrop` in the `lib_ui` fork
  (`ui/platform/ui_popup_backdrop.*`) is set by the app from the
  `popup-material-prototype` option and `Window::Material::Active()`.
- A popup menu created while it is on primes the QRhi backing store, so Qt
  composes it through DirectComposition instead of making it a layered
  window. It then gets the transient (Acrylic) system backdrop, rounded DWM
  corners and the DWM shadow, and drops Telegram's drawn shadow and rounding
  overlay.
- The menu, its items and separators paint `itemBg` at the fill opacity
  `kPopupBackdropFillOpacity` (80 %) in `window/window_material.cpp`.
- Menus that reserve space around themselves for the reaction strip
  (`PopupMenu::setAdditionalMenuPadding`) keep the drawn look, because the
  backdrop would fill the reserved area as a frosted frame.

## Matrix

| Check | How | Result |
| --- | --- | --- |
| Not layered | `WS_EX_LAYERED` on the menu and a submenu | Pass: clear on both, so DWM applies the backdrop. |
| Backdrop visible | `Test-Backdrop.ps1 -Window Popup -Expect Transparent` with a solid red, then blue, window behind the menu | Fail against the plan's threshold: 3 of 5 934 samples changed; a direct comparison of the two captures differs by 0 levels over 5 628 samples. Windows' own Acrylic tint and the 80 % fill hide what is behind. |
| Corners and shadow | Captures in day and night themes | Pass: Windows draws the rounded corners and shadow. |
| Submenus | Hover "Mute notifications" | Pass: the submenu opens as a separate non-layered window with the backdrop. |
| Message menu with reactions | Right-click a message | Pass after the fix: the reserved reaction space no longer shows a frosted frame; the user confirmed on 2026-11-09. |
| Text quality | Share of colored-edge text pixels in the menu | Grayscale anti-aliasing (0 % colored edges). Not compared with menus before the prototype, which were translucent layered windows. |
| Fill and contrast | Menu fill sample | `246,246,246` in the day theme: effectively opaque, so text contrast matches the solid menu. |
| Monitor with different DPI | Move a menu between monitors | Not run: one display. |
| Transparency effects off | Windows Settings | Not run: changes a Windows setting. New popups read the switch when created, so they open in the drawn look once material is inactive. |
| High contrast | Windows Settings | Not run. |
| Inactive main window | Popup over an inactive window | Not applicable: a popup menu takes focus when it opens. |

## Decision

Go, decided by the user on 2026-11-08 after comparing 97 % and 80 % fills in
a real window, with the fill at 80 %. The option stays experimental and off
by default; whether it becomes a setting is decided in C15 Task 3.

On 2026-11-12 the user found the tray menu glitchy. With the backdrop the
window is frosted at full size at once, so the panel animation showed an
empty frosted box and a second shadow; such menus now show and hide without
it. The menu's top and bottom scroll padding was still filled opaque and
showed as darker bands; it now uses the backdrop fill. The user approved both
in a real window.

What the native backdrop contributes at this fill is Windows' rounded corners
and shadow, not a frost the desktop shows through, as the backdrop row
measures. A thinner fill would let more of it through, at the cost of the
contrast the Popup role protects; revisit it together with the C15 decision.
