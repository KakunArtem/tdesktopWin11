# Opaque paint ownership

C4 audit, 2026-11-04. Scope: every `WA_OpaquePaintEvent` occurrence from
`git grep WA_OpaquePaintEvent -- 'Telegram/SourceFiles/*.cpp'`, plus the
toolkit children that cover the folder rail. Paths below are relative to
`Telegram/SourceFiles/`. Multiple occurrences in one function share a row.
False assignments, reads and comments are included, rather than mistaken
for additional opaque paint owners. The disposable scenario is excluded.

| File | Widget / occurrence | Region | Disposition |
| --- | --- | --- | --- |
| `boxes/share_box.cpp` | Share box inner list | Popup | C14 |
| `boxes/sticker_set_box.cpp` | Sticker set inner list | Popup | C14 |
| `calls/calls_panel.cpp` | Call panel window | Separate window | Stays opaque; native video surface, C19 |
| `calls/calls_video_incoming.cpp` | Incoming video widget | Separate window | Stays opaque; video content, C19 |
| `calls/group/calls_group_panel.cpp` | Group call window | Separate window | Stays opaque; C19 |
| `calls/group/calls_group_viewport.cpp` | Viewport child | Separate window | Stays opaque; GPU/video content, C19 |
| `chat_helpers/emoji_list_widget.cpp` | Emoji list | Popup | C14 |
| `chat_helpers/field_autocomplete.cpp` | Autocomplete visibility/cache transitions (three assignments) | Popup | C14; false assignments already permit transparency |
| `chat_helpers/gifs_list_widget.cpp` | GIF list | Popup | C14; media tiles retain opaque underlays |
| `chat_helpers/stickers_list_widget.cpp` | Sticker list | Popup | C14 |
| `chat_helpers/tabbed_panel.cpp` | Tabbed panel, false assignment | Popup | C14 |
| `chat_helpers/tabbed_section.cpp` | Selector in a section | Conversation | C15; third-column content |
| `chat_helpers/tabbed_selector.cpp` | Selector, false assignment | Popup / conversation | C14 / C15 according to host |
| `chat_helpers/ttl_media_layer_widget.cpp` | TTL media layer, false assignment | Popup | C14; media pixels stay opaque |
| `dialogs/dialogs_inner_widget.cpp` | Chats list constructor | Navigation | Converted C4 through `SetOpaquePaint`; Navigation base and per-row mattes replace pixels; RGB scroll cache bypassed while active |
| `dialogs/dialogs_widget.cpp` | Chat-filter slide canvas | Navigation | Converted C4; opaque snapshots remain owned by C5 |
| `dialogs/dialogs_widget.cpp` | Button rendering workaround comment | Navigation | No assignment; C10 owns the controls |
| `history/admin_log/history_admin_log_section.cpp` | Admin log show/cache transitions (two assignments) | Conversation | C15; specialized section |
| `history/view/controls/history_view_draft_options.cpp` | Draft options box, false assignment | Popup | C14 |
| `history/view/history_view_top_bar_widget.cpp` | Header constructor and animation mode | Conversation | Converted C4; helper preserves current animation intent on live fallback |
| `history/view/history_view_translate_bar.cpp` | Translate button | Conversation | C7 bar layout / C17 controls |
| `info/info_content_widget.cpp` | Info content | Conversation | C15 |
| `info/info_top_bar.cpp` | Info header, conditional on rounding | Conversation | C15 |
| `info/info_wrap_widget.cpp` | Top-bar rendering comment | Conversation | No assignment; C15 |
| `inline_bots/inline_results_inner.cpp` | Inline results list | Popup | C14 |
| `inline_bots/inline_results_widget.cpp` | Results shell, false assignment | Popup | C14 |
| `mainwindow.cpp` | Main window constructor | Window / frame | C4 clears the opaque promise while material is active; title painting remains C8 |
| `media/player/media_player_widget.cpp` | Audio player bar | Conversation | C7 / C17; stays solid until its bar conversion |
| `media/view/media_view_pip.cpp` | PiP, conditional on native transparency | Separate window | Existing policy retained; C19 |
| `settings/sections/settings_notifications.cpp` | Notification preview | Other | C15 settings; preview remains a solid example |
| `settings/settings_intro.cpp` | Settings wrap assignment and rendering comment | Conversation | C15 |
| `ui/boxes/country_select_box.cpp` | Country list | Popup | C14 |
| `ui/chat/group_call_bar.cpp` | Call bar wrapper | Conversation | C7 / C17 |
| `ui/chat/more_chats_bar.cpp` | More-chats bar wrapper | Conversation | C7 / C17 |
| `ui/chat/pinned_bar.cpp` | Pinned bar wrapper | Conversation | C7 / C17 |
| `ui/chat/requests_bar.cpp` | Join requests bar wrapper | Conversation | C7 / C17 |
| `ui/controls/popup_selector.cpp` | Popup selector inner | Popup | C14 |
| `ui/controls/ton_common.cpp` | TON control, false assignment | Other | C17; existing transparency retained |
| `ui/filter_icon_panel.cpp` | Filter icon popup, false assignment | Popup | C14 |
| `ui/ui_material.cpp` | Two helper assignments | Shared policy | C4; current solid-mode intent AND inactive material |
| `ui/widgets/multi_select.cpp` | Multi-select, conditional on background alpha | Other | C17; follows style background |
| `wallet/wallet_content.cpp` | Wallet content | Conversation / specialized screen | C15 / C19; remains opaque |
| `wallet/wallet_window_transition.cpp` | Panel read, overlay read/set and paint read | Specialized overlay | C19; hole-dependent overlay promise retained |
| `window/notifications_manager_default.cpp` | Notification window and inner (two assignments) | Separate window | Stays opaque; C19 |
| `window/window_connecting_widget.cpp` | Connecting indicator | Navigation | C10; temporary solid status overlay |
| `window/window_filters_favorite.cpp` | Favorite rail button | Navigation | C9; independent button background |
| `window/window_filters_menu.cpp` | Rail outer container | Navigation | Converted C4 through `SetOpaquePaint` |
| `window/window_main_menu.cpp` | Main menu | Navigation / popup | C9 |
| `test/test_animation_clock.cpp` | Opaque self-test fixture | Test | Stays opaque to test capture behavior |
| `test/test_capture.cpp` | Two reads and refusal text | Test | Observation only; no product assignment |
| `test/test_layer_root.cpp` | Comment, reads and diagnostic strings | Test | Observation / helper self-test assertions |
| `test/test_mapped_target.cpp` | Opaque capture fixture | Test | Stays opaque to test mapped capture |
| `test/test_post_paint.cpp` | Opaque sampling fixture | Test | Stays opaque to test sampling |
| `test/test_via_window.cpp` | Read and refusal text | Test | Observation only |
| `test/test_window_exposure.cpp` | Two opaque fixture assignments | Test | Stays opaque to test exposure |
| `tests/test_window_material.cpp` | Four helper assertions | Test | Verifies C4 policy; no product owner |

