# C13 messages checks

Date: 2026-11-07. Task 1 is implemented. Tasks 2 and 3 are the survey of real messages,
which needs a signed-in window and has not been run. Plan:
[messages](../plans/2026-10-29-messages.md).

## Implemented behavior

Under Soft Panels the large bubble radius is 14 px instead of 16. Small and grouped
corners stay as they were, and so does the file thumbnail radius. The "Use small message
bubble radius" option still wins. Soft Panels is a restart option, and the value is read
once.

With the window material chat background on, service messages, date pills and the unread
bar use the Bar fill and readable text, as built with
[C11](conversation-chrome.md#chat-background). The history scroll bars now also leave
their wallpaper tint and use the main palette's colors. Turning the background off
restores the chat theme's colors, including the scroll bars.

Bubble fills stay opaque palette colors.

## Changes from the plan

- The plan's four setters and `applyMaterialChatColors` rework were replaced earlier by
  one `ChatStyle::applyServiceColors`. The scroll colors are copied in that function, not
  a separate `applyScrollColors`.
- C11 recorded that `applyScrollColors` was left out because the history scroll style
  reads the global palette. That was wrong: `ChatStyle::make` maps the scroll style
  through the chat palette, and the checks below show the colors reach it. The admin log
  uses the global style directly, which was never tinted.

## Automated evidence

A disposable `-testagent` overlay runs with the real startup path, so Soft Panels comes
from the saved option. It covers day and night, Soft Panels on and off, material on and
off, at 100% and 150%. It passes 32 checks per run, 256 in all:

- Premises, the large radius and corner, and that the small and thumbnail radii stay.
- `BubbleRadiusLarge()` returns the Soft Panels value when the option is on, so nothing
  reads it before the option is applied during a launch with no session.
- Real `Ui::PaintBubble` with a real `ChatStyle`: all large corners, grouped corners and
  a tail. A pixel inside a 14 px corner and outside a 16 px one is filled only under Soft
  Panels. The middle of the bubble keeps its opaque fill.
- A scroll style from `ChatStyle::value` follows a tint on the chat palette, leaves it on
  `applyServiceColors`, and is restored by `reapplyPalette`. The other service colors
  follow too, and open views are told.

Against a baseline taken before any change, with Soft Panels off all captures are
byte-identical in solid and material. With Soft Panels on all captures differ, as the
radius changes.

Regressions, on this change: [chat list](chat-list.md#automated-evidence) 480 pass,
[foundations](foundations.md#automated-evidence) 592,
[navigation rail](navigation-rail.md#automated-evidence) 332,
[shared controls](shared-controls.md#automated-evidence) 308,
[composer](composer.md#automated-evidence) 192,
[bars](conversation-chrome.md#automated-evidence) 532 and
[title bar](title-bar.md#automated-evidence) 1024. Every capture compared is
byte-identical to the run before this change. `test_window_material` passes 147 checks.

Messages, `HistoryWidget` and the other chat sections need a session, so this test does
not build them.

## Scroll bar contrast

Read on the Conversation fill, over the best and worst end of the Acrylic range. The
thumb's contrast is below 3:1 in both themes, and was not changed. In the night theme the
over color reads weaker than the resting one.

| Entry | Day | Night |
| --- | --- | --- |
| `historyScrollBarBg` | 1.94, 1.57 | 1.94, 1.40 |
| `historyScrollBarBgOver` | 2.97, 2.09 | 2.02, 1.22 |

## Not verified

There is no test-server account on this machine. All of the plan's survey needs a real
window:

- Task 1's check: Bar pills on service messages and the floating date, selecting them, the
  unread bar, the night mode switch, a chat theme with custom bubbles, and turning the
  background off.
- Bubbles and text: incoming, outgoing, grouped, reply, forward, via bot, signatures,
  edited, views, reactions, link preview, quote, code block, spoiler, custom emoji and
  selection, in light and dark, on the wallpaper and on the material background.
- Bubble-less media: sticker, animated sticker, emoji-only, round video, single photo and
  video, album, with their time, views and reaction pills.
- GPU and animated content: video, round video and GIF playing inline, Lottie stickers,
  and fast scrolling, for any black or opaque rectangle.
- Mirrored sections: a topic, a replies thread, a Saved Messages sublist, pinned,
  scheduled and the admin log.
- An album, a reply and a sticker with a caption at the new radius, and the small radius
  option.
- Contrast of message text and meta over a real backdrop, Scale beyond 150%, and RTL.

Also open:

- `st::bubbleRadiusLarge` is read directly by the similar channels card, the send files
  reply header, the font box and the editor's loading link. The scale settings preview
  hardcodes 16. They stay at 16 px, so they differ by 2 px from real bubbles.
