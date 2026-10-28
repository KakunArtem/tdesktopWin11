# Visual concept alternatives

Date: 2026-10-28. Three original Telegram window concepts were presented in the
chat for comparison. The user selected Soft Panels and requested thinner
borders, a translucent contacts area and a translucent conversation panel.

## Approved reference

The user approved the corrected preview with the desktop backdrop visibly
showing through both panels. Its source, standalone preview and light/dark
screenshots are saved in [reference/](reference/README.md). Use this snapshot
as the baseline for subsequent element refinement.

## Earlier alternatives

| Concept | Layout and controls | Material direction |
| --- | --- | --- |
| Quiet Fluent | Integrated folder rail, compact chat list, restrained row selection and a simple inset composer. | Subtle Windows material across the shell and navigation; solid conversation surface. |
| Explorer Acrylic | Wider sidebar with horizontal folder tabs, stronger selection marker and a composer docked to the bottom edge. | Stronger blue/violet illumination inspired by the supplied Explorer reference; solid conversation surface. |
| Soft Panels (selected) | Folder rail, a lightly framed conversation panel, grouped pinned-message surface and a floating rounded composer. | Shared translucent material across shell, contacts and conversation; readable messages and inputs. |

The previews use the same example conversation and support light/dark
appearances. Material intensity, bubble corners and density are exploration
controls; they are not proposed product settings.

Materials are visual approximations. The previews do not establish how native
Windows backdrops will render in Qt or select a new dependency. The rounded
grouping is an original proposal, not a claim about a specific macOS client.

For transparency reviews, the mock window sits over a separate desktop
backdrop. Its outer surface must also be translucent: transparent child panels
over an opaque window only reveal that window's fill. Changing the backdrop
must visibly change both the contacts and conversation panels. Blur and tint
are preview controls, not a validated native Windows implementation.

## Selected direction and first element pass

- Reduce both the outer frame inset and the contrast of thin panel borders.
- Reveal the common material through the contacts panel and conversation
  canvas. Keep text and icons at full opacity.
- Refine contact row selection, search, the header and pinned-message strip,
  message bubbles and the rounded composer within that shared material.
- Keep message and input fills substantially denser than their surrounding
  panels for reading and composing.

The refined preview provides separate exploration controls for the window,
contacts, conversation and composer. These controls are not product settings.
The user explicitly chose visual element refinement before Qt implementation.

Proposed next step, not yet confirmed by the user: refine the individual
element states in this order: contacts/search, header and pinned content,
composer, messages/reactions, then menus and pickers.

## Readability check of the approved reference

Measured 2026-10-28 on the saved screenshots: WCAG contrast of the
reference's text colors against background pixels sampled from the contact
list, rail, empty conversation canvas and sidebar footer. Browser blur and
opacity values produced these backgrounds, not native rendering.

| Appearance | Primary text | Secondary text | Accent text |
| --- | --- | --- | --- |
| Light | 6.5–9.8:1 | 2.7–4.1:1 | 2.7–4.1:1 |
| Dark | 6.2–11.7:1 | 3.3–6.3:1 | 3.8–7.3:1 |

Secondary and accent text covers chat previews, times, rail labels, the
search placeholder, sender names and the pinned label, mostly at 11–12 px.
WCAG 2.2 asks for 4.5:1 at those sizes. The colors suit the opaque surfaces
they were chosen for, but the reference leaves the contact list unfilled and
the conversation canvas at 10% fill. A light appearance over a dark wallpaper
therefore lands on mid-tone backgrounds, the weakest case for both colors.

Proposed requirement, not yet confirmed by the user:

- Text and icons placed directly on the material meet 4.5:1, or 3:1 for
  icons and large text, against the least favorable supported backdrop.
- Reach it with a minimum fill under list and header content, text colors
  derived from the material, or both. Retuning only the preview controls does
  not change the approved snapshot.
- Check bright, dark and busy backdrops in light, dark and solid-fallback
  appearances.

The [readable exploration](explorations/README.md) applies this requirement
and passes it on four test wallpapers.

## Gaps against current Telegram behavior

The reference is a visual baseline, not a behavior specification. Element
refinement should restore these Telegram behaviors, which it omits:

- The composer shows only a send button. Telegram switches between send and
  voice or video recording ([SendButton](../../Telegram/SourceFiles/ui/controls/send_button.h)).
- The rail has no main menu entry for settings, contacts and calls
  ([MainMenu](../../Telegram/SourceFiles/window/window_main_menu.h)).
- Folder buttons show no unread counts
  ([FiltersMenu](../../Telegram/SourceFiles/window/window_filters_menu.h)),
  and unread badges do not distinguish muted chats (`dialogsUnreadBgMuted`
  in the [lib_ui palette](../../Telegram/lib_ui/ui/colors.palette)).
- Message text is 16 px and the list 14 px, while Telegram's base font is
  13 px (`fsize` in [basic.style](../../Telegram/lib_ui/ui/basic.style)).
  Sizes must follow the interface scale and font family settings.
- Chat wallpapers and custom themes have no defined behavior on the
  translucent conversation canvas.
