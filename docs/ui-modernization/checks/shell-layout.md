# C7 shell layout checks

Date: 2026-11-05. Implementation and isolated native checks are complete;
visible-desktop acceptance remains pending. Plan:
[shell layout](../plans/2026-10-29-shell-layout.md). Contract:
[shell geometry](../architecture.md#shell-geometry-c7).

## Activation and scope

Enable **Soft Panels layout** in Experimental settings and restart the Debug
app. The Windows-only option is off by default and works independently of
Window transparency. With both options off, the original layout remains.

The navigation panel stays flush left, with right corners rounded to 12;
conversation and third-column panels use radius 14. Gaps are 4 and the outer
right/bottom inset is 6 at 100% interface scale. Style tokens scale these
values. Existing column minimums and ratios remain in use, with space reserved
for the frame and visible gaps. Resize receivers sit inside the gaps.

Call, export and player bars retain their order at the conversation top.
Settings, boxes, media preview, introduction and locks retain full-body
geometry. Frames pass mouse events and do not enter keyboard navigation.

## Automated evidence

The standalone `test_window_material` suite passes 117 checks with zero
failures. Its C7 cases cover one/two/three-column rectangles, a separate
window without navigation, translated origins, right-only rounded corners,
small bounds and an outline without internal seams. Existing C1-C5 checks
remain in the suite.

A disposable `-testagent` overlay creates a synthetic local Test session and
a hidden, unregistered controller in a separate work directory. No user
profile is copied, and no primary or active account is switched. It invokes
real shell widgets and resize receivers. Captures are Qt raster output,
not DWM-composited screenshots.

| Theme | Material premise | Interface scale | Native passes | Failures | PNGs |
| --- | --- | --- | --- | --- | --- |
| Light | Active | 100% | 89 | 0 | 9 |
| Light | Solid | 100% | 88 | 0 | 9 |
| Dark | Active | 100% | 89 | 0 | 9 |
| Dark | Solid | 100% | 88 | 0 | 9 |
| Light | Active | 150% | 89 | 0 | 9 |
| Light | Solid | 150% | 88 | 0 | 9 |
| Dark | Active | 150% | 89 | 0 | 9 |
| Dark | Solid | 150% | 88 | 0 | 9 |

All 708 native checks pass. Each leg saves one/two/three-column captures
with the switch on and one/two-column captures with it off, plus three moving
and one settled whole-show frame. Logs verify the
actual palette, material state and startup scale. Checks cover:

- Narrowing across the one/two-column threshold and widening again with
  Saved Messages open, without overlap or a retained gap.
- Exact outer inset and panel budgets, including two gaps in three columns.
- Actual chats-list and third-column drags, matching frame and resize-area
  geometry, and preserving chats width when enough conversation space exists.
- Opening a third column at minimum width and restoring the exact original
  width when it closes.
- Window-role density in the gaps in material and solid modes.
- Mouse-transparent, nonfocusable frames only on the active panels.
- Conversation snapshots cropped to their viewport, excluding the gaps and
  stationary frame strokes.
- A real box layer covering the full body, including the outer inset.
- Three manager-clocked whole-show frames and natural completion, retaining
  the Window role in gaps and outside rounded corners in both material and
  solid modes.

Evidence is under `%LOCALAPPDATA%\Packages\OpenAI.Codex_2p2nqsd0c76g0\LocalCache\Local\tdesktop-ui-checks\shell-layout\`.
Final matrix folders are `review-<material|solid>-<day|night>-<100|150>-final`, each
containing `test_log.txt` and `columns-<1|2|3>-switch-<0|1>.png`. There are
72 final matrix PNGs, including 24 steady-state captures with the switch on
and 32 animation/completion captures. The initial 40-PNG matrix is retained
in folders without the `review-` prefix. Build logs, the disposable
overlay and the execution ledger are retained with the execution evidence.

The initial native RED run found missing frames, gap 0 instead of 4 and the
old 6-pixel resize receiver. Further checks caught an internal navigation
stroke, a third-column extension four pixels short, a snapshot four pixels
too wide and a retained stationary border at alpha 26. The corresponding
GREEN runs pass, including a border-free material snapshot at alpha 0.

Final review found whole-show frames painting panel density in the gaps and
corners. Three real animation ticks reproduced alpha 201/207 instead of
Window alpha 199. Reapplying the Window matte outside the current rounded
panels after compositing fixes all sampled moving and settled frames.
Whole-workspace frame strokes move with the captured workspace; per-section
frames remain stationary and are excluded from section snapshots.

Deferred minor: forcing the toolkit RTL flag can mirror physical bar/resize
coordinates again. The flag defaults to false and has no production setter
caller in this checkout; selecting an RTL language is not a proven trigger.
Required new-code formatting was corrected after review.

## Visible-desktop acceptance still pending

The native matrix proves widget geometry and raster painting. It does not
replace these desktop checks from the plan:

- `Capture-Matrix.ps1` and `Test-Backdrop.ps1 -Expect Transparent` against
  actual composed gaps/panels on the reference wallpapers.
- Maximized, snapped, full-screen and inactive desktop windows.
- Populated folders/topics, live call/export/player bars, settings and media
  preview, and signed-in pointer interaction beyond the isolated fixture.
- A visual scrolling comparison on a long chat and populated chats list.
  Replace the overlay with corner/edge pieces only if that comparison shows
  a frame-rate regression.
- Exact pre-plan pixel parity with both switches off, RTL and additional
  scale factors. Automated switch-off checks cover the original geometry.

No OS settings or security controls were changed for these checks.
