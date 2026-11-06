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

Go with Acrylic (`DWMSBT_TRANSIENTWINDOW`). User decision, 2026-11-06.

The automated rows support it: every DWM call succeeds through the QRhi
Direct3D path, the window is not layered, only the painted region becomes
translucent and text rendering is unchanged. The plan's go criteria also ask
for maximized and full-screen states to draw correctly. The user accepted the
decision without that manual pass, so the rows marked "Not run" or
"Inconclusive" above are waived, not passed. If one of those states
misbehaves later, this record is where to reopen the decision.

## Native measurement

Date: 2026-11-06. Raw backdrops on a topmost probe window, with focus held and
verified, over solid and rendered-wallpaper windows. Values are RGB, or one
number when the channels are equal. These numbers settled the "more muted than
the mockup" finding above.

| Backdrop | Over black | Over white | Over red | Over blue | Unfocused, any backdrop |
| --- | --- | --- | --- | --- | --- |
| Acrylic, dark | 54 | 146 | 255, 7, 7 | 40, 40, 255 | 84 |
| Acrylic, light | 135 | 227 | 255, 123, 123 | 132, 132, 255 | 211 |
| Mica, dark | 28, 32, 40 | 28, 32, 40 | 28, 32, 40 | 28, 32, 40 | 32 |
| Mica, light | 240, 243, 249 | 240, 243, 249 | 240, 243, 249 | 240, 243, 249 | 243 |

- Focused Acrylic is vivid. It blurs and strongly saturates, as the mockup
  does. The muted look came from C3, whose fills assumed raw black and white
  behind the window and so hid most of it.
- Windows drops Acrylic to a flat color while the window is unfocused. Nothing
  in the documented backdrop API changes this.
- Mica ignores windows behind it, because it samples only the wallpaper. Its
  values above show that, and they do not compare the two over a real
  wallpaper, since changing the wallpaper is a system setting. Mica cannot show
  the desktop through Telegram the way the mockup does, so it was not pursued.
- `SetWindowCompositionAttribute` accent blur and accent Acrylic rendered flat
  black in the probe. They were not pursued; that probe is not proof they can
  never work.

Acrylic over the preview wallpapers, as mean color: Blue folds 57, 96, 148
dark and 143, 177, 221 light; Warm shapes 136, 92, 87 and 214, 174, 170; Bright
sky 126, 141, 155 and 208, 222, 236; Night 54, 63, 89 and 135, 144, 170. C3 now
uses the measured range; see
[material fills](../checks/material-fills.md#native-acrylic-re-measure).
