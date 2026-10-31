# Shared controls, foundations and separate windows

Date: 2026-10-30. Status: grouping and proposed design direction. C17 has a
[plan](plans/2026-10-30-shared-controls.md) and a
[visual reference](component-references/c17-shared-controls/README.md), and
C18 a [plan](plans/2026-10-31-foundations.md) and a
[visual reference](component-references/c18-foundations/README.md), and C19 a
[plan](plans/2026-10-31-separate-windows.md) and a
[visual reference](component-references/c19-separate-windows/README.md). This
document covers the elements that the
[component architecture](architecture.md#coverage-of-the-element-map) left
without an owner, groups them into three proposed components, C17 to C19, and
records what each one paints today.

Scope follows the user's decision of 2026-10-30: Windows first. Everything
below is judged on Windows; macOS and Linux keep the upstream look.

Sources: the tdesktop checkout, and `lib_ui` at the pinned commit `79c846fd`,
read from `github.com/desktop-app/lib_ui` because the submodule is not checked
out here. Palette values are that commit's day palette (`ui/colors.palette`).
Contrast is the WCAG formula `Ui::CountContrast` uses, against `windowBg`
(`#ffffff`), so it describes solid mode; on the material it can only get worse.

## Groups

| ID | Group | Elements | Owner today |
| --- | --- | --- | --- |
| C17 | Shared controls | Buttons, selection controls, text fields, tabs and sliders, scroll bars, lists, indicators | None; C10, C12, C14 and C15 restyle single instances |
| C18 | Foundations | Typography, icons, motion | None; Telegram's values are kept |
| C19 | Separate windows | Media viewer, calls, Mini Apps and other separate panels, Instant View, notifications, restore shell | None; C4 and C15 keep them opaque |

## C17 Shared controls

Controls are `lib_ui` widgets configured by style structs. App `.style` files
declare 255 `IconButton`, 127 `RoundButton`, 90 `InputField`, 23
`ScrollArea`, 17 `Checkbox`, 11 `MediaSlider`, 10 `Toggle`, 9
`SettingsSlider`, 8 `Check` and 3 `Radio` styles. A control is a problem on
the material only when it paints an opaque resting background, or when its
lines and marks miss 3:1.

| Subgroup | Elements | What paints today | Proposed direction |
| --- | --- | --- | --- |
| Buttons | `RoundButton` (active, light, attention), `FlatButton`, `IconButton`, `IconButtonWithText`, `OutlineButton`, `LinkButton`, `CrossButton` | `RoundButton` fills `textBg` as a rect; the light style uses `lightButtonBg: windowBg`. `FlatButton` fills `bgColor`. `IconButton` has no background, only its ripple. | Under Soft Panels, a fill in the surface color is skipped, so light and flat buttons rest on the role behind them; a `windowBgOver` hover becomes the chats list's soft hover. Filled active buttons, shapes and ripples stay. |
| Selection controls | `Checkbox` with `Check` or `Radio`, `Toggle`, `RoundCheckbox`, `RoundImageCheckbox` | `Check` and `Radio` have `bg: transparent`. `Toggle` fills its knob with `windowBg` in both states. `RoundCheckbox` draws a `windowBg` ring around a selected avatar. | Keep the shapes. The knob stays opaque, as the mockup's switch does. The avatar ring stays: it sits on the avatar, and its boxes have the 0.97 Popup fill. Unchecked marks (`checkboxFg` `#b3b3b3`) are 2.10:1 on white, below 3:1; C17 raises them to 3:1 on the material. |
| Text fields | `InputField` and its masked, password, number and time variants, `MultiSelect` | `defaultInputField` fills `textBg: windowBg` and draws a bottom line in `inputBorderFg` (`#e0e0e0`, 1.32:1). `MultiSelect` fills `boxSearchBg`. | Fields on a role fill take `textBg: transparent`, like the composer field (C12). The search field already becomes a pill (C10); whether other fields keep the bottom line or become pills is a decision. The focus line `activeLineFg` stays. |
| Tabs and sliders | `SettingsSlider` and tabs slider (`discrete_sliders`), `MediaSlider` and `FilledSlider` (`continuous_sliders`), `PillTabs`, `SlidingTabs`, the folder tabs strip (`chat_filters_tabs_strip`) | The sliders paint bars only. `sliderBgInactive` (`#e1eaef`) is 1.22:1. Tabs paint ripples, and `PillTabs` an active pill (`windowBgOver`). | Keep the shapes, and check bar contrast on the material. The folder tabs strip appears above the chats list when folders are shown as tabs instead of the rail, which C9 and C10 do not cover. It belongs with C10's navigation panel styling. |
| Scroll bars | `ScrollArea` (all scrolling content), elastic scroll | Bars are translucent black (`scrollBarBg` `#00000053` on `scrollBg` `#0000001a`), round 2 px. Top and bottom shadows use `shadowFg`. | Translucent already, so they work on the material. The mockup's thin bars are a style value. C17 checks the scroll shadow lines inside rounded panels, where they could cut across the corners. |
| Lists | `PeerList` rows (members, contacts, boxes), `SettingsButton` rows (C15), `DividerBar` | Each `PeerList` row button fills `defaultPeerListButton.textBg: windowBg`. Dividers fill `boxDividerBg` (`windowBgOver`). | The same mapping as buttons: no resting fill, the soft hover, and divider bands in the hover tint. It covers settings rows (C15 consumes it) and the info pane's members list. |
| Indicators | `InfiniteRadialAnimation`, progress and level meters, the passcode strength meter, `Shadow` between areas | Strokes and fills on top of the background. `Shadow` paints `shadowFg` lines. | Keep them. Plain shadows between panels are replaced by the gaps of C7; the ones left inside a panel stay. |

Badges belong to C10, and tooltips and toasts to C14.

## C18 Foundations

### Typography

- Telegram's base size is `fsize` 13 px, with `normalFont`, `semiboldFont`
  and `defaultTextStyle` in `ui/basic.style`. The readable mockup uses the
  same 13 px base and 600 weight, so sizes do not change.
- The mockup's family is "Segoe UI Variable Text". Settings > Chat settings >
  Font family already offers Default (bundled Open Sans), System font and any
  installed family (`Core::Settings::customFontFamily`,
  `style::SystemFontTag()`). On Windows, System is the Segoe UI family, sized
  to Open Sans's x-height. Choosing it is a user setting today. Making it the
  Soft Panels default is a decision; if the user makes it, C18 adds one
  function so that Default means the system font under Soft Panels.
- Text drawn on a translucent surface loses ClearType subpixel rendering, as
  the roadmap's risks note. Bubbles stay opaque (C13), so message text keeps
  it; list and header text is the case to compare in the Phase 1 record.

### Icons

- Icons are monochrome masks recolored by palette entries
  (`icon{{ "path", color }}`): 2,442 PNG files at 1x, 2x and 3x and 321 SVG
  files in `Telegram/Resources/icons`, plus `lib_ui`'s own. The mockup draws
  thin line icons (1.4 to 1.7 px strokes, Lucide in the approved reference).
  Redrawing Telegram's icon set is out of scope; the shapes stay.
- Icon colors are a readability issue. `menuIconFg` (`#999999`) is 2.85:1 on
  white, below the 3:1 icon target even in solid mode. `dialogsMenuIconFg` and
  `historyComposeIconFg` are defined as `menuIconFg` in the palette. C3's
  adjusted list had no icon entries, and C8 adds only the caption buttons.
  The C18 plan adds the grey icon entries at 3:1 and their hover entries at
  4.5:1, so hover stays stronger than rest. Each derived entry is adjusted
  itself, because a theme stores its own value for it. Most app icons use
  `menuIconColor` (`windowBoldFg`) and already pass.

### Motion

- Durations are `lib_ui` style values: `universalDuration` 120 ms,
  `slideDuration` 240 ms, `fadeWrapDuration` 200 ms, ripples and per-widget
  `duration` fields. The mockup's transitions are 120 ms, which matches.
- Power saving (`ui/power_saving.h`, flags such as `kAnimations` and
  `kChatBackground`) already turns animations off. New Soft Panels effects,
  such as the pill hover, follow the same flags and the existing durations.
  No new motion is proposed; grabs during animations are C5.

## C19 Separate windows

These are top-level windows of their own, not regions of the main window.
Separate chat windows ("Open in new window") are main windows and are covered
by C2 and C7.

| Window | Source | Today | Proposed direction |
| --- | --- | --- | --- |
| Media viewer, stories and picture-in-picture | `media/view/media_view_overlay_widget.*`, `media_view_pip.*`, `platform/win/overlay_widget_win.h` | Dark full-window overlay (`mediaviewBg` `#222222eb`) with video and GPU content | Stays as it is. It is content-first and dark, and its GPU surfaces need opaque fills. |
| Calls and group calls, including the screen-share source chooser | `calls/calls_window.*`, `calls/calls_panel.*`, `calls/group/calls_group_panel.*`, `calls/group/ui/desktop_capture_choose_source.cpp` | Own dark palette, custom title, video tiles | Stays opaque, with its own title. |
| Mini Apps and other separate panels | `Ui::SeparatePanel` users: `ui/chat/attach/attach_bot_webview.cpp`, `payments/ui/payments_panel.cpp`, `passport/passport_panel.cpp`, `wallet/wallet_panel.cpp`, `ui/controls/location_picker.cpp`, `export/view/export_view_panel_controller.cpp` | Rounded panel windows (`callRadius` 6) with their own shadow; Mini Apps host a web view | Opaque, as the user decided. The frame takes the popup radius 12 if the user chooses the popup shape. |
| Instant View, its editor and the markdown viewer | `iv/iv_controller.*`, `iv/editor/iv_editor_window.*`, `iv/markdown/iv_markdown_controller.*` | Separate `RpWindow`s around a web view | Stay opaque, with their own title. |
| Notifications | `window/notifications_manager_default.*` | Opaque `RpWidget` windows. These are the Windows default: `Platform::Notifications::ByDefault()` is `false` in `notifications_manager_win.cpp`, so native toasts are opt-in. | The most visible separate window. Opaque; if the user chooses, the popup shape (radius 12, the Popup border). Native toasts stay the system's. |
| Restore shell | `window/window_restore_shell.*` | A placeholder window shown while separate windows reopen | Takes the material, Window role and title painter when material is active, so restoring does not flash an upstream window. |

## Order and next steps

- C17 comes after C8 and before the element plans C9–C13, which build their
  rows, fields and hovers on it, and so before C14 and C15. Its plan maps the surface fills and the opaque
  hover in one `lib_ui` switch, raises three state colors, and checks the
  subgroups that paint no fill.
- C18 comes after C17 and before C9–C13, whose checks include its icons. Its plan raises the grey icon entries,
  implements the system font default only if the user chooses it, and checks
  text rendering and motion. Icon shapes stay Telegram's.
- C19 comes after C14, whose popup radius it reuses, and before C15's final
  pass. Its plan gives the
  restore shell the material, asks the user about the popup shape for
  notifications and separate panels, and checks that every other separate
  window is unchanged. No separate window gets material: the user decided
  they stay opaque.
