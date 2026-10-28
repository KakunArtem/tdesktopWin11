# Soft Panels, readable (exploration)

Date: 2026-10-28. Status: proposal, not yet reviewed by the user. The
[approved reference](../reference/README.md) remains the baseline and is
unchanged.

This exploration applies the [readability check](../design-directions.md#readability-check-of-the-approved-reference)
and the [gaps against Telegram behavior](../design-directions.md#gaps-against-current-telegram-behavior)
to the approved Soft Panels layout.

## Files

- [Interactive preview](soft-panels-readable.html): standalone and offline.
  Its controls switch appearance, desktop wallpaper, interface scale, material
  (this proposal or the approved values), transparency and chat background.
- [Light screenshot](soft-panels-readable-light.png) and
  [dark screenshot](soft-panels-readable-dark.png): the approved Blue folds
  wallpaper at 125% interface scale.

## Proposed changes

- **Material:** the folder rail and chat list share one panel fill, and the
  conversation canvas is denser. Window fill is 72% in light and 74% in dark;
  the shared panel adds 20% and 30%; the canvas adds 24% and 28%. Blur is
  32 px with 1.45 saturation. These are browser reference values, like the
  approved ones.
- **Text colors:** text `#132335`, secondary `#3a4b5f` and accent text
  `#004a8f` in light; text `#f2f6fb`, secondary `#c2cfdd` and accent
  `#9ad3ff` in dark. Stronger text allows thinner material.
- **Main menu:** a menu button at the top of the rail opens a main menu panel
  with the account, Contacts, Calls, Saved Messages, Settings and Night Mode.
  The account button moves into that menu, and the rail ends with Edit.
- **Unread state:** folders show unread counts. Muted chats use grey badges
  plus a muted icon, so the state does not rely on color alone.
- **Composer:** the button records a voice message when the field is empty and
  sends when it has text.
- **Sizes:** Telegram's style values at 100% scale (13 px text, 46 px list
  avatars, 62 px rows, 19 px badges), multiplied by the interface scale. At
  125% they are close to the approved sizes.
- **Chat wallpaper:** a chat wallpaper fills the message area below the
  header. The header, pinned bar, date pills and bubbles keep their own fills.
- The "MESSAGES" list label is removed; Telegram has no equivalent.

## Contrast results

Method: headless Microsoft Edge renders the preview with `?measure=1`, which
records each text and icon element's color and position, then hides text so
the background pixels underneath can be sampled. Text needs 4.5:1 (3:1 when
large) and icons 3:1, at 125% scale. 58 text items and 28 icons were checked
per run. "Approved values" rows apply the approved material and text colors to
this exploration's layout.

| Material | Appearance | Blue folds | Warm shapes | Bright sky | Night |
| --- | --- | --- | --- | --- | --- |
| This proposal | Light | 0 fail, min 5.2 | 0 fail, min 5.2 | 0 fail, min 5.5 | 0 fail, min 4.8 |
| This proposal | Dark | 0 fail, min 5.6 | 0 fail, min 5.5 | 0 fail, min 5.4 | 0 fail, min 5.6 |
| Approved values | Light | 16 fail, min 2.7 | 16 fail, min 2.8 | 7 fail, min 4.3 | 17 fail, min 2.4 |
| Approved values | Dark | 8 fail, min 3.0 | 12 fail, min 3.1 | 20 fail, min 2.4 | 0 fail, min 4.7 |

The proposal also passed with the chat wallpaper pattern (Blue folds, Bright
sky and Night) and with transparency off. Browser blur only approximates
native Windows materials, so a native prototype must repeat this check.

## Trade-off to decide

To keep small text readable over any wallpaper, about 78% (light) and 82%
(dark) of the backdrop is covered under list text, against 64% in the approved
preview. The desktop visibly tints and shows through the panels, with less
detail than in the approved preview. The lightest test, light appearance over
the Night wallpaper, keeps 4.8:1, so further thinning needs stronger text or a
different approach. Compare both with the Material control.
