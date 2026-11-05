# C5 transparent grabs evidence

Checked on native Windows on 2026-11-04 and 2026-11-05, Debug, Qt 6.11.2, 100% interface
scale / 96 DPI. See the [plan](../plans/2026-10-29-transparent-grabs.md),
[visual reference](../component-references/c5-transparent-grabs/README.md)
and [complete capture inventory](grab-audit.md).

## Resolved blocker before C7: static search animation

Reported by the user and fixed on 2026-11-05. The affected element was the
large magnifying-glass illustration in the empty search panel. C7 is no
longer blocked by this report; it has not been started.

The playback trigger in `Suggestions::setupEmpty` observed only the empty
wrapper toggling on. If the main chats list was still loading, its readiness
check rejected that trigger. Finishing loading produced no new wrapper toggle,
so the icon remained on its first frame. The same Debug executable animated
correctly when loading had already finished or search was reopened.

The fix observes both wrapper toggles and main-list loading completion,
reevaluates the combined current state and starts playback on its false-to-true
transition. Duplicate ready notifications do not restart playback; archive
folder notifications do not trigger it. `SearchEmpty::animate` still requests
one play, and motion-disabled behavior is retained. No Lottie asset, duration,
loop policy, rendering engine or snapshot code changed.

The RED fixture used the actual `Dialogs::Widget` search owner in an isolated
empty profile and a synthetic local Test session, with no primary-account
switch or user-profile copy. At 350, 850 and 1400 ms after loading completed,
the illustration produced zero changing image pairs, while a direct Lottie
reference and reopened search each produced a changing pair. The ready-at-entry
baseline passed. These checks measure the icon's own paint owner after the
panel reveal, rather than interpreting a panel slide as illustration playback.

The GREEN fixture passed 115 checks across five modes: delayed and already-ready
loading in material and solid mode, plus animations disabled through power
saving. It covers first entry, reopening, completed one-shot playback and a
duplicate loading notification. The material regression suite also passed
97 checks. Changed-line Qt lint returned zero findings. The disposable overlay
was restored to its original no-op before the normal Debug rebuild.

Evidence, PNG sequences and the reproducible scenario/launcher are under
`%LOCALAPPDATA%\tdesktop-ui-checks\search-empty-animation\`, in `loading-red`,
`ready-baseline`, the five `*-green` directories and the archived execution
folder. Full signed-in visible-desktop acceptance remains separate from these
native widget checks.

## Implemented

Main-window section, chats-list, filter, forum-child, suggestion and banner
collapse snapshots capture content without material role fills. Their
animation owners paint one steady background beneath the moving foreground.
One-column back navigation uses Navigation; section cross-fades use
Conversation with a steady Composer region for inputs and reply headers.
Wallpaper outside those role regions, text, cards and input controls remain foreground.

`GrabScope` nests and restores its depth with RAII. Active captures initialize
transparent pixels even when the source widget normally promises opaque
painting. The root promise is restored using its current binding and policy,
including a policy change during capture. `PaintSurface` clears role regions
only while active and grabbing, removing wallpaper previously painted beneath
the composer too; raw `PaintFill` keeps its replacement contract.

An appearance revision guards preparation. Active-state changes and active
fill publications finish held caches. Inactive fill publications preserve
the original animation lifecycle. Dialog canvas destruction is queued if
an appearance publication occurs inside a widget grab. Suggestions retain
their completion callback when an appearance change finishes a reveal/hide.
The dialogs owner transfers hiding suggestions before invoking hide, so a
capture-time appearance change can complete and release the panel immediately.

No toolkit/submodule changes, duration/easing changes, or later-component
layout/control redesigns are included. The additional direct-render width
snapshot follows the same scope and background rules. Ambient suggestion
fades use the Navigation role while active; solid mode retains their original
gradients. Pill foreground remains solid.

## Automated evidence

The disposable overlay ran in an empty isolated work directory. A synthetic
local Test-environment session supplied actual widgets through a hidden,
unregistered `Window::Controller`. No user profile was copied and the
primary/domain account was never switched. `RunClockedFrames` proved real
animation-manager ticks at three in-progress times per sampled transition;
each synchronous raster capture advanced no animation ticks itself.

| Check | Result / scope |
| --- | --- |
| `test_window_material` | 97 checks, zero failures: scope nesting/lifetime, rectangle/path clearing over prior pixels, opaque and nonopaque sources, crop and foreground pixels, live fallback during capture, exact upstream inactive pixmap parity, five cross-fade density samples |
| Section and narrow-window fixture | 20 passes, zero failures: real base-section grabs, three cross-fade frames, live fallback, fill publication, state/fill changes during preparation, inactive palette parity, three narrow MainWidget back-navigation frames |
| Chats-list fixture | 32 passes, zero failures: real dialog slides and fallback, nested collapse snapshots and fallback, manual width captures and fallback, suggestion frames and callback lifecycle, forum child opening/closing frames |
| Review regression fixture | 19 passes, zero failures: actual scheduled and classic Composer density at three ticks each, classic header capture clearing, Widget-owned focus-loss hide with a capture-time appearance change, panel destruction and restored chats scroll |
| Inventory | All 59 baseline source files have an audit row; converted calls and new adapter/unit-reference calls are also accounted for |

The initial RED runs observed section backgrounds baked at alpha 207/209,
cross-fade density changing through 52/102/156 instead of staying 207,
uncancelled transparent snapshots after fallback and missing narrow-list
backgrounds. The chats-list RED run reproduced a retained alpha-209 snapshot
after fallback and a collapse cache containing its background. Corresponding
GREEN runs passed. An added inactive-palette check first caught two lifecycle
regressions; active-only fill invalidation corrected them.

Dark-theme steady role alpha was 207 for Conversation and 209 for Navigation;
transparent cache samples were zero and solid fallback samples were 255.
These are Qt raster measurements, not DWM-composited screen measurements.

Local logs and PNGs are under
`%LOCALAPPDATA%\tdesktop-ui-checks\transparent-grabs\`, including
`section-red`, `solid-parity-red`, `section-final`, `dialogs-red` and
`dialogs-green`, `dialogs-fix-final`, `sections-fix-final` and `review-fix-final`. Build logs and the disposable overlay are retained in the
archived execution evidence after final review.

## Review outcome

The original whole-branch review found two Important issues: skipping fills
retained wallpaper beneath composer/header regions and lost Composer density;
synchronous suggestion cancellation could complete before ownership transfer.
Both were reproduced in the actual widgets and fixed in one pass. The unit
suite first failed three transparent-clearing checks, then passed all 97.
The final native suite passed all 71 checks. Composer padding stayed at alpha
245 across the sampled frames; intentional moving controls and slide shadows
retain their own foreground opacity. Saved frames distinguish those from
the steady matte. Qt lint and inline ownership/API checks found no new issues.

Deferred minor: move the local `CaptureWidget` method definitions outside its
class body to match the review style guide. This is test formatting only.

## Visible-desktop acceptance still pending

Full signed-in playback with populated folders/topics, pointer interactions,
multiple scale factors and actual DWM composition remains a visual acceptance
step. Actual Windows Transparency effects and contrast-theme cycles were not
controlled during C5. The native-frame fallback tests drive the same material
consumer transition, but do not replace those OS checks. Separate popup/media
and info/settings captures retain the owners recorded in the audit.
