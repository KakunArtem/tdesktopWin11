# C5 grab audit

Inventory checked on native Windows on 2026-11-04 against the C4 baseline
and the C5 implementation. Paths below are relative to `Telegram/SourceFiles/`.
Repeated calls in one function are counted explicitly. Counts describe source
call sites, including conditional branches, rather than runtime invocations.

The baseline inventory command was:

```powershell
git grep -n -E "GrabWidget|GrabOpaque|GrabWidgetToImage" b971f26b77 -- "Telegram/SourceFiles/*.cpp"
```

The same search without the baseline revision inventories remaining calls.
Also search `Ui::Material::Grab` for converted callers. The disposable
`test_scenario.cpp` overlay is excluded; it is restored before delivery.

## Converted main-window animation caches

Each content-only snapshot clears role regions within `GrabScope`. Its owner
paints steady roles beneath the animation, including Composer in mixed chat
sections. Clearing also removes previously drawn wallpaper under those roles.
Active-state changes and active
fill publications finish held animations; revision checks reject preparation
that spans an appearance change. Inactive fill publications keep upstream
animation behavior. Durations and easing are unchanged.

| File | Function and calls | Cache / region | Status |
| --- | --- | --- | --- |
| `mainwidget.cpp` | `prepareShowAnimation` (1) | Departing main/third-column region | C5 `Grab`; stamped preparation |
| `mainwidget.cpp` | `grabForShowAnimation` (2 branches) | Main section or dialogs region | C5 `Grab` |
| `mainwidget.cpp` | `showAnimated` (1) | Arriving main scene | C5 `Grab`; Conversation base, Navigation for list-only one-column layout |
| `window/section_widget.cpp` | `SectionWidget::grabForShowAnimation` (1) | Generic main-window section | C5 `Grab`; Conversation animation base |
| `history/history_widget.cpp` | `showAnimated`, `grabForShowAnimation` (1 each) | History section | C5 `Grab`; wallpaper remains intentional content |
| `history/view/history_view_chat_section.cpp` | `ChatWidget::grabForShowAnimation` (1) | Newer chat section | C5 `Grab` |
| `history/view/history_view_pinned_section.cpp` | `PinnedWidget::grabForShowAnimation` (1) | Pinned section | C5 `Grab` |
| `history/view/history_view_scheduled_section.cpp` | `ScheduledWidget::grabForShowAnimation` (1) | Scheduled section | C5 `Grab` |
| `history/view/history_view_welcome_messages_section.cpp` | `WelcomeMessagesWidget::grabForShowAnimation` (1) | Welcome section | C5 `Grab` |
| `history/admin_log/history_admin_log_section.cpp` | `Widget::grabForShowAnimation` (1) | Admin-log section | C5 `Grab` |
| `dialogs/dialogs_widget.cpp` | `Widget::grabForChatsFilterSlide` (1) | Folder/filter scroll frame | C5 `GrabOpaque`; Navigation canvas paints the role once |
| `dialogs/dialogs_widget.cpp` | `Widget::grabForFolderSlideAnimation` (1) | Folder/forum/search subsection | C5 `Grab` |
| `dialogs/dialogs_widget.cpp` | `Widget::showAnimated` (1) | Arriving list | C5 `Grab`; Navigation base |
| `dialogs/dialogs_widget.cpp` | `Widget::openChildList` (1) | Departing portion behind forum topics | C5 `Grab`; stamped child preparation |
| `dialogs/dialogs_widget.cpp` | `Widget::closeChildList` (2) | Departing child and arriving list crop | C5 `Grab`; Navigation canvas |
| `dialogs/ui/dialogs_suggestions.cpp` | `Suggestions::startSlideAnimation` (2) | Departing/arriving suggestion tabs | C5 `Grab`; Navigation base |
| `dialogs/ui/dialogs_suggestions.cpp` | `Suggestions::startShownAnimation` (1) | Search suggestions reveal/hide | C5 `Grab`; finish callback retained on appearance cancellation |
| `dialogs/ui/dialogs_top_bar_suggestion_content.cpp` | `UnconfirmedAuthWrap::prepareCollapseSnapshot` (1) | Collapsing authorization banner | C5 `Grab`; Navigation base |
| `dialogs/ui/dialogs_top_bar_suggestion_content.cpp` | `TopBarSuggestionContent::prepareCollapseSnapshot` (1) | Collapsing suggestion card | C5 `Grab`; Navigation base; pill foreground remains solid |

`Widget::grabNonNarrowScrollFrame` in `dialogs_widget.cpp` is an additional
manual cache: three `Ui::RenderWidget` calls into an initialized ARGB image,
outside the grep above. It now uses an active-only `GrabScope`, revision-checked
publication and a steady Navigation background during width changes. C4's
active bypass of the RGB `RowsScrollCache` remains in force.

## Remaining product calls and owners

These calls retain their existing capture semantics. A future owner is a scope
assignment, not a claim that its material treatment is already implemented.

