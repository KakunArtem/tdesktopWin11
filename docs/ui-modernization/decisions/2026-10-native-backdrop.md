# Native backdrop prototype results

Date: 2026-11-04. Build: Debug, commit `52f8b0c1b6` (Tasks 1–5 of the
[prototype plan](../plans/2026-10-28-native-backdrop-prototype.md)).
Windows: `Microsoft Windows [Version 10.0.26300.9711]`, one 2560×1440
display. GPUs: AMD Radeon(TM) Graphics; NVIDIA GeForce RTX 5080 (the `QRhi`
log line names the API, not the adapter).

Automated rows ran from scripts that start `out\Debug\Telegram.exe`, set the
option through `tdata\experimental_options.json`, place solid or wallpaper
windows behind it and capture the screen. Captures show a real account and are
kept out of the repository.

## Log

Option off:

```text
Material: build 26300, transparency [TRUE], high contrast [FALSE], native frame [FALSE], refusal: not requested
```

Option on:

```text
Material: build 26300, transparency [TRUE], high contrast [FALSE], native frame [FALSE], refusal: none
Material: extend frame 0x0, dark mode 0x0, backdrop 0x0
QRhi-Check [material]: surface=Direct3D, usesRhi=YES.
```

The window is not layered (`WS_EX_LAYERED` clear) in either state, so the
roadmap's main risk, DWM ignoring the backdrop on a layered window, did not
occur: the QRhi Direct3D backing store composites with the backdrop.

## Matrix

| Check | How | Result |
| --- | --- | --- |
| Option off is unchanged | `Test-Backdrop.ps1 -Expect Opaque` | Pass: 9 of 55 328 samples changed (0.02%), all in one animated emoji of a chat preview. |
| Backdrop visible | `Test-Backdrop.ps1 -Expect Transparent` | Pass: 1.26% changed, confined to the strip above the chats list (x 0–284, y 0–80 of an 798×622 capture) plus the same animated emoji. |
| Matches the mockup | Rendered `tools/wallpapers.html` wallpapers behind the window; compared the strip with the C1 reference and the mockup | Partial: the strip takes each wallpaper's tint with no sharp edges (blue folds, warm, sky, night), but more muted than the mockup. Over solid red and blue the strip moves about 10 levels per channel (174,163,163 against 164,164,174): Acrylic's own tint dominates the 60% prototype fill. Mica comparison not run. |
| Light and dark | Toggle night mode; backdrop tint follows | Not run: needs the night mode switch in the UI. |
| Maximized | Maximize, rerun the transparent check | Inconclusive: a full-screen game on the desktop came to the front during several automated captures. In one controlled run the maximized window drew normally and was not cloaked (`DWMWA_CLOAKED` 0); translucency of the strip while maximized is not confirmed. |
| Snapped | Snap left with Win+Left, look at the strip | Not run. |
| Full screen | Enter and leave full screen; no flicker or black area | Not run. |
| Inactive window | Click another app; note how the strip looks | Not run reliably: the same game covered the window when another window was activated. |
| DPI change | Move between 100% and 150% monitors, or change scale | Not run: one display. |
| Text quality | Zoom a screenshot of the search placeholder; ClearType or grayscale | Pass for opaque areas: the share of colored-edge text pixels in the chats list is 49.3% with the option off and on, so subpixel rendering is unchanged. No text sits on the translucent strip in the prototype; text over material is checked in C4. |
| Holes | Any area other than the strip that became see-through or black | Pass in a normal window: no change outside the strip except the animated emoji; no black areas in the captures. |
| Scrolling cost | Scroll the chats list fast; compare smoothness with the option off | Not run. |
| Video | Play a video message in a chat; check it draws correctly | Not run. |
| Calls panel | Open a call panel or group call window if available | Not run. |
| Transparency effects off | Windows Settings > Personalization > Colors; restart Telegram; log shows `transparency effects off` | Not run: changes a Windows setting; left to the user. The refusal itself is covered by the policy unit test. |

## Decision

Pending. The automated rows support going ahead with Acrylic: every DWM call
succeeds through the QRhi Direct3D path, the window is not layered, only the
painted region becomes translucent and text rendering is unchanged. The plan's
go criteria also require maximized and full-screen states to draw correctly,
and those, with the other "Not run" rows, need a manual pass with no
full-screen application on the desktop.

Already visible for later plans: the native strip is more muted than the
mockup, so C3's floors should be re-measured on native screenshots, and the
Mica comparison from Task 6 Step 4 is worth running if the manual pass also
finds Acrylic too muted.
