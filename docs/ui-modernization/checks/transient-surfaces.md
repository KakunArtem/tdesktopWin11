# C14 transient surfaces checks

Date: 2026-11-07. Task 1 is done. Task 2 is partial, and Task 3 is built. Task 4, the popup
Acrylic prototype, has not been started. Task 5 is partial: the automated checks are done,
but nothing has been seen in a real window. Plan:
[transient surfaces](../plans/2026-10-29-transient-surfaces.md).

## Inventory

| Surface | Code | Kind | Style chosen by | Radius before | Under Soft Panels |
| --- | --- | --- | --- | --- | --- |
| Context menus, send menu, tray menu | `Ui::PopupMenu` (`lib_ui`) | Top-level window | App or `defaultPopupMenu` | 8 | 12, rounded rows |
| Submenus | `Ui::PopupMenu` | Top-level window | Parent menu | 8 | 12, rounded rows |
| Dropdown menus | `Ui::DropdownMenu`, `Ui::InnerDropdown` (`lib_ui`) | Child layer | App | `innerDropdownRadius` | Same shape, rounded rows |
| Tooltip | `Ui::Tooltip` (`lib_ui`) | Top-level window | `defaultTooltip` | None | Unchanged |
| Important tooltip | `Ui::ImportantTooltip` (`lib_ui`) | Child layer | App or default | 4 | Unchanged |
| Toast | `Ui::Toast` (`lib_ui`) | Child layer | App or `defaultToast` | 8 | Unchanged, by the reference |
| Boxes | `Ui::BoxLayerWidget` (`lib_ui`) | Child layer | `boxRadius`, read directly | 8 | Unchanged, see below |
| Emoji, sticker and GIF panel | `ChatHelpers::TabbedPanel`, `TabbedSelector` | Child layer | App | 8 | 12, with a border |
| Emoji skin tone picker | `EmojiColorPicker` | Child layer | Panel | 8 | 12 |
| Third column selector | `ChatHelpers::TabbedSection` | Column | App | 0 | Unchanged, square in its column |
| Autocomplete | `ChatHelpers::FieldAutocomplete` | Child layer | App | 0, full chat width | 12, Popup fill, composer width |
| Support autocomplete | `Support::Autocomplete` | Child layer | App | 0 | Unchanged |
| Inline bot results | `InlineBots::Layout::Widget` | Child layer | App | 4 | Unchanged, see below |
| Reaction selector | `HistoryView::Reactions::Selector` | Inside a popup menu | App | Strip, then 8 | Strip, then 12 |
| Folder icon picker | `Ui::FilterIconPanel` | Child layer | App | 8 | 12, with a border |

These tdesktop menu items paint their own highlight, and now share the rounded one:
`calls_volume_item.cpp`, `calls_group_menu.cpp` (two), `attach_bot_downloads.cpp`,
`who_reacted_context_action.cpp` (two, and its two pill brushes stay),
`delete_message_context_action.cpp`, `peer_qr_box.cpp`, `bank_card_click_handler.cpp`,
`phone_click_handler.cpp`, `bot_attach_web_view.cpp`, `history_view_context_menu.cpp`,
`history_view_translate_bar.cpp`, `chat_search_in.cpp`,
`iv_editor_insert_suggestions.cpp`, `editor_audio_menu.cpp` and
`photo_editor_controls.cpp`.

## Implemented behavior

Under Soft Panels, context menus have 12 px corners and hovered rows are 8 px pills inset
by 4 px, with the ripple in the same shape. The emoji panel, its skin tone picker, the
folder icon picker and the reaction selector's expanded panel have 12 px corners. The two
floating panels also have a thin border. Autocomplete is a rounded Popup surface with the
composer's width, 6 px above it, with rounded hover rows and no separator lines.

## Changes from the plan

These need the user's view, because they keep something the plan asked to change:

- **Panels keep their opaque fills, not the 0.97 Popup role.** The emoji panel is painted
  by the selector, the lists, the footer and the tabs, and several of them need the color:
  the skin tone picker, the footer's fade gradient and the reaction selector's buffers.
  At 0.97 the difference is about 3% of the backdrop. Only autocomplete, with one paint
  site, uses the Popup role.
- **Boxes keep 8 px.** `st::boxRadius` is read in 131 places across 57 files, and box
  content such as premium headers is clipped to that value. A 12 px box would need those
  audited, or corners of content would poke out past it.
- **Inline bot results keep 4 px.** Its scroll area starts 4 px from the edge and runs the
  full height, so the list would paint over a 12 px corner and leave it square.

These are smaller:

- `Ui::PopupPanelRadius()` replaces the emoji panel radius only where it draws a shape.
  Where the old value is layout, such as the scroll area's left offset, the search margin
  and the selector's strip, `st::emojiPanRadius` stays. Using 12 there would narrow the
  emoji grid by 4 px and could drop a column.
- The footer's selection pill keeps 8 px, matching the menu rows.
- The fork has no new `SetSoftMenus`. `Ui::SoftControls()` already means the same thing, and
  the app sets it once.
- `who_reacted_context_action.cpp` has three highlight fills by the plan's count. `WhenAction`
  fills only `itemBg` and has a pill for its button, so it was left.
- Borders are drawn 1 px outside the panel, by the panel itself, so the selector's children
  cannot cover them.
- `FieldAutocomplete::setComposerBoundings` holds the composer inset, so the autocompletes
  in boxes keep their own bounds. They still take the rounded shape.

## Automated evidence

A disposable `-testagent` overlay builds a real `Ui::PopupMenu` with real `Menu::Action`
rows and shows it off screen, in day and night, with Soft Panels on and off, material on
and off, at 100% and 150%. It passes 57 checks per run, 228 in all:

- With Soft Panels on, the hovered row leaves a gutter, starts at the 4 px margin, and has
  rounded corners. The menu's corner pixel is nearly transparent, alpha 10.
- With Soft Panels off, the hovered row is square and the same corner pixel is mostly
  filled, alpha 192.
- `PaintItemBackground` at rest and hovered, on both edges.
- `PopupPanelRadius()` follows Soft Panels, and `emojiPanRadius` stays 8.

With Soft Panels off, all menu captures are byte-identical to a baseline taken before any
change.

Regressions, on this change: [messages](messages.md#automated-evidence) 256 pass,
[chat list](chat-list.md#automated-evidence) 480,
[foundations](foundations.md#automated-evidence) 592,
[navigation rail](navigation-rail.md#automated-evidence) 332,
[shared controls](shared-controls.md#automated-evidence) 308,
[composer](composer.md#automated-evidence) 192,
[bars](conversation-chrome.md#automated-evidence) 532 and
[title bar](title-bar.md#automated-evidence) 1024. Every capture compared is
byte-identical to the run before this change. `test_window_material` passes 147 checks.

`TabbedPanel`, `FieldAutocomplete` and the reaction selector need a session, so this test
does not build them. Their shapes have not been painted by any test.

## Not verified

There is no test-server account on this machine, and all of this needs a real window:

- The emoji, sticker and GIF panel: corners, border, show and hide animation, switching
  tabs, the search field, and the skin tone picker.
- Autocomplete: `@`, `:`, `/` and `#` lists, sticker suggestions, width and gap above the
  composer, keyboard selection and scrolling.
- The reaction selector, expanded and collapsed.
- Context menus on chats, messages, links and the tray, each custom menu item above, and
  the send menu.
- A submenu near the right screen edge, keyboard navigation, disabled items and dismissal.
- The folder icon picker.
- Contrast over a real backdrop, Scale beyond 150%, and RTL.

## Not started

Task 4, the popup Acrylic prototype, is not built. It needs a decision record from a real
desktop, so it should be judged after the shapes above are seen.