Owner plans: [C7 shell](../plans/2026-10-29-shell-layout.md),
[C11 messages](../plans/2026-10-29-messages.md),
[C12 composer](../plans/2026-10-29-composer.md),
[C14 transient surfaces](../plans/2026-10-29-transient-surfaces.md),
[C15 info/settings](../plans/2026-10-29-info-settings.md),
[C17 controls](../plans/2026-10-30-shared-controls.md), and
[C19 separate windows](../plans/2026-10-31-separate-windows.md).

| File | Function and calls | Cache / region | Status and owner |
| --- | --- | --- | --- |
| `boxes/background_preview_box.cpp` | `BackgroundPreviewBox::applyForPeer` (1) | Scaled wallpaper preview / box | Unaffected image output; C11 owns wallpaper presentation |
| `boxes/gift_premium_box.cpp` | Spinner overload of `MakeAttributeValue` (1) | Attribute row frame / box | Foreground spinner content; C14 transient surfaces |
| `boxes/star_gift_box.cpp` | `SwitchToUpgradedAnimation` (1) | Old gift content / box | C14 transient surfaces; solid gift foreground |
| `boxes/star_gift_craft_box.cpp` | `MakeCraftingView` (5) | Forge percent, percent badge, remove button, add button, failed radial / box | C14/C17 foreground controls; keep their current captures |
| `calls/group/calls_group_message_field.cpp` | `ReactionPanel::fadeOutSelector`, `MessageField::toggle` (1 each) | Selector and message field / call window | C19 separate windows |
| `chat_helpers/emoji_list_widget.cpp` | `EmojiColorPicker::hideAnimated`, `showAnimated` (1 each) | Picker fade / popup | C14 transient surfaces |
| `chat_helpers/emoji_list_widget.cpp` | `EmojiListWidget::startSearchSwapAnimation` (2) | Search swap / picker | C14 transient surfaces |
| `chat_helpers/emoji_list_widget.cpp` | `EmojiListWidget::lookupChosen` (1 conditional) | Selected emoji image / foreground | Unaffected image content; C14 picker owner |
| `chat_helpers/field_autocomplete.cpp` | `FieldAutocomplete::hideAnimated`, `showAnimated` (1 each) | Autocomplete fade / popup | C14 transient surfaces |
| `chat_helpers/stickers_list_widget.cpp` | `StickersListWidget::startSearchSwapAnimation` (2) | Search swap / picker | C14 transient surfaces |
| `chat_helpers/tabbed_panel.cpp` | `TabbedPanel::prepareCacheFor` (1) | Panel fade / popup | C14 transient surfaces |
| `chat_helpers/ttl_media_layer_widget.cpp` | `PreviewWrap::PreviewWrap` (1) | Last media preview frame / layer | Intentional media image; C14/C19 |
| `editor/color_picker.cpp` | `ColorPicker::grabRow` (1) | Row of control images / editor | Foreground colors; C17 controls/C19 separate editor |
| `history/history_drag_area.cpp` | `DragArea::hideStart`, `showStart` (1 each) | Drag overlay fade / conversation | C12 composer and C17 controls; independent overlay |
| `history/view/controls/history_view_voice_record_bar.cpp` | `ListenWrap::prepareForSendAnimation` (1) | Recorded voice send image / composer | C12 composer |
| `history/view/reactions/history_view_reactions_selector.cpp` | `Selector::initGeometry` (1) | About label / reaction popup | C14 transient surfaces |
| `history/view/reactions/history_view_reactions_selector.cpp` | `Selector::expand` (1) | Deliberately unused render / popup | Existing geometry/render workaround retained; C14 |
| `info/info_wrap_widget.cpp` | `WrapWidget::grabForShowAnimation` (1) | Info/settings section, also layer | C15 owns its still-solid backgrounds and wrap modes; retain together |
| `info/media/info_media_grid_zoom.cpp` | `ListZoom::grabViewport` (1) | Media-grid zoom / info | C15 info/settings; media foreground |
| `info/profile/info_profile_balance_pill.cpp` | `BalancePill::grabEmpty` (1) | Empty pill image / info | C15/C17 foreground pill |
| `info/profile/tabs/info_profile_tabs_host.cpp` | `TabsHost::activateTab`, `startSlideAnimation` (1 each) | Old/new profile tabs / info | C15 info/settings |
| `inline_bots/inline_results_widget.cpp` | `Widget::prepareCache` (1) | Inline results fade / popup | C14 transient surfaces |
| `intro/intro_step.cpp` | `Step::prepareContentSnapshot`, `prepareSlideAnimation` (1 each) | Login-step transitions / intro scene | C15 solid intro foreground |
| `intro/intro_widget.cpp` | `Widget::showAnimated` (1) | Arriving intro scene | C15 intro/lock scene |
| `mainwindow.cpp` | `MainWindow::grabForSlideAnimation` (1) | Shared body departing intro/lock scene | C15 retains opaque scene; `setupMain` does not animate Main-to-Main account switches |
| `media/player/media_player_dropdown.cpp` | `Dropdown::startAnimation` (1) | Player dropdown fade / separate panel | C19 separate windows |
| `media/player/media_player_panel.cpp` | `Panel::startAnimation` (1) | Player panel fade / separate panel | C19 separate windows |
| `media/stories/media_stories_reactions.cpp` | `Reactions::Panel::fadeOutSelector` (1) | Reaction selector / stories viewer | C19 separate viewer; C14 popup treatment |
| `media/view/media_view_playback_sponsored.cpp` | `PlaybackSponsored::Message::startFade` (1) | Sponsored message / media viewer | C19 separate viewer foreground |
| `menu/menu_send.cpp` | `EffectPreview::toggle` (1) | Effect preview bottom / menu | C14 transient surfaces |
| `profile/profile_cover_drop_area.cpp` | `CoverDropArea::setupAnimation` (1) | Profile drop overlay / info | C15 info/settings |
| `ui/chat/message_bar.cpp` | `MessageBar::grabTitleRange`, `grabBodyPart`, `grabTextPart`, `grabImagePart` (1 each) | Individual title/body/text/image fragments | Foreground fragments, C7 shell/C17 controls |
| `ui/controls/send_button.cpp` | `SendButton::grabContent` (1) | Send glyph / composer | C12/C17 foreground control |
| `ui/controls/tabbed_search.cpp` | `TabbedSearch::grab` (1) | Search control image | C14/C17 foreground control |
| `ui/controls/userpic_button.cpp` | `UserpicButton::grabOldUserpic` (1) | Previous photo image | Unaffected photo foreground; C17 control owner |
| `ui/effects/peer_qr_panel.cpp` | `PeerQrPanel::toggle` (1) | QR content fade / panel | C14/C19; intentional QR image |
| `ui/filter_icon_panel.cpp` | `FilterIconPanel::prepareCacheFor` (1) | Filter icon panel fade / popup | C14 transient surfaces |
| `ui/widgets/sliding_tabs.cpp` | `SlidingTabs::showTab` (2) | Generic old/new tab contents | C15/C17 owns generic solid controls; not a C5 material owner |
| `wallet/wallet_card_view.cpp` | `CardView::paintEvent` (1), `grabParts` (2) | Card and outlined button images / wallet | C15/C19 specialized foreground |
| `window/notifications_manager_default.cpp` | `Notification::prepareActionsCache` (1) | Reply action image / notification window | C19 separate notification; current background retained |
| `window/themes/window_theme_warning.cpp` | `WarningWidget::startAnimation` (1) | Theme warning fade / overlay | C14/C15 transient/theme UI |
| `window/window_lock_widgets.cpp` | `LockWidget::showAnimated` (1) | Arriving lock scene | C15 intentional solid lock foreground |
| `window/window_setup_email.cpp` | `SetupEmailLockWidget::showEmailInput`, `grabContent`, `SetupEmailConfirmWidget::showAnimated` (1 each) | Email lock transitions | C15 intentional solid lock foreground |

