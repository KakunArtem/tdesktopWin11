# Verification Tooling Implementation Plan (C16)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give every phase the same evidence tools: the four test wallpapers behind the real window, native-screenshot contrast measurement, screenshot capture and comparison for solid parity, a region-aware backdrop check, and one format for check records.

**Architecture:** PowerShell 5.1 scripts in `docs/ui-modernization/tools/`, next to Phase 1's `Test-Backdrop.ps1`. A backdrop window shows a wallpaper or a solid color behind Telegram without touching the user's desktop settings. Contrast is measured on native captures over black and white backdrops, the worst cases for any material, against sample rectangles listed per check in JSON. Captures are saved outside the repository; check records in `docs/ui-modernization/checks/` hold results and at most four representative images per plan.

**Tech Stack:** PowerShell 5.1, .NET `System.Drawing` and `System.Windows.Forms`, Win32 `user32` through `Add-Type`, Microsoft Edge headless for rendering wallpapers.

**Spec:** [architecture.md, C16 and cross-cutting rules](../architecture.md#cross-cutting-rules); the [exploration's contrast method](../explorations/README.md#contrast-results); [plans README check rows](README.md#check-rows-used-by-element-plans).

**Depends on:** Phase 1 Task 5 (`Test-Backdrop.ps1`). Tasks 1–3 are needed before C3; Task 4 before C7.

## Global Constraints

- All [ground rules](README.md#ground-rules). These are product verification tools that people run too, so their commits take no `[ai] ` prefix.
- PowerShell 5.1 compatible; no modules beyond the .NET Framework assemblies above.
- Never change the user's desktop wallpaper, theme, scale or transparency settings from a script.
- Capture directory: `$env:LOCALAPPDATA\tdesktop-ui-checks\<plan>\`, never inside the repository.
- Contrast thresholds: text 4.5:1, large text and icons 3:1 (WCAG 2.2), as in the exploration.
- Telegram process selection: the `Telegram` process whose path contains this checkout's folder name, as `Test-Backdrop.ps1` does.

## Review Focus

- Mixed-DPI monitors: captures must use physical pixels of the monitor Telegram is on. Pinned by Task 3 `-SelfTest` run on a scaled monitor (Step 4).
- Antialiased text: sample rectangles contain edge pixels between text and background; the background estimate must ignore them. Pinned by Task 2's self-test with a 13 px label.
- The backdrop window stealing focus and making Telegram inactive (changing its look): scripts must keep Telegram in front and active. Pinned by Task 1 Step 3.
- A sample rectangle outside the window after a resize: the script must report it as invalid, not as a pass. Pinned by Task 2 Step 3.
- Captures compared at different interface scales: the comparison must refuse mismatched sizes. Pinned by Task 4 Step 3.

---

### Task 1: Test wallpapers and the backdrop window

**Files:**
- Create: `docs/ui-modernization/tools/wallpapers.html`
- Create: `docs/ui-modernization/tools/Export-Wallpapers.ps1`
- Create: `docs/ui-modernization/tools/Show-Backdrop.ps1`

**Interfaces:**
- `wallpapers.html?wallpaper=<folds|warm|sky|night>` renders that wallpaper, copied from the `.stage[data-wallpaper=...]` rules of `explorations/soft-panels-readable.html`, over the full viewport with nothing else.
- `Export-Wallpapers.ps1 [-Width 2560] [-Height 1440]` writes `folds.png`, `warm.png`, `sky.png`, `night.png` to `$env:LOCALAPPDATA\tdesktop-ui-checks\wallpapers\` with `msedge --headless --screenshot=<file> --window-size=<w>,<h> <file url>`.
- `Show-Backdrop.ps1 -Wallpaper <name> | -Color <#rrggbb> [-Seconds 0]` covers the monitor Telegram is on with a borderless, non-activating window placed directly below Telegram in z-order (`SetWindowPos` with Telegram's handle as `hWndInsertAfter`, `SWP_NOACTIVATE`), then returns its process id; `-Seconds 0` keeps it until `Stop-Process`. Dot-sourcing it exposes `Show-Backdrop` and `Hide-Backdrop` functions for other scripts.

- [ ] **Step 1: Write `wallpapers.html` and `Export-Wallpapers.ps1`**

- [ ] **Step 2: Export and look**

Run: `powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tools\Export-Wallpapers.ps1`
Expected: four PNGs; each matches the exploration's wallpaper control by eye.

- [ ] **Step 3: Write `Show-Backdrop.ps1` and check focus**

Start Telegram, run `Show-Backdrop.ps1 -Wallpaper night -Seconds 10`. Expected: the wallpaper fills the monitor behind Telegram; Telegram stays in front and its title stays active.

- [ ] **Step 4: Commit**

```bash
git add docs/ui-modernization/tools/wallpapers.html docs/ui-modernization/tools/Export-Wallpapers.ps1 docs/ui-modernization/tools/Show-Backdrop.ps1
git commit -m "Added test wallpapers and backdrop window for UI checks."
```

---

### Task 2: Contrast measurement on native captures

**Files:**
- Create: `docs/ui-modernization/tools/Measure-Contrast.ps1`
- Create: `docs/ui-modernization/checks/samples/README.md`

**Interfaces:**
- Samples file (JSON array): `{ "name": "chat row preview", "rect": [x, y, w, h], "kind": "text" | "large" | "icon", "text": "#rrggbb" }` with `rect` in physical pixels relative to Telegram's client area; `text` optional (when absent, the pixel with the greatest contrast to the estimated background is used).
- `Measure-Contrast.ps1 -Samples <file> [-Out <report.md>] [-SelfTest]` captures the window over `-Color #000000` and over `-Color #ffffff` (Task 1), and for each sample and backdrop reports background, text, ratio, threshold and pass or fail; exit code 1 if any sample fails or is invalid.
- Background estimate: quantize the rect's pixels to 4 bits per channel, drop pixels within 1.5:1 contrast of the text color (text and antialiased edges), take the most frequent remaining bin, and report the mean color of the pixels in it.

- [ ] **Step 1: Write the self-test first**

`-SelfTest` opens a WinForms window with a 13 px label `#3a4b5f` on `#e9eef5` and a 13 px label `#999999` on `#ffffff`, measures both, and expects ratios within 0.1 of 7.66 and 2.85 (the values from the C3 calculation), pass for the first and fail for the second.

- [ ] **Step 2: Implement until the self-test passes**

Run: `powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tools\Measure-Contrast.ps1 -SelfTest`
Expected: `SELFTEST PASS`, exit code 0.

- [ ] **Step 3: Invalid samples**

A sample whose rect extends outside the client area is reported `INVALID` and makes the exit code 1. Add this case to the self-test.

- [ ] **Step 4: Document samples**

`checks/samples/README.md` explains how to find rects (capture with Task 3, read coordinates in an image viewer at 100 % zoom) and that each element plan commits its samples file as `checks/samples/<plan>.json`.

- [ ] **Step 5: Commit**

```bash
git add docs/ui-modernization/tools/Measure-Contrast.ps1 docs/ui-modernization/checks/samples/README.md
git commit -m "Added native contrast measurement for UI checks."
```

---

### Task 3: Region-aware backdrop check

**Files:**
- Modify: `docs/ui-modernization/tools/Test-Backdrop.ps1`

**Interfaces:**
- New parameters: `-Region x,y,w,h` (client coordinates; default the whole window), `-Window Main | Popup` (Popup selects the topmost visible top-level window of the Telegram process other than the main one).
- New output line: `Changed bounds: x,y,w,h` — the bounding box of changed pixels, so records can show the translucent area is where the plan put it.

- [ ] **Step 1: Add the parameters and the bounds line**

- [ ] **Step 2: Re-run Phase 1's two checks**

With material off: `-Expect Opaque` passes. With material on: `-Expect Transparent` passes and `Changed bounds` covers the expected area.

- [ ] **Step 3: Popup**

Open a context menu and run `-Window Popup -Expect Opaque` with no popup backdrop: passes.

- [ ] **Step 4: Scaled monitor**

Run Step 2 on a monitor at 150 %: bounds are in physical pixels and match the window.

- [ ] **Step 5: Commit**

```bash
git add docs/ui-modernization/tools/Test-Backdrop.ps1
git commit -m "Added regions and popup windows to the backdrop check."
```

---

### Task 4: Capture, compare and record format

**Files:**
- Create: `docs/ui-modernization/tools/Capture-Matrix.ps1`
- Create: `docs/ui-modernization/tools/Compare-Captures.ps1`
- Create: `docs/ui-modernization/checks/README.md`

**Interfaces:**
- `Capture-Matrix.ps1 -Plan <name> -State <name> [-Wallpaper <name>] [-Tag <text>]` saves the Telegram window to `$env:LOCALAPPDATA\tdesktop-ui-checks\<plan>\<state>[-<wallpaper>][-<tag>].png` and prints the path. Appearance, scale and material state are part of `-Tag` (for example `light-125-material`).
- `Compare-Captures.ps1 -Before <png> -After <png> [-Tolerance 8]` prints `Different pixels: <n> of <total>` and `Changed bounds: x,y,w,h`; exit code 0 when no pixel differs by more than the tolerance, 1 otherwise, 2 when sizes differ.
- Check record format (`checks/README.md`): title, date, commit, Windows build, GPU, then one table per check row with `Row | How | Result | Evidence`; evidence is a capture path, a measured value or an embedded image (at most four images committed per plan, under `checks/images/<plan>/`).

- [ ] **Step 1: Write both scripts**

- [ ] **Step 2: Parity self-check**

Capture the same Telegram state twice without changes and compare: exit code 0.

- [ ] **Step 3: Size mismatch**

Compare captures taken at 100 % and 125 % interface scale: exit code 2 with a message naming both sizes.

- [ ] **Step 4: Write the record format and commit**

```bash
git add docs/ui-modernization/tools/Capture-Matrix.ps1 docs/ui-modernization/tools/Compare-Captures.ps1 docs/ui-modernization/checks/README.md
git commit -m "Added capture comparison and check record format."
```

## Done when

- Every script's self-check passes on a 100 % and a 150 % monitor.
- `checks/README.md` defines the record format used by every other plan.