## Other paint owners in these regions

The grep inventory is not an inventory of every solid fill:

| Owner | Disposition |
| --- | --- |
| `MainWindow::_body` | C4 Window role; its frame container inherits the transparent window palette |
| `MainWidget` chat background and `HistoryWidget` wallpaper | C11; wallpaper remains opaque |
| `HistoryWidget::drawField` | C4 Composer role over the complete ordinary-chat field and reply area |
| `ComposeControls::paintBackground` | C4 Composer role for regular-window hosts, including rounded styles; Stories keeps its style |
| `FieldHeader::init` | C4 Composer role on the regular-window reply/edit/forward strip; independent hosts stay unchanged |
| `lib_ui/ui/widgets/side_bar_button.cpp` | C9; buttons still paint their normal/active solid backgrounds over the converted rail container |
| `Dialogs::InnerWidget` selected/hovered rows and badge cutout | C10; solid state colors remain intentional |
| Search field, ordinary/thread input widgets and child buttons | C10 / C12 / C17; readable solid controls remain intentional |
| Scroll viewports and rail layouts | Transparent containers; their content owns painting |

The converted rail container therefore does not yet imply translucent folder
buttons. That remaining owner is explicit, rather than treated as a successful
whole-rail visual acceptance check. See [surface evidence](surface-roles.md).