## Utility and test hits

These are neither additional product animation owners nor omitted conversions.

| File | Hits / purpose | Status |
| --- | --- | --- |
| `ui/ui_material.cpp` | `Grab` uses upstream `Ui::GrabWidget` in active and fallback branches; `GrabOpaque` uses upstream `Ui::GrabOpaque` in fallback | C5 adapter boundary; inactive captures preserve upstream arguments |
| `test/test_activation.cpp` | Capture API mentioned in a comment | Unaffected harness documentation |
| `test/test_animation_clock.cpp` | Diagnostic text and `RunClockedWidgetFrames` capture call | Unaffected harness observation |
| `test/test_box_button.cpp` | `ProbeTextHeavyBoxButton` shell image | Unaffected harness observation |
| `test/test_capture.cpp` | Two sentinel captures, `GrabWidget` definition and upstream call, `GrabRect` upstream call, own-paint and region capture consumers | Unaffected harness observation; includes declaration-name grep hit |
| `test/test_hover.cpp` | Fixture comment and four container/frame captures | Unaffected harness observation |
| `test/test_secrecy_scan.cpp` | Diagnostic text naming capture helpers | Unaffected harness diagnostic |
| `test/test_toast_capture.cpp` | Comment and diagnostic text naming capture helpers | Unaffected harness diagnostic |
| `tests/test_text.cpp` | Five `InputField` raster captures | Unaffected text tests |
| `tests/test_window_material.cpp` | Upstream `GrabWidget`/`GrabOpaque` images used as inactive parity references | C5 regression tests; deliberately bypass the material adapter for the reference |

## Evidence and limits

See [transparent-grabs.md](transparent-grabs.md) for native animation-frame
results, fallback and preparation tests, and visible-desktop acceptance limits.
