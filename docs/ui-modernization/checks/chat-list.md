# C10 chat list checks

Date: 2026-11-07. Tasks 1 to 3 are implemented. Task 4 is partial: the automated checks
are done and the user approved the look in a real window, but the states below were not
individually confirmed. Plan: [chat list](../plans/2026-10-29-chat-list.md).

## Implemented behavior

Under Soft Panels a highlighted row is a rounded 10 px pill inside the unchanged row,
with margins 6, 1, 2, 1, or 4, 1, 4, 1 in the narrow list. Hover uses the soft hover tint
while material is active and `dialogsBgOver` without it. The open chat is the opaque
`SelectedFill()` pill with the row's normal text, badge and icon colors. The press ripple
is clipped to the pill. The archive row, search results, hashtag rows and peer results
use the same pill.

The search field is a 34 px, 9 px radius pill in the Bar role with `Border(Surface::Bar)`,
and a 1 px `activeLineFg` stroke while focused. It has a magnifier icon.

Row height, avatar size, badge fills and hit testing are the same as before.

## Changes from the plan

- The plan sets `softActive` at each call site in `dialogs_inner_widget.cpp`.
  `SoftActiveContext` resolves it once at the top of `RowPainter::Paint` and
  `paintPeerSearchResult` instead, so every caller gets it.
- Painting an open chat as `selected` would also have turned on the jump-to-topic area,
  which upstream hides on the open chat, and so changed what a click there does.
  `dialogs_message_view.cpp` and `dialogs_topics_view.cpp` also test `softActive`.
- The active chats filter tag uses its normal color on the light pill.
- `PaintRowBackground` and `PaintRowRipple` are exported rather than file-local, because
  peer and hashtag rows are painted in `dialogs_inner_widget.cpp`.
- `BasicRow` gains `hasRipple()`, so a row with no ripple never builds a clip path while
  scrolling.
- The search pill is painted by `_searchControls`, not `Widget::paintEvent`. It then hides
  with the controls during slide animations and folder, forum and community views.
- The search pill is centered in the 56 px top bar, so it lines up with the menu button
  and chat header. The plan's top and bottom margins, 6 and 8, are not used. The left
  margin of 10 applies when the rail is shown; without the rail the menu button stays to
  its left. The right margin of 8 always applies.
- The plan does not list the magnifier, but the reference shows it. It uses the existing
  `box_search` icon.
- Plan Task 3 Step 1, the badge cut-out, needs no change. Rows are never cached while
  material is active, and without material `dialogsBg` is the right color.
- The live badge ring in the stories strip is a real cut-out while material is active,
  through the strip's existing layer. The strip now always paints through that layer
  under material, so the ring cannot punch through the window.

These go beyond the plan, because measuring them failed:

- `dialogsTextFgOver`, `dialogsDateFgOver` and `dialogsTextFgServiceOver` measured as low
  as 1.59:1 on hover and 2.39:1 on the open chat. They are now adjusted on Navigation to
  5.4:1, which leaves 4.5:1 under the hover tint.
- The focused search placeholder, `placeholderFgActive`, measured 1.96:1. The search style
  now keeps `placeholderFg` while focused, and the shared entry is unchanged.
- The search results section bars were an opaque `searchedBarBg` strip on the material.
  They now use `ControlFill`, so under Soft Panels they are the hover tint.

## Contrast

Read in the material runs over the measured Acrylic range, with the best and worst end
of the range for each. Day, then night:

| Entry | On | Ratios |
| --- | --- | --- |
| `dialogsTextFgOver` | hover | 8.41, 4.83; 8.89, 4.57 |
| `dialogsDateFgOver` | hover | 8.41, 4.83; 8.90, 4.58 |
| `dialogsTextFgServiceOver` | hover | 8.27, 4.75; 8.93, 4.59 |
| `dialogsNameFgOver` | hover | 13.96, 8.02; 9.89, 5.08 |
| `dialogsTextFgOver` | open chat | 7.28; 7.90 |
| `dialogsDateFgOver` | open chat | 7.28; 7.91 |
| `dialogsTextFgServiceOver` | open chat | 7.16; 7.93 |
| `dialogsNameFgOver` | open chat | 12.09; 8.79 |
| `placeholderFg` | search pill | 8.06, 7.27; 9.84, 8.41 |
| `dialogsMenuIconFg` | search pill | 5.25, 4.73; 6.57, 5.61 |
| `activeLineFg` | search pill | 5.30, 4.77; 6.66, 5.69 |

