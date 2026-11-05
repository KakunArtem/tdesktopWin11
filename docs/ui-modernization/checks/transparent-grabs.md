# C5 transparent grabs evidence

Checked on native Windows on 2026-11-04 and 2026-11-05, Debug, Qt 6.11.2, 100% interface
scale / 96 DPI. See the [plan](../plans/2026-10-29-transparent-grabs.md),
[visual reference](../component-references/c5-transparent-grabs/README.md)
and [complete capture inventory](grab-audit.md).

## Open blocker before C7: static search animation

Reported by the user on 2026-11-05 after C5: switching to search shows a
static picture instead of the expected animation. Resolve this report before
starting C7. Status: reported, not yet reproduced; no fix or root cause is
confirmed. The working interpretation is the search magnifying-glass
illustration; confirm that it is this illustration rather than the panel
transition when reproducing.

Source inspection: `SearchEmpty::animate` in `dialogs/ui/chat_search_empty.cpp`
requests `anim::repeat::once`. `Suggestions::setupEmpty` starts it when the
empty-state wrapper toggles on and the chats list is loaded. The icon factory
is `Settings::CreateLottieIcon` in `settings/settings_common.cpp`. This path
does not disable animation specifically for Debug. Stopping after one visible
play is expected; never moving on entry is the reported failure to investigate.

Reproduce with the actual search owner, record motion/power-saving settings,
and distinguish first entry from reopening search. Check both material and
solid modes. Trace the empty-state playback trigger, Lottie frame progression,
and restoration of live children after the suggestions reveal cache finishes.
C5 regression is a hypothesis, not an established cause. The C5 native checks
above cover panel frames and callback cleanup, not this Lottie illustration's
playback.

Close the blocker only after a regression test reproduces the missing motion
and passes with the fix, or after reproduction establishes that the reported
behavior is the intended one-shot playback. Preserve motion preferences and
the existing one-shot behavior; do not change it into a continuous loop to
hide the problem.

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
