# Native contrast samples

Commit one JSON array per element plan as `<plan>.json` in this directory:

```json
[
  { "name": "chat row preview", "rect": [100, 140, 240, 24], "kind": "text", "text": "#3a4b5f" }
]
```

Rectangles use physical pixels from the top-left of the client area, including
Telegram's own title bar. Capture with `Capture-Matrix.ps1`, or use the client
region captures from `Test-Backdrop.ps1 -Region x,y,w,h`. Read coordinates in
an image viewer at 100% zoom. Record the window size, theme and interface scale;
recreate the samples after moving text, resizing or changing scale. A rectangle
must include text and surrounding background, with no unrelated content.

`kind` is `text` (4.5:1), `large` or `icon` (3:1). Prefer the exact foreground
color in `text`. If omitted, the tool estimates it from the pixel with greatest
contrast against the modal background. This estimate needs a clean rectangle;
an unrelated icon or border can otherwise be mistaken for the foreground.

Run in Windows PowerShell 5.1:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tools\Measure-Contrast.ps1 -Samples docs\ui-modernization\checks\samples\<plan>.json
```

The tool places temporary black and white windows behind the target, captures
both, excludes pixels within 1.5:1 of the foreground, and averages the most
frequent remaining 4-bit RGB bin. It preserves foreground focus and rejects
covered windows, moved windows and rectangles outside the physical client.
`INVALID` and contrast failures return exit code 1. Keep the target fully
exposed on one monitor and keep the same theme and state for both captures.
The target or its popup owner must be active unless `-AllowInactive` explicitly
selects an inactive-state check. The report records activation and rejects a
change between captures.
Use `-WindowHandle` to select an isolated test window explicitly.
