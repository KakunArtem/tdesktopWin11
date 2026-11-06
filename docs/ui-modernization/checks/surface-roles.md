# C4 surface roles evidence

Implementation checked on native Windows on 2026-11-04, Debug, Qt 6.11.2,
100% interface scale / 96 DPI. See the [plan](../plans/2026-10-29-surface-roles.md)
and [opaque paint inventory](opaque-paint-audit.md).

## Implemented

The client body paints Window; the chats list, its surrounding chrome,
folder rail container and filter-slide canvas paint Navigation; the chat
header paints Conversation; ordinary and newer composers paint Composer.
The newer composer includes both its reply header and its actual flat or
rounded wrap background. Shared composers without a regular-window host,
including the separate stories viewer, retain their original painting. Material fills replace destination pixels with
`CompositionMode_Source`; the previous painter state is restored.

The main window, list, rail and header clear their opaque-paint promise
while active. Header animation changes update the same live binding rather
than overwriting its fallback behavior. Every converted paint owner subscribes
to fill changes. With material inactive, existing solid brushes, shapes and
draw operations are retained, including the animated list highlight brush.

The C1 prototype strip and its fill API have been removed. Wallpaper stays
opaque. Folder buttons, hover/selected rows, input fields, caption controls,
bars and separate windows retain the owners listed in the audit; this is not
the complete Soft Panels layout.

## Automated evidence

The disposable overlay used an empty isolated work directory. A synthetic
local Test-environment session supplied real widgets through a hidden,
unregistered `Window::Controller`; it was not a signed-in server fixture.
The primary/domain account was never switched and no user profile was copied.
The committed `test_scenario.cpp` was restored to its original no-op.

| Check | Result | Scope |
| --- | --- | --- |
| `test_window_material` | 63 checks, zero failures | Source replacement repeated five times, clip and painter-state restoration, path fill, both surface overloads, live opacity intent, binding reuse and destruction, existing C1–C3 policy/contrast checks |
| Client body | 28 passes, zero failures | Actual client body and window palette; active, native-frame fallback, recovery, day switch and night restoration; root opaque attribute |
| Navigation | 12 passes, zero failures | Actual `Dialogs::InnerWidget` and `Dialogs::Widget`, active/fallback/recovery; inner opaque attribute |
| Header/composers | 22 passes, zero failures | Actual header, ordinary self-history composer, flat and rounded newer composers and private reply headers; independent Stories style stays solid with original color |
| Populated scrolling row | 4 passes, zero failures | A real local list entry over three scrolling frames; RGB cache bypass preserves Navigation alpha |
| Solid navigation images | Identical PNG hashes | Entire inner/outer own-paint images before and after navigation conversion, in fallback mode |
| Opaque inventory | No missing files | Every checked-in source-file grep result has a row, including false assignments, comments, reads and test fixtures; disposable overlay excluded |

Observed dark-theme alpha values: body/header 207, Navigation 209 and
Composer 245. Day body alpha was 199. Solid samples were 255. These are
Qt widget raster measurements, not measurements of the DWM-composited screen.
The existing RGB scrolling cache is bypassed while material is active; rows
paint directly until an alpha-capable cache is adopted. Each material row
source-replaces its matte before foreground so animated pinned rows cannot
reveal previously drawn neighboring foreground.

The RED runs first reproduced the old prototype/list alpha, header opacity,
both newer-composer fills and the root opaque promise; the corresponding
GREEN runs passed.

Local logs and images are under
`%LOCALAPPDATA%\Packages\OpenAI.Codex_2p2nqsd0c76g0\LocalCache\Local\tdesktop-ui-checks\surface-roles\`, in `review-body-green`,
`review-navigation-green` and `review-chrome-green`. `navigation-red` and
`navigation-green` retain the solid image comparison. `review-cache-green` retains populated-row evidence. Build/unit logs belong
to this plan's ignored execution workspace until archived after review.

## Child list tint

Date: 2026-11-06. A review found that material dropped upstream's tint on the
chats list while a forum child list is open, because the transparent row
background skipped it. `PaintTintedSurface` now shifts the Navigation fill toward
`dialogsBgOver` by the child list's progress and keeps the fill's alpha, so the
panel stays translucent. Unit checks failed against a stub first. The disposable
overlay reads `#85ffffff` with no child list and `#85f2f2f2` with one open, in
the day theme, and grabs stay transparent. No automated check opens a real forum
child list, because this machine has no test account.

## Acceptance still requiring a visible desktop

- Full C16 backdrop checks across the rail/list/header/composer in a signed-in
  window, and whole-scene solid comparisons for reply, edit and forward states.
- Pointer hover, populated-list scrolling and actual folder-slide playback.
  The repeated-fill test verifies replacement behavior; it does not simulate
  those complete interactions. C5 still owns opaque animation snapshots.
- Actual Windows Transparency effects OFF/ON and high-contrast cycles. The
  native-frame fallback tests exercise the shared C4 consumer transitions,
  not those operating-system settings. C2's remaining system acceptance is
  still tracked in [material state](material-state.md).
- Other DPI scales, RTL and custom-theme scene matrices.

The rail container is converted, but its solid sidebar buttons remain C9.
The standard C16 backdrop comparator uses a channel-difference threshold of
30; near-opaque Composer fills can change screen pixels by less than that.
A failed default-threshold Composer comparison alone does not establish an
opaque compositor result; acceptance must inspect a suitable pixel metric.
