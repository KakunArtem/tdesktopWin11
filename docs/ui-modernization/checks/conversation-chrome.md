# C11 conversation chrome checks

Date: 2026-11-06. Only Task 3, the "Window material" chat background, is
implemented. It was built ahead of C11's other tasks at the user's request, with
the service colors from [C13](../plans/2026-10-29-messages.md) Task 1 that keep
messages readable on it. Plan:
[conversation chrome](../plans/2026-10-29-conversation-chrome.md).

## Implemented behavior

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

## Automated evidence

`test_window_material` passes 133 checks, with 10 failures against a stub first.

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

## Not verified

There is no test-server account on this machine, so no automated check has run
these. The screenshot is a single resting state and does not establish them:

- Scrolling and repaint in a long chat.
- Service messages other than date pills, selected service messages and the
  unread bar. Each pill is a translucent fill over the translucent canvas, so
  seams or stacked alpha could show up where shapes meet.
- Night mode switching with the option on, and turning the option off live.
- Pinned, scheduled, welcome-message, admin-log and topic views.
- A per-chat theme with the option on, stickers, round videos and inline video
  over the material.

Also open:

- Scroll bars keep their wallpaper-tinted colors. The plan's `applyScrollColors`
  was left out because the history scroll style reads the global palette, not
  `ChatStyle`, so it would not have reached them.
- C13's 14 px bubble radius and its message survey have not been done.
