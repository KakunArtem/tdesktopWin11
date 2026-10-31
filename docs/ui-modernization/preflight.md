# Plan preflight

Date: 2026-10-31. Sources: tdesktop at `ea2d6dff9d`, `lib_ui` at the pinned
`79c846fd` (now checked out from the fork) and `lib_base` at the pinned
`95a543e4`. Done on macOS, with no build.

The component plans were written partly from `lib_ui` read on GitHub. This
check compares them with the checked-out sources before implementation
starts on Windows, so tasks do not stop on a wrong name or a missed paint
site. Each finding below was fixed in its plan in the same change.

## Method

- Every file path and identifier in backticks in the plans was looked up in
  tdesktop, `lib_ui` and `lib_base`. Of 258 that were not found, all but two
  are names the plans create (new files, functions, styles, prefs and tests)
  or Windows API names.
- Every palette entry the plans read or adjust exists in
  `lib_ui/ui/colors.palette`.
- The "recheck at the pinned commit" steps that can be answered by reading
  code were answered, and the read counts the plans quote were recounted.

## Findings and fixes

| Plan | Finding | Fix |
| --- | --- | --- |
| C3 Material fills | The Popup role's text entry `menuFg` does not exist. Menu items use `defaultMenu.itemFg`, which is `windowFg`. | Popup text is `windowFg`. |
| C11 Conversation chrome | There is no `SponsoredMessageBar` class. The bar is built by `FillSponsoredMessageBar` in `ui/chat/sponsored_message_bar.cpp`, whose `draw` fills `historyPinnedBg`. | The step names that fill. |
| C8 Title bar | `TitleWidget::paintEvent` (`ui_window_title_win.cpp`) fills `bgActive` or `bg` by window activity. The plan's painter used `titleBg` only. | The painter uses `titleBgActive` while active, `titleBg` otherwise. |
| C9 Navigation rail | `SideBarButton` fills `textBgActive` / `textBg`, both style fields, so no fork flag is needed. The badge ring is a transparent cut-out in the cached icon (`validateIconCache`), not a stroke in the button color. | Task 1 Step 1 and Task 2 Step 1 record both; no ring work. |
| C9 Navigation rail | The main menu drawer's rows are `Ui::SettingsButton`s (`mainMenuButton`), not a `Menu`, and neither style has a radius. | Task 3 adds `mainMenuButtonSoftPanels` and paints the rounded hover under the rows. |
| C14 Transient surfaces | Menu highlights are flat fills in `Menu::Action::paintBackground` and `MultilineAction::paintEvent`, and fifteen tdesktop files paint their own item highlight. A flag read only inside `lib_ui` would leave those square. | A shared `Ui::Menu::PaintItemBackground`, used by `lib_ui` and by each listed tdesktop item. |
| C19 Separate windows | `SeparatePanel` reads `callRadius` in three places, not five. The Mini App and wallet panels round their own bottom bars with `callRadius`. | The hook covers the three reads and both hosts' bottom bars. |
| All `lib_ui` plans | Commands pushed to a `fork` remote. With `.gitmodules` on the fork, the submodule's `origin` is the fork, and a detached `HEAD` needs a branch name. | `git -C Telegram/lib_ui push origin HEAD:soft-panels`; the [toolkit fork](toolkit-fork.md) procedure sets the SSH push URL. |

## Confirmed as planned

- C17: `RoundButton` (`_roundRect`, `_roundRectOver`), `FlatButton::paintEvent`,
  `SettingsButton::paintBg`, the `InputField` constructor's
  `WA_OpaquePaintEvent`, `InputField::paintFlatSurrounding`,
  `InputField::paintRoundSurrounding` and `BoxContentDivider::paintEvent`.
- C5: `Ui::GrabWidget(target, rect, bg)` and `Ui::GrabOpaque(target, rect, bg)`
  take a `QColor` background, as assumed.
- C18: `style::SetCustomFont` and `ResolveFont` in `style_core_font.cpp`.
- C19: `RpWindow(bool translucent, QWidget *parent)` exists for the restore
  shell.
- C11: `st::topBarHeight` is read 23 times in 6 files; the plan converts 5
  and keeps the theme preview. C15: `st::infoTopBar` is read in the 7 files
  the plan lists. C9: `st::windowFiltersWidth` is read 3 times.
- C13: `BubbleRadiusLarge()` in `chat_style_radius.cpp` returns
  `bubbleRadiusLarge`.

## Not checked here

Windows-only behavior (DWM, the non-client title, notifications' window
flags at run time), anything that needs a build, and `lib_base`, which no
plan changes.
