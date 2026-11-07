# C18 foundations checks

Date: 2026-11-07. Tasks 1 and 2 are implemented. Task 3 is partial: the automated
checks and a code search for motion are done, but nothing has been seen in a real
window. Plan: [foundations](../plans/2026-10-31-foundations.md).

## Implemented behavior

While material is active, 17 grey icon entries are raised on their role fills: resting
icons to 3:1 and hover icons to 4.5:1. Icon shapes, sizes and positions are unchanged,
and solid mode keeps the theme's own values.

The Default font family stays Open Sans, with Soft Panels on or off. The font code has
no changes from upstream.

## Changes from the plans

- Raised values differ slightly from the plan's table, by up to five steps in a
  channel, more for some night icons. The table composited on black and white, and C3
  has since moved to the measured `AcrylicRange()`. The targets are met, so the table
  no longer applies:

| Entry | Plan, day | Built, day | Plan, night | Built, night |
| --- | --- | --- | --- | --- |
| `menuIconFg` | `#6e6e6e` | `#6c6c6c` | `#8c99a4` | `#9ca8b4` |
| `menuIconFgOver` | `#535353` | `#515151` | `#dcdcdc` | `#dcdcdc` |
| `sideBarIconFg` | `#606f7f` | `#5f6e7d` | `#8097a9` | `#92a9bc` |
| `sideBarIconFgActive` | `#0972b0` | `#0470ae` | `#5eb5f7` | `#5eb5f7` |
| `historyComposeIconFg` | `#8e8e8e` | `#919191` | unchanged | `#6c7883` |
| `historyComposeIconFgOver` | `#6f6f6f` | `#727272` | unchanged | `#dcdcdc` |
| `smallCloseIconFg` | `#8f8f8f` | `#929292` | `#66727e` | `#636f7b` |
| `smallCloseIconFgOver` | `#707070` | `#747474` | unchanged | `#a3a3a3` |

- Task 2 was built and then withdrawn. With the system font as the default, the user
  shared a screenshot and said the text was a bit hard to read. After being shown the
  options, they chose Open Sans as the default, so `DefaultFontFamily` and its callers
  were removed.
- `ReadableOver` is in `Window::Material`, not `Ui::Material`.

## Automated evidence

A disposable `-testagent` overlay runs with an empty profile, at 100% and 150%. Its
profiles have the Soft Panels option on or off, so the real startup path chooses the
font. It passes 141 checks per material run and 7 per solid run, 592 in all:

- `normalFont` resolves to Open Sans, with Soft Panels on and off.
- All 17 entries reach their targets on both ends of the measured Acrylic range, for
  each of their roles, day and night.
- Hover is at least as strong as rest for all seven pairs, day and night.
- The new entries raise no role's coverage: every fill stays at its floor.
- Solid mode keeps the sampled day colors exactly.

Regressions, all passing with the icon colors in. They ran before the font default
was removed, and their profiles have the Soft Panels option off, so they already used
Open Sans:

- [Shared controls](shared-controls.md#automated-evidence): 308 pass. Solid captures
  are byte-identical. Material captures differ only in the multi-select's search icon.
- [Composer](composer.md#automated-evidence): 192 pass.
- [Bars](conversation-chrome.md#automated-evidence): 532 pass. Solid captures are
  byte-identical. Material captures differ only in the bars' close icons.
- [Title bar](title-bar.md#automated-evidence): 1024 pass, all 80 captures
  byte-identical.
- `test_window_material`: 143 pass, 1 of them new, which passed from the start because
  `ReadableOver` already existed.

## Motion

A search of the redesign's whole diff, in the app and `lib_ui`, found no added
animation, timer, duration or `PowerSaving` use, and no changed duration in a style
file. Its one match reads an existing animation's value. This is a search of the code:
it has not been watched running, and turning off interface animations has not been
tried with the redesign on.

## Not verified

These need a real window, and most need a signed-in session:

- Task 3 Step 1: each icon group at rest and on hover, measured over the test
  wallpapers. A custom theme with an accent icon color. Turning material off.
- Task 3 Step 2: whether grayscale text on the material reads as well as solid mode.
  Text is drawn with the GDI engine on Windows, and the existing FreeType font engine
  option was raised with the user as a possible improvement. It stays an option.
- Task 3 Step 3: turning off interface animations.
- RTL.