Badge fills are the user's decision from 2026-10-31 and were kept. They are recorded here
and not held to a target:

| Badge shape | On | Ratios |
| --- | --- | --- |
| `dialogsUnreadBg` | rest | 2.67, 1.53; 3.59, 1.79 |
| `dialogsUnreadBgMuted` | rest | 1.92, 1.10; 1.88, 1.07 |
| `dialogsUnreadBgOver` | open chat | 2.03; 2.61 |
| `dialogsUnreadBgMutedOver` | open chat | 1.46; 1.45 |

White badge text on its own badge is 2.67 and 1.92 in the day theme, and 4.09 and 7.35 in
the night theme.

## Automated evidence

A disposable `-testagent` overlay paints with the real functions and styles and an empty
profile, in day and night, with Soft Panels on and off, material on and off, at 100% and
150%. It passes 150 checks per material run and 90 per solid run, 480 in all:

- Rest shows the list: nothing in material, `dialogsBg` in solid.
- Hover fill, its rounded corners, all four gaps and both margins.
- The open chat pill's fill, corners and opacity, and narrow margins.
- The context helper: an active row takes the normal colors, the result is final, and
  with Soft Panels off an active row stays active.
- A real ripple: painted inside the pill, clipped at the corner, gutter and top. With Soft
  Panels off it stays square and fills the row. A row with no ripple leaves no clip.
- The archive row's pill, from the real `PaintCollapsedRow`.
- The search pill's height, radius, fill through the real field, corner, edge and focus
  stroke.
- The contrast table above.

Soft Panels off, against a baseline taken before any change:

- Solid: all 6 captures per run are byte-identical.
- Material: the 2 captures of an empty field change, from the raised placeholder. The 4
  others are byte-identical.

`test_window_material` passes, with a new check that row text keeps 4.5:1 under the hover
tint.

Regressions, on this change: [foundations](foundations.md#automated-evidence) 592 pass,
[navigation rail](navigation-rail.md#automated-evidence) 332,
[shared controls](shared-controls.md#automated-evidence) 308,
[composer](composer.md#automated-evidence) 192,
[bars](conversation-chrome.md#automated-evidence) 532 and
[title bar](title-bar.md#automated-evidence) 1024. Every capture compared is
byte-identical to the run before this change, except the 4 shared controls material
captures per run. Read at 100% in two of them, the difference is a 40 by 10 px box around
one field's placeholder, from the raised `placeholderFg`.

`RowPainter`, `InnerWidget`, `Dialogs::Widget` and the stories list need a session, so
this test does not build them. It does not paint a real chat row, avatar, name or badge.

## User check

On 2026-11-07 the user ran the build and said the chat list looks right. They shared no
screenshot and did not say which states they looked at, so nothing below is marked as
passed.

## Not verified

There is no test-server account on this machine, and these were not individually
confirmed in a real window:

- Real rows: names, previews, times, sender prefixes, badges, and how the pill sits around
  them. Unread mentions and reactions, typing, drafts and pinned chats.
- Keyboard selection with the arrow keys, middle click and the context menu.
- Dragging a pinned chat: the pill moving with it and no stale pill in the empty slot.
- A touchpad swipe: the action fill under the sliding row.
- A forum row: expanded topics, the jump-to-topic area, on the open chat and on hover.
- The narrow list: pills centered on the avatars.
- The search field in place: typing, the cancel button, search from and calendar, search
  scopes, and alignment beside the menu button and stories.
- Search results: the section bars, peer and hashtag rows, and empty search.
- The stories strip: the live badge ring and its expand and collapse animation.
- Scale beyond 150%, and RTL.

Also open:

- The speaking badge ring in `Row::paintUserpic` uses `dialogsBg`, including on a pill
  and over the material. It was not changed.
- Forum rows paint the jump-to-topic area from opaque corner images. It has not been seen
  on the pills.
