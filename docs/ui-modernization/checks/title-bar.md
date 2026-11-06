# C8 title bar checks

Date: 2026-11-06. Local implementation and native raster checks pass.
Fork publication and visible-desktop acceptance remain pending. Plan:
[title bar](../plans/2026-10-29-title-bar.md).

## Implemented behavior

Windows main windows install the opt-in title painter when material is
capable and the custom frame is in use. The title and canary-label
backgrounds use the Window surface. Solid fallback retains `titleBgActive`
and `titleBg` according to window activity.

App-owned copies of `defaultWindowTitle` suppress the toolkit shadow for
Soft Panels or material. The material variant replaces only the resting
caption icons with glyph-only icons; dimensions, hover and pressed styles
come from the upstream title. Both switches off selects the original style.
The app's native-frame-only `_titleShadow` is unchanged.

Four caption foreground entries use C3's 3:1 target and both canary text
entries use 4.5:1 against the Window role. Other platforms ignore the new
toolkit method. The existing title height, caption controls and native
event-handling paths remain unchanged.

## Automated evidence

The standalone `test_window_material` executable was rebuilt and passes
117 checks with zero failures. The Windows Debug app also builds after
restoring the disposable scenario to its original no-op and recompiling it.

The disposable `-testagent` overlay uses empty, isolated profiles and the
real Windows main window, `RpWindow`, title widgets, caption controls and
canary-label helper. No user profile or authorized account was copied.
Windows are shown offscreen to exercise Qt widget visibility; captures are
Qt raster output, not DWM-composited desktop screenshots.

| Startup material | Interface scale | Native passes | Failures | PNGs |
| --- | --- | --- | --- | --- |
| On | 100% | 373 | 0 | 23 |
| Off | 100% | 134 | 0 | 17 |
| On | 150% | 373 | 0 | 23 |
| Off | 150% | 134 | 0 | 17 |

All 1014 native checks pass, including the
[chat background stage](conversation-chrome.md#automated-evidence). Each leg
exercises light and dark palettes,
active and inactive windows, and Soft Panels style switching. Checks cover:

- Custom painter alpha, clearing/restoring the opaque-paint attribute,
  persistence through a native-frame roundtrip, unchanged height and exact
  original raster restoration after clearing the painter.
- Window-role pixels in the open title area and all three resting caption
  buttons, with matching canary-label fills.
- Caption glyph and canary-text contrast over both ends of the measured
  Acrylic range and two measured wallpaper colors. These are color
  calculations, not screen measurements.
- The system menu style: removed with a painter on a custom frame, restored
  under the native frame and when the painter is cleared.
- Actual `WM_NCHITTEST` results for minimize, maximize and close.
- Original hover and pressed fills through the real caption-state APIs,
  with synthetic presses cancelled before release.
- Actual native-frame setting changes and material restoration, plus live
  solid fallback with deliberately different active/inactive title colors.
- A uniform bottom row under Soft Panels and exact raster restoration after
  a Soft Panels style roundtrip.
- The toolkit title shadow in the day theme: absent under material and Soft
  Panels, shown and reaching pixels after a live solid fallback or a Soft
  Panels roundtrip. The night theme cannot show this, because the shadow's
  alpha rounds away on its dark title.

All eight 100% solid title/canary PNGs are byte-identical to captures taken
with the pre-change executable. The initial RED run failed 93 checks for
the absent hook, opaque title and contrast. A corrected complete review RED
run failed 21 resting-caption pixel checks; its corresponding GREEN runs
pass. The fresh review's two paint findings were addressed; final Qt lint
reports zero findings for the app and toolkit changes.

A later validation found that `TitleWidget::setStyle` recreated the shadow
without showing it, so a live fallback lost the shadow until restart. The
day-theme checks above failed before the fix: 2 with material at startup and
4 with solid. They pass after it.

Evidence for the current matrix is under
`%LOCALAPPDATA%\tdesktop-ui-checks\title-bar\`, in folders named
`chatbg-<material|solid>-<100|150>`; each contains `test_log.txt` and
PNGs. The `final-<material|solid>-<100|150>` runs before the shadow fix, the
overlay's execution folder, the build logs and the review notes were written
from a Codex sandbox and are in its package cache instead:
`%LOCALAPPDATA%\Packages\OpenAI.Codex_2p2nqsd0c76g0\LocalCache\Local\tdesktop-ui-checks\title-bar\`.

## Native caption under the translucent title

Date: 2026-11-06. The first real desktop look found a bug the offscreen raster
could not. The toolkit always set the Windows caption color to the title's solid
color, so a 0.50 title composited to a flat 248 in the day theme, and the Windows
caption buttons showed through as doubled glyphs.

With a background painter on a custom frame, the toolkit now clears the caption
color, stops drawing the caption text and icon, and removes `WS_SYSMENU` so
Windows draws no buttons. The native frame and an empty painter restore all of
it. `ShowWindowMenu` adds the style back while the menu is open, and Alt+Space
is routed to it. This is more toolkit change than the plan's one additive API.

Observed on an isolated window on the real desktop, with an empty profile:

| Behavior | Result |
| --- | --- |
| Title responds to the backdrop | Yes: 195 over black, 241 over white |
| Doubled caption buttons | Gone |
| Snap Layouts flyout on maximize hover | Appears |
| Right-click menu on the title | Opens |
| Alt+Space, real keystrokes | Opens the menu |
| `SC_MINIMIZE` | Minimizes |
| Win+Up, real keystrokes | Maximizes |

## Visible-desktop acceptance still pending

These were not observed:

- Clicking the taskbar button to minimize and restore. `SC_MINIMIZE` was sent
  directly, and removing `WS_SYSMENU` could affect the taskbar path.
- Choosing a layout in the Snap Layouts flyout.
- Pointer drag, double-click maximize/restore and resizing from all
  edges/corners.
- Win+Down, Win+Left and Win+Right.
- Composed contrast on the four wallpapers via `Measure-Contrast.ps1`, and the
  title in a signed-in window.
- Natural desktop repaint after theme/fill changes without a forced grab.
- Whether the Windows window-open and minimize animations change.

## Publication

The toolkit hook is committed locally on `lib_ui`'s `soft-panels` branch;
the shadow fix is committed after it, and the native caption change is still
uncommitted in that checkout. Publishing needs
explicit approval under the
[toolkit rules](../toolkit-fork.md#branch-and-publication-rules). The app
changes and gitlink update remain uncommitted until that fork commit is
published and its remote reachability is verified.
