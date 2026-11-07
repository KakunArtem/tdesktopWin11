# C9 navigation rail checks

Date: 2026-11-07. Tasks 1 to 3 are implemented. Task 4 is partial: the automated
checks are done and the user checked the open drawer, but the rail has not been seen in
a real window. Plan: [navigation rail](../plans/2026-10-29-navigation-rail.md).

## Implemented behavior

Under Soft Panels the rail is 64 px wide and part of the navigation panel. Its labels
use `windowSubTextFg`, its icons `menuIconFg`, and the selected label and icon
`windowActiveTextFg`. The selected folder is a rounded 10 px pill in `SelectedFill()`
with a 3 by 20 px accent marker at the rail's edge. Hover is a rounded pill in the soft
hover tint, and the ripple keeps the pill's shape. Without Soft Panels the rail keeps
its dark upstream look, and `sideBarTextFg` and `sideBarTextFgActive` are raised to
4.5:1 while material is active.

The main menu is a Popup role surface with 14 px right corners. Its rows hover as
rounded 8 px pills, and the layer dim shows in the corners outside the drawer.

Folder clicks, reordering, context menus, unread counters, muted badges, the Edit entry
and the premium lock use the same code as before.

## Changes from the plans

- `SideBarButton` always sets `WA_OpaquePaintEvent`, so the plan's transparent styles
  alone would have left stale pixels, and it has no rounded ripple. The fork gains
  `SideBarButton::setBackgroundPainter`, `setIconColors` and the style's `pillMargins`
  and `pillRadius`. The default is upstream's behavior.
- The plan paints pills on the container, which would need repaint wiring for hover,
  selection and drags. The button paints its own pill, so all of those follow it.
- A locked folder's filler characters were hidden by the opaque fill. Under Soft
  Panels they are clipped out instead.
- `SettingsButton` gains `pillMargins` and `pillRadius` style values, in place of the
  plan's rounded hover painted by the drawer's container.
- The fork gains `SetMainMenuCornerRadius`, which extends the layer dim under the
  drawer's corners. The dim stopped at the drawer's square edge.
- Filter icons are recolored at paint time by `setIconColors`, not through parallel
  icons, so `Ui::LookupFilterIcon` stays the same. The main menu icon has parallel
  styles, because its unread dot must keep its own color.
- The plan rebuilds buttons when `SoftPanelsValue()` changes. Soft Panels is a restart
  option, so this was left out.
- `FolderFavoriteButton` gets a transparent style and is not opaque, but it has no pill
  or hover. It sits behind the experimental favorite link option.
- `sideBarTextFg` and `sideBarTextFgActive` measure `#455360` and `#005586` in the day
  theme, not the plan's `#485563` and `#005788`, because C3 now uses the measured
  Acrylic range.

## Automated evidence

A disposable `-testagent` overlay builds real `SideBarButton`s and a real
`SettingsButton` with an empty profile, in day and night, with Soft Panels on and off,
material on and off, at 100% and 150%. It passes 87 checks per material run and 79 per
solid run, 332 in all:

- Rail width, and that buttons are no longer opaque under Soft Panels.
- Rest shows the rail. The selected pill, in all three rail modes, its rounded corners,
  the gutter, the marker's position and extent, and the hover pill.
- The main menu row hover, its rounded corners and gutter, and the drawer's shape: right
  corners rounded, left edge square, and the Popup fill.
- `sideBarTextFg` and `sideBarTextFgActive` reach 4.5:1 across the Acrylic range.
- Soft Panels off, in solid mode: all captures are byte-identical to a baseline taken
  before any change.
- Soft Panels off, in material mode: captures differ only in rows that line up with
  the labels, from the raised text colors. This was read at 100% in the day theme.

`FiltersMenu`, `MainMenu` and the layer dim need a session, so they were not built by
this test.

Regressions, on this change: [foundations](foundations.md#automated-evidence) 592 pass,
[shared controls](shared-controls.md#automated-evidence) 308,
[composer](composer.md#automated-evidence) 192,
[bars](conversation-chrome.md#automated-evidence) 532 and
[title bar](title-bar.md#automated-evidence) 1024. Every capture compared is
byte-identical to the run before this change. `test_window_material` passes 143 checks.

## User check

On 2026-11-07 the user shared two signed-in screenshots in the night theme with
material on and said they look good. One shows the open main menu with its rows and the
dim behind it, and the other the Settings box. Neither shows the rail, so it has still
not been seen in a real window.

## Not verified

There is no test-server account on this machine, and all of this needs a real window:

- The rail in place at 64 px, and the chats list and layout around it.
- Dragging folders in Edit mode, long folder names, context menus.
- Keyboard focus. A 2 px ring is implemented, but folders only take focus in screen
  reader mode, and it has not been seen.
- Real badges, a muted-only folder, and a premium-locked folder.
- The main menu: row hover, account switching, using the night mode toggle, the show
  and hide animation, the snow cover, Escape and outside clicks.
- Contrast over a real backdrop, Scale beyond 150%, and RTL.

Also open:

- Account rows and the add account row in the drawer keep the square soft hover.
- A ripple on the favorite link button is square.
