# C11 conversation chrome checks

Date: 2026-11-07. Tasks 1 to 3 are implemented. Task 4 is partial: the header and
most bars need a signed-in session, and only one resting state has been seen in a
real window.
The chat background was built first, at the user's request, with the service
colors from [C13](../plans/2026-10-29-messages.md) Task 1 that keep messages
readable on it. Plan:
[conversation chrome](../plans/2026-10-29-conversation-chrome.md).

## Implemented behavior

### Header and bars

Under Soft Panels the chat header is 56 px tall and its shadow is gone, so its
bottom edge is the panel's continuous fill. The chats list header aligns with it.
The call, requests, pinned, more chats, sponsored, translate and contact status
bars become rounded pills inset by 12 px, with a 4 px gap below each. Their
animations, close buttons, click targets and context menus are the same code.
Topic tabs paint the Conversation fill. Bars in the chats list keep the Navigation
fill around their pills.

Soft Panels off leaves all of this as it was, with material on or off, apart from
the topic tabs, header search bar and admin log header, which now paint the
Conversation fill while material is active, as the chat header already did.

### Chat background

Settings > Chat Settings has "Show window material behind messages", shown only
while material is capable. It applies live and defaults to off, so the wallpaper
stays until the user turns it on. That default remains an
[open decision](../architecture.md#open-decisions).

With the option on and material active, the chat history, chat section, pinned,
scheduled, welcome-message and admin-log views paint the Conversation fill. With
the option off, or when material falls back to solid, they paint the wallpaper
through the same code as before.

Service messages, date pills and the unread bar then use the Bar fill. Their
text is raised to 4.5:1 over that fill and its selected variant, across the
measured Acrylic range. Turning the option off restores Telegram's colors.

## Changes from the plans

- The plan paints inside the innermost `SectionWidget::PaintBackground`. More
  than ten message previews in boxes and settings share that overload, and it
  would have punched translucent holes in them. A separate
  `PaintChatBackground` is called only by the chat views, and the previews keep
  the wallpaper.
- The plan composites service colors on black and white. They use
  `AcrylicRange()` instead, as C3 now does.
- `ChatStyle` resets to its palette on theme changes. The overrides are reapplied
  from its `paletteChanged()` signal, which fires after that reset.
- The plan puts the bar helpers in `ui_material.cpp`, but `test_window_material`
  compiles that file without generated styles. They live in
  `ui/ui_material_bars.*`.
- The plan offsets each bar's content by the pill margins. A child widget sized to
  the pill holds the content instead, so its paint and layout code stays the same.
  Without Soft Panels that widget is the bar's own, as before.
- The plan hides header shadows wherever they are shown. They take zero height
  under Soft Panels instead, which needs one change per section rather than one
  per show call.
- Header buttons and text keep their 54 px styles and are centered in the 56 px
  header.
- Added to the plan: the header search bar, the admin log header and the back
  button paint the Conversation fill, so these headers match the chat header.

## Automated evidence

### Header and bars

A disposable `-testagent` overlay builds the real `GroupCallBar`, `RequestsBar`,
`PinnedBar`, `MoreChatsBar` and `SlidingBar` with an empty profile, in day and
night, with Soft Panels on and off, material on and off, at 100% and 150%.

- Before the change the test failed 84, 86, 39 and 41 of its 129 checks across
  those four runs. After it, each run passes all 133, which include four header
  value checks added in between.
- With Soft Panels on: each row adds the bottom margin, the gaps and corners show
  the panel, the pill uses the Bar fill and has a border, bar shadows are gone, and
  buttons stay inside the pill.
- With Soft Panels off, all eight captures are byte-identical to the unchanged
  code.
- The header height and shadow height values are checked. The header widget itself
  needs a session and was not built by this test.

The [title bar matrix](title-bar.md#automated-evidence) still passes 1024 checks,
and all 80 captures are byte-identical to the run before this change.
`test_window_material` passes 136 checks.

### Chat background

`test_window_material` passes 136 checks. The chat color checks had 10 failures
against a stub first.

The disposable `-testagent` overlay adds this stage to the
[title bar matrix](title-bar.md#automated-evidence), on a real widget and a real
`Ui::ChatStyle`, with an empty profile:

- The option off paints the wallpaper path, and the option on paints the
  translucent Conversation fill: `#85ffffff` in the day theme.
- The native frame falls back to the wallpaper path and back, firing one live
  change each way. Starting in solid mode never activates it.
- Service and unread colors apply, notify open views once, and reapplying the
  palette restores the originals exactly.

All 80 captures of the matrix are byte-identical to the run before this change.

## User check

On 2026-11-06 the user turned the option on in their signed-in window, shared a
screenshot of a chat and confirmed that it looks right. It shows the Saved
Messages history painting the material, with readable date pills, intact bubbles
including a link preview, and the composer.

That check predates the header and bars, so it does not cover them.

On 2026-11-07 the user shared a screenshot of a signed-in chat with the header and
bars built, in the day theme with material on. It shows the header blending into
the panel with no shadow, its text and buttons centered, and the chats list search
field level with it. A single pinned message sits in an inset rounded pill with its
accent line and close button inside. Message bubbles are cut with a hard edge at the
bottom of the bar's row, just below the pill, where a shadow line used to mark it.

## Not verified

There is no test-server account on this machine, so no automated check has run
these. The screenshot is a single resting state and does not establish them.

Header and bars, beyond the resting state in that screenshot:

- The header in a topic, pinned and scheduled messages, search mode and selection
  mode, and in night mode.
- The chats list bars on the Navigation fill.
- The translate and sponsored bars, topic tabs in both orientations, and contact
  status bars with their real buttons.
- Showing, switching, hiding and closing each bar, the pinned bar's message
  switching, and several bars stacked in a real chat.
- Contrast of each bar's text over a real backdrop, and RTL.

Chat background:

- Scrolling and repaint in a long chat.
- Service messages other than date pills, selected service messages and the
  unread bar. Each pill is a translucent fill over the translucent canvas, so
  seams or stacked alpha could show up where shapes meet.
- Night mode switching with the option on, and turning the option off live.
- Pinned, scheduled, welcome-message, admin-log and topic views.
- A per-chat theme with the option on, stickers, round videos and inline video
  over the material.

Also open:

- C13 later corrected this record: the history scroll style does read `ChatStyle`, and
  the scroll bars now leave their wallpaper tint. See
  [messages checks](messages.md#changes-from-the-plan).
- Contact status pills are opaque, not 90%, because their buttons keep their own
  fills.
- The sponsored bar's ripple has square corners inside the pill.
- The pinned bar's fade at the bottom of a multi-message preview uses
  `historyPinnedBg`, not the Bar fill.
- The translate bar's gaps stay clickable, because the row is the button.
- The header search field keeps its own fill, which may differ from the panel.
- During a section slide a pill is painted over the panel, so it looks slightly
  more opaque until the slide ends.
- The info pane's top bar stays 54 px until [C15](../architecture.md#components).
