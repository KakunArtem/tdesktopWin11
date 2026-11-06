# Verification tooling checks

Date: 2026-11-04. Code commit: `12031d0312`. Windows: 26300.9711.
GPUs: AMD Radeon Graphics and NVIDIA GeForce RTX 5080.
Fixture monitor: 2560 x 1440, display DPI 96 (100%). Windows PowerShell 5.1.

| Row | How | Result | Evidence |
| --- | --- | --- | --- |
| Wallpapers | Export four 320 x 180 test images and default 2560 x 1440 images; inspect against reference | Pass | Distinct hashes and exact dimensions; default images in `%LOCALAPPDATA%\Packages\OpenAI.Codex_2p2nqsd0c76g0\LocalCache\Local\tdesktop-ui-checks\wallpapers\` |
| Backdrop window | Non-activating native fixture; real Telegram ten-second wallpaper worker | Pass for API behavior | Correct monitor bounds and z-order; foreground handle unchanged; worker returns PID and exits on time |
| Contrast | Native 13 px antialiased labels | Pass | `#3a4b5f` / `#e9eef5`: 7.66:1 PASS; `#999999` / white: 2.85:1 FAIL as expected |
| Contrast CLI | Native fixture captured over black and white | Pass | Both report 7.66:1; valid samples exit 0 and outside samples report INVALID and exit 1 |
| Regions | Opaque and 0.7-opacity native fixtures | Pass | Opaque has zero changes; transparent region has exact physical client bounds `12,9,80,40` |
| Popup | Two native top-level fixtures | Pass | Largest is Main, topmost other window is Popup; opaque popup has zero changes |
| Invalid regions | Rectangle crossing client edge | Pass | Rejected before screen capture |
| Native capture parity | Two unchanged 240 x 120 opaque native fixture captures | Pass | Zero differences out of 28,800 pixels; exact dimensions and opaque white sample verified |
| Comparison | Generated equal, changed, alpha-only and differently sized PNGs | Pass | Exit codes 0, 1, 1, 2 respectively; single-pixel bounds `3,4,1,1`; mismatch names 32 x 24 and 40 x 30 |
| Output location | Attempt to choose a repository output path | Pass | Rejected before writing; capture sets and reports remain external |
| Activation | Non-activating native fixture, default capture and explicit inactive capture | Pass | Default API and CLI reject it; `-AllowInactive` captures record false activation and the explicit opt-in |
| Syntax | Parse all scripts with Windows PowerShell 5.1 | Pass | Zero parser errors; C# helpers compiled during native suite |
| Telegram material | Real Acrylic off/on and context menu | Pending | Exact checkout process has no visible target window at the capture attempt; no window was restored or settings changed |
| Scaled monitor | Repeat suite on an actual 150% display | Pending | Only one 100% display is available; `Test-UiChecks.ps1 -Monitor <index>` selects fixtures on an existing scaled display |
| Actual interface scale mismatch | Telegram captures at 100% and 125% | Pending | Generated dimension mismatch is covered; no user interface-scale setting was changed |

The full native fixture run is at
`%LOCALAPPDATA%\Packages\OpenAI.Codex_2p2nqsd0c76g0\LocalCache\Local\tdesktop-ui-checks\self-tests\992b5967c90a4975b6c336129c672c58\`.
Its backdrop fixtures are at
`%LOCALAPPDATA%\Packages\OpenAI.Codex_2p2nqsd0c76g0\LocalCache\Local\tdesktop-ui-checks\backdrop\20261104-144252-621\`.
These checks verify tooling behavior, not Telegram's Acrylic rendering or a
display configuration that was not exercised. C16 Tasks 3 and 4 retain their
manual acceptance rows; the architecture tracker does not count them complete.
