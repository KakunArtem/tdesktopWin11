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

Next, refine the individual element states in this order: contacts/search,
header and pinned content, composer, messages/reactions, then menus and pickers.
