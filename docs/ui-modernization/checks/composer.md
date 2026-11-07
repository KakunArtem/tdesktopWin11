# C12 composer checks

Date: 2026-11-07. Tasks 1 to 3 are implemented. Task 4 is partial: both composers
need a signed-in session, and only the empty state has been seen in a real window.
Plan:
[composer](../plans/2026-10-29-composer.md).

## Implemented behavior

Under Soft Panels both composers become a rounded pill with an 18 px radius, inset
16 px at the sides and 14 px at the bottom, with a thin border and no drop shadow.
The message list ends above the pill. The panel, or the wallpaper if it is on, shows
in the gaps.

The reply, edit, forward and link preview strips sit inside the pill, and it grows
upward with them and with the field. Voice recording takes the pill's shape.

In the main chat view the bot keyboard sits inside the pill under the field row. The
join, unblock, bot start, mute and report buttons and the write restriction take the
pill too, which settles the plan's open question about the join bar.

Soft Panels off, the stories composer and the video comment composer use the same
styles and code paths as before.

## Changes from the plans

- The plan passes the style and shrinks the geometry in each section.
  `ComposeControls` picks the Soft Panels style itself for a regular window and
  applies the new `outerMargins` style value in `move`, `resizeToWidth`, `height` and
  `heightCurrent`. The scheduled and welcome message sections needed no change, and
  the chat section changed only for the items below.
- The quick reply editor passes its own style, so it uses
  `softPanelsRepliesComposeControls`, which keeps its 220 px emoji panel minimum.
- `VoiceRecordBar` gains a `windowMaterial` descriptor flag, because the stories
  composer also has a radius and must not paint the Composer role.
- The record bar's fill is inset by 1 px, so it does not cover the pill's border.
- The join, unblock and similar buttons have transparent fills over a pill painted
  behind them. This removes their hover tint.
- The message field's fade now uses the composer's background, because the Soft
  Panels field's own background is transparent.
- Added to the plan: `softPanelsAttachEmoji`, a transparent emoji button.
  `EmojiButton` fills its area with `historyComposeAreaBg`, which showed as a square
  over the translucent pill.
- Added to the plan: in a chat section, the bot keyboard and suggested post strip
  each paint their own pill, because `ComposeControls` does not contain them.

## Automated evidence

The composer needs a session, and this machine has no test-server account, so the
checks below do not build `HistoryWidget` or `ComposeControls`. They do not show that
either composer is laid out or painted correctly.

A disposable `-testagent` overlay runs with an empty profile, in day and night,
material on and off, at 100% and 150%. It passes 50 checks per material run and 46
per solid run, 192 in all:

- Style values: margins, radius, the transparent field and buttons, and the kept
  field heights, padding and send size. The default, replies and stories composer
  styles stay as they were.
- The rounded surface helper: the pill's fill and border, corners and margins that
  keep what is behind them, a section grab clearing the pill without a border, and an
  empty rect painting nothing.
- A real `InputField`, `EmojiButton` and `FlatButton` with the Soft Panels styles
  show what is behind them.

Regressions, run again with the emoji button fix and C17 in the build:

- The [bar test](conversation-chrome.md#automated-evidence) passes 532 checks, and
  all 32 captures are byte-identical to the run before this change.
- The [title bar matrix](title-bar.md#automated-evidence) passes 1024 checks, and all
  80 captures are byte-identical.
- `test_window_material` passes 142 checks.

## User check

On 2026-11-07 the user shared a screenshot of the Saved Messages chat, in the day
theme with material on and the material chat background. It shows the empty
`HistoryWidget` composer as an inset rounded pill with the message list ending above
it, and it showed a square behind the emoji button. After the fix, the user opened the
build again and confirmed that the emoji icon looks right.

Later that day, with C17 in the same build, the user shared screenshots without
comment. One shows a muted channel's Unmute bar as a pill with its gift button inside,
and the message list ending above it. Another shows the empty composer with no patch
behind the emoji icon.

## Not verified

Everything else in a real composer, in `HistoryWidget` and in a `ComposeControls`
section:

- Solid parity with Soft Panels off, which rests on reading the code: every margin
  is zero and every style is the old one.
- Contrast of the placeholder, field text, strips and button icons, night mode, and
  how the pill sits over a wallpaper.
- States: typing, multi-line up to the maximum height, reply, edit, forward,
  link preview, autocomplete, voice, video, locked recording, slowmode, scheduled,
  silent, send-as, write-restricted, bot keyboard, drag-and-drop, and clicking the
  bars that replace the composer.
- Popups anchoring above the pill: emoji panel, attach menu, inline results and
  autocomplete.
- Section transitions, Scale and RTL.

Known differences, not yet seen:

- The join, unblock and similar buttons have no hover tint, and their click ripple has
  square corners inside the pill.
- In a chat section, the bot keyboard and suggested post strip are separate pills that
  touch the composer's pill, so their corners leave small notches.
- The composer's fade strips use `historyComposeAreaBg`, which may differ slightly
  from the translucent pill.
- The voice message preview's show animation still fills `_st.bg`, not the Composer
  role.
- The chat search bar at the bottom and a section's bottom bar stay square and full
  width.
- Autocomplete keeps the chat's full width until
  [C14](../plans/2026-10-29-transient-surfaces.md).
- The quick reply editor already offsets its composer by the box radius, so its
  bottom gap is smaller than 14 px.
