# UI implementation checks

Keep one record per component plan here. Use the same state, theme, interface
scale, window size and samples for comparisons. Run the tools in Windows
PowerShell 5.1. They never change desktop wallpaper, theme, scale or material
settings. Bring the target window fully into view on one monitor first;
covered, minimized or unavailable windows are errors rather than evidence.
Capture tools require the target or its popup owner to be active. Launch the
check in the background with the target active. For an intentional inactive
state, pass `-AllowInactive`; reports and capture metadata record that choice
and the observed activation state. A state change during or between paired
captures is rejected. No tool activates user windows.

## Record format

Each record starts with its title, date, source commit, Windows build, GPU,
interface scale and display DPI. Copy capture metadata from the adjacent JSON
file when available. Distinguish automated fixtures from real Telegram checks.

```markdown
# Component name verification

Date: YYYY-MM-DD. Commit: <sha>. Windows: <build>. GPU: <name>.
Theme: <light/dark>. Interface scale: <percent>. Display DPI: <value>.

| Row | How | Result | Evidence |
| --- | --- | --- | --- |
| Solid parity | Same state with both switches off | Pass/Fail/Pending | Capture paths and diff count |
| Material | Red/blue backdrop in the component's regions | Pass/Fail/Pending | Changed fraction and bounds |
| Contrast | Black/white backdrop and committed samples | Pass/Fail/Pending | Report path and lowest ratio |
| States | Listed mouse and keyboard states | Pass/Fail/Pending | Capture paths |
| Scale | 100%, 125%, 150%, 200% | Pass/Fail/Pending | Paths and observed clipping/seams |
| RTL | Same screens in an RTL language | Pass/Fail/Pending | Capture paths |
```

Evidence can be an external capture path, a measured value or an embedded
image. Commit at most four representative images per plan, under
`images/<plan>/`. Keep full capture sets outside the repository at
`$env:LOCALAPPDATA\tdesktop-ui-checks\<plan>\`. Do not commit private chats.
Runs made from a Codex sandbox land in its package cache instead, and the
records written from there name that path.
Pending or invalid captures do not count as passes.

## Tools

Run these from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tools\Export-Wallpapers.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tools\Show-Backdrop.ps1 -Wallpaper night -Seconds 10
powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tools\Capture-Matrix.ps1 -Plan shell-layout -State idle -Wallpaper folds -Tag light-125-material
powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tools\Compare-Captures.ps1 -Before before.png -After after.png
powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tools\Test-Backdrop.ps1 -Region 0,20,400,60 -Expect Transparent
powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tools\Measure-Contrast.ps1 -Samples docs\ui-modernization\checks\samples\shell-layout.json
```

The default target is this checkout's exact `out\Debug\Telegram.exe`.
`-WindowHandle` selects an isolated native test window. `Test-Backdrop -Window
Popup` selects the topmost visible top-level window other than the largest
window in that process; close unrelated separate windows before checking a
menu. Regions and changed bounds use physical client coordinates, including
Telegram's own title bar. Without a region, the backdrop check keeps the
original visible-frame capture with a two-pixel border inset.

`Show-Backdrop` returns its worker PID and closes after `-Seconds`; zero keeps
it until that exact worker is stopped. Dot-sourcing exposes `Show-Backdrop`
and `Hide-Backdrop`. Other scripts remove their own temporary backdrops in
`finally`. They preserve focus and do not activate or restore Telegram.

Capture names use letters, digits, dots, underscores and hyphens. Include theme,
scale and material state in `-Tag`. The PNG is the physical client area; JSON
beside it records dimensions, DPI, source commit, Windows and GPU. Reusing the
same name replaces that capture, so give distinct states or tags to baselines.

Comparison returns 0 for no RGBA channel difference above `-Tolerance` (8 by
default), 1 for differences, and 2 for mismatched dimensions. A size difference
cannot establish visual parity. Freeze animated content before capturing.
Backdrop detection returns 0 for the expected result, 1 for a mismatch. Its
sampled RGB sum threshold is 30 and changed fraction threshold is 0.2%; bounds
include all changed pixels, not only the sampled grid. Capture or input errors
also return nonzero. Contrast thresholds and samples are in
[samples/README.md](samples/README.md).

## Tool self-checks

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tests\Test-UiChecks.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tests\Test-UiChecks.ps1 -Monitor 1
```

The suite uses generated images and temporary non-activating native windows.
It checks exact dimensions, focus and z-order, native antialiased labels,
invalid regions, opaque and transparent fixtures, popup selection, capture
parity, changed bounds and size rejection. Fixture checks do not certify
Telegram's Acrylic or a monitor configuration that was not exercised.
`-Monitor` selects the zero-based display for fixtures, allowing a second run
on a monitor already set to 150% without changing display settings.
