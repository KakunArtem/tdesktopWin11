# Soft Panels redesign architecture

Date: 2026-10-29, updated 2026-10-30. Status: proposal for review. This
document expands the [implementation roadmap](implementation-plan.md) into the
complete list of components to build, the contracts between them and one plan
per component. It is also the main record of the redesign's
[progress](#progress) and [open decisions](#open-decisions). It changes no
confirmed user decision; proposals and assumptions are labeled.

Inputs: the [project brief](README.md), the
[readable Soft Panels exploration](explorations/README.md) (the appearance
target), the [approved reference](reference/README.md), the
[element map](element-map.md), the [design directions](design-directions.md)
and the [native backdrop prototype plan](plans/2026-10-28-native-backdrop-prototype.md).

## Progress

Snapshot 2026-10-30: every component has a plan and a visual reference; no
plan has started, and no product code exists. Rows are in execution order.

Update a row whenever a plan task commits: the tasks done, the status and the
record the plan writes (paths are under `docs/ui-modernization/`). Statuses
are Not started, In progress, Blocked (say on what) and Done (every task
committed and the plan's "Done when" list holds). A plan corrected before
execution keeps its row; note the correction in the plan.

| ID | Component | Plan | Visual reference | Tasks done | Status | Record |
| --- | --- | --- | --- | --- | --- | --- |
| C1 | Native backdrop | [plan](plans/2026-10-28-native-backdrop-prototype.md) | [reference](component-references/c1-native-backdrop/README.md) | 0 / 7 | Not started | `decisions/2026-10-native-backdrop.md` |
| C16 | Verification tooling | [plan](plans/2026-10-29-verification-tooling.md) | [reference](component-references/c16-verification-tooling/README.md) | 0 / 4 | Not started | `tools/`, `checks/README.md` |
| C2 | Material state | [plan](plans/2026-10-29-material-state.md) | [reference](component-references/c2-material-state/README.md) | 0 / 4 | Not started | `checks/material-state.md` |
| C3 | Material fills | [plan](plans/2026-10-29-material-fills.md) | [reference](component-references/c3-material-fills/README.md) | 0 / 3 | Not started | unit tests |
| C4 | Surface roles | [plan](plans/2026-10-29-surface-roles.md) | [reference](component-references/c4-surface-roles/README.md) | 0 / 5 | Not started | `checks/surface-roles.md`, `checks/opaque-paint-audit.md` |
| C5 | Transparent grabs | [plan](plans/2026-10-29-transparent-grabs.md) | [reference](component-references/c5-transparent-grabs/README.md) | 0 / 4 | Not started | `checks/grab-audit.md` |
| C6 | Toolkit fork | [plan](plans/2026-10-29-toolkit-fork.md) | [reference](component-references/c6-toolkit-fork/README.md) | 0 / 3 | Not started | `toolkit-fork.md` |
| C7 | Shell layout | [plan](plans/2026-10-29-shell-layout.md) | [reference](component-references/c7-shell-layout/README.md) | 0 / 4 | Not started | `checks/shell-layout.md` |
| C8 | Title bar | [plan](plans/2026-10-29-title-bar.md) | [reference](component-references/c8-title-bar/README.md) | 0 / 3 | Not started | `checks/title-bar.md` |
| C9 | Navigation rail | [plan](plans/2026-10-29-navigation-rail.md) | [reference](component-references/c9-navigation-rail/README.md) | 0 / 4 | Not started | `checks/navigation-rail.md` |
| C10 | Chat list and search | [plan](plans/2026-10-29-chat-list.md) | [reference](component-references/c10-chat-list/README.md) | 0 / 4 | Not started | `checks/chat-list.md` |
| C11 | Conversation chrome | [plan](plans/2026-10-29-conversation-chrome.md) | [reference](component-references/c11-conversation-chrome/README.md) | 0 / 4 | Not started | `checks/conversation-chrome.md` |
| C12 | Composer | [plan](plans/2026-10-29-composer.md) | [reference](component-references/c12-composer/README.md) | 0 / 4 | Not started | `checks/composer.md` |
| C13 | Messages | [plan](plans/2026-10-29-messages.md) | [reference](component-references/c13-messages/README.md) | 0 / 3 | Not started | `checks/messages.md` |
| C14 | Transient surfaces | [plan](plans/2026-10-29-transient-surfaces.md) | [reference](component-references/c14-transient-surfaces/README.md) | 0 / 5 | Not started | `checks/transient-surfaces.md`, `decisions/popup-acrylic.md` |
| C15 | Info, settings and specialized screens | [plan](plans/2026-10-29-info-settings.md) | [reference](component-references/c15-info-settings/README.md) | 0 / 4 | Not started | `checks/final.md` |

## Open decisions

The plans are written against the assumption in the second column. Each
decision is the user's; "Needed before" is the last point where an answer
changes no written code. Record answers in the [project brief](README.md) as
user decisions with the date, then remove the row here.

| Decision | Plans assume | Needed before | Raised in |
| --- | --- | --- | --- |
| Phase 1 result: go with Acrylic, Mica or no-go | Go with Acrylic | C2 | C1 decision record; see [What a Phase 1 no-go changes](#what-a-phase-1-no-go-changes) |
| Minimum Windows build for material | 22621 (Windows 11 22H2) | C2 | Roadmap decision 1 |
| Material refused while the native frame is on | Yes | C2 | Roadmap decision 5 |
| Fork `lib_ui` and `lib_base` under your account | `KakunArtem/lib_ui` and `KakunArtem/lib_base`, branch `soft-panels` | C6 Task 1 Step 2 | Roadmap decision 2 |
| Soft Panels layout on macOS and Linux | Compiled everywhere; the switch exists only on Windows | C7 (C15 Task 3 asks it, after the switch is built) | Roadmap decision 4 |
| Default `lib_ui` styles changed on every platform | No; parallel app styles and fork flags | C14 | C14 Global Constraints |
| Night theme badge contrast: both unread badges are below 3:1 on the material, the muted one already 2.08:1 on the opaque list | Keep today's fills | C10 Task 3 Step 2 | [C10 reference](component-references/c10-chat-list/README.md#findings) |
| Mockup values not adopted: rail width 64 (72 kept), header 56 (54 kept), bubble radius 14 (16 kept), light active chat row, composer drop shadow | Telegram's values | Any time; each is a style value | C9–C13 Global Constraints |
| Window transparency on by default; chat background default | Off; the user's wallpaper | C15 Task 3 | Roadmap decision 3 |
| Soft Panels layout on by default | Off | C15 Task 3 | C15 |
| Popup Acrylic kept and promoted to a setting | Off, experimental | C15 Task 3 | C14 Task 4 decision record |

## Confirmed direction

Recorded by the user and repeated here only so that the component contracts
can refer to it:

- Soft Panels with thin borders, translucent contacts and conversation panels,
  and readable message content and inputs.
- The readable exploration is the appearance target. Acrylic is the working
  guideline for the native material; Mica stays the alternative.
- Telegram behavior is preserved; only presentation changes.

## Assumptions used by every plan

These are the roadmap's proposals for its open decisions. Each plan depends
on them; if the user decides otherwise, the plans named in the last column
change.

| Assumption | Source | Plans affected |
| --- | --- | --- |
| Material requires Windows 11 build 22621 or later. | Roadmap decision 1 | Material state |
| `lib_ui` and `lib_base` are forked under the user's account before Phase 3. | Roadmap decision 2 | Toolkit fork, Title bar, Transient surfaces |
| "Window transparency" is off by default until Phase 9; the chat background in material mode stays the user's wallpaper. | Roadmap decision 3 | Material state, Conversation chrome |
| The Soft Panels layout and element styles are compiled for every platform, but the layout switch and material exist only on Windows. | Roadmap decision 4 (proposal) | Shell layout and every element plan |
| Material is refused while the native window frame is on. | Roadmap decision 5 | Material state, Title bar |
| The Phase 1 prototype ends in go or conditional go with Acrylic. | Phase 1 plan | Every plan after Phase 1 |

A no-go in Phase 1 does not cancel the layout and element plans: they work in
solid mode. It replaces the Native backdrop component and adjusts the
Material state plan; see [What a Phase 1 no-go changes](#what-a-phase-1-no-go-changes).

## Layer model

The redesign is five layers. Upper layers depend only on the interfaces of
lower layers, never on Windows APIs.

```text
  Element styling     rail, chat list, header and bars, composer, messages,
                      transient surfaces, info and settings
        |  uses roles, fills, tokens
  Shell layout        Soft Panels frame, panel containers, title area
        |  uses roles, fills
  Surface roles       one paint helper per fill site, opaque-paint audit,
                      transparent grabs
        |  uses state, fills
  Material state      policy, setting, live state producer, fill calculator
        |  uses backdrop result
  Native backdrop     DWM backdrop and frame extension (Windows only)
```

Two properties hold at every layer:

- **Solid parity.** With material inactive, every role paints exactly the
  opaque palette color it painted before, so the app looks like the Soft
  Panels layout without transparency. With the Soft Panels switch also off,
  the app looks like upstream Telegram.
- **Paint once.** A translucent fill is painted by exactly one widget per
  screen region. Children over it paint nothing there; otherwise alpha
  accumulates and the region turns opaque.

## Verified source facts

Facts from the roadmap stay valid. This survey (2026-10-29, macOS checkout
with uninitialized submodules) adds the following, all in `Telegram/SourceFiles/`.
`lib_ui` facts come from the roadmap's Windows survey and must be rechecked at
the pinned submodule commit before editing.

| Fact | Where | Consequence |
| --- | --- | --- |
| 53 `.cpp` files set `Qt::WA_OpaquePaintEvent`, among them the folder rail container, the chats list inner widget, the chat header, the info content widget and the main menu. | `grep -rln WA_OpaquePaintEvent` | Each widget over a translucent region must drop the attribute in material mode, or Qt skips painting what is behind it and leaves stale pixels. |
| The folder rail container `_outer` is opaque and fills itself with `st::windowFiltersButton.textBg`. | `FiltersMenu::FiltersMenu` in `window/window_filters_menu.cpp` | The rail is one paint site plus one opaque attribute. |
| The rail is created as a child of the window body by `SessionController::toggleFiltersMenu`. | `window/window_session_controller.cpp` | The navigation panel container must contain the rail and the chats list together. |
| `MainWidget::updateControlsGeometry` places the chats list, the side shadow, the main section and the third column with no gaps. | `mainwidget.cpp` | Panel insets and gaps are added in one function. |
| `Window::MainWindow::updatePalette` sets `QPalette::Window` to `st::windowBg`. | `window/main_window.cpp` | Material mode must keep that palette entry transparent. |
| `MainWindow::updateControlsGeometry` stacks the title shadow, outdated bar, screen reader bar, right column and body. | `window/main_window.cpp` | The outer frame inset belongs to the body geometry. |
| The chats list fills above, between and below its scroll area in `Dialogs::Widget::paintEvent`. | `dialogs/dialogs_widget.cpp` | Three fill rectangles become one role call. |
| `HistoryWidget::paintEvent` calls `SectionWidget::PaintBackground` and `HistoryWidget::drawField` fills `st::historyReplyBg`. | `history/history_widget.cpp` | The wallpaper and the older composer have separate paint sites. |
| `ComposeControls` fills `st::historyComposeAreaBg` in its paint request. | `history/view/controls/history_view_compose_controls.cpp` | The newer composer has its own paint site. |
| `TopBarWidget::paintEvent` fills `st::topBarBg` over `st::topBarHeight` (54 px in `info/info.style`). | `history/view/history_view_top_bar_widget.cpp` | Header fill is one site; its opaque attribute toggles during animation. |
| `Ui::EnsureBubblesReadable`, `Ui::IsDarkPalette` and `Ui::EnsurePeerNameReadable` already enforce readability floors. | `ui/chat/chat_theme_readability.h` | The fill calculator reuses the dark-palette test and follows the same floor idea. |
| Bubble radii come from `BubbleRadiusSmall()` / `BubbleRadiusLarge()` with an existing small-radius option. | `ui/chat/chat_style_radius.h`, `ui/chat/chat.style` | Bubble shape changes are style values, not new painting. |
| 33 `Ui::GrabWidget` call lines in `window/`, `history/`, `dialogs/` and `mainwidget.cpp`; 58 files use it app-wide. | `grep -rn GrabWidget` | The roadmap counted 27 sites; this count includes `mainwidget.cpp`. The grab plan audits by file instead of relying on a count. |
| `Core::Settings::writePref` / `readPref` store KV prefs; `Window::Theme::IsNightModeValue()` reports night mode. | `core/core_settings.h`, `window/themes/window_theme.h` | The setting needs no serialization change; night mode is observable. |
| Unit tests live in `tests/test_*.cpp` with CMake files under `Telegram/cmake/`; the `-testagent` harness in `test/` captures widget pixels. | `tests/`, `test/README.md` | Pure logic is unit-tested; widget captures cannot see DWM composition, so desktop checks stay in PowerShell. |
| `ui/chat/` bars, `ui/controls/send_button.cpp`, `window/window_slide_animation.cpp` and `ui/chat/chat_theme_readability.cpp` are built in the `td_ui` library; `dialogs/ui/dialogs_layout.cpp`, `history/` sections and `window/section_widget.cpp` in the `Telegram` target. | `Telegram/cmake/td_ui.cmake`, `Telegram/CMakeLists.txt` | Paint-time material state must live in `td_ui`; see [Two namespaces, two targets](#two-namespaces-two-targets). |
| `ComposeControls` already paints a rounded background when its style's `radius` is above zero (`ComposeControls::paintBackground`, used by `storiesComposeControls`). | `history/view/controls/history_view_compose_controls.cpp`, `media/view/media_view.style` | The floating composer is a style plus placement, not new painting. |
| `ChatStyle::applyAdjustedServiceBg` overrides the wallpaper-derived service color; `SessionController::setChatStyleTheme` applies the theme to the chat style. | `ui/chat/chat_style.cpp`, `window/window_session_controller.cpp` | Service colors on the material background follow an existing override path. |
| Default secondary text `#999999` on white is 2.85:1; the exploration's colors give 0.78 navigation fill at 4.5:1 (light). | Calculated with the WCAG formula used by `Ui::CountContrast` | Fills alone cannot make default themes translucent; C3 adjusts secondary text. |

## Components

Each component has one responsibility, one owner directory and one plan.
"Phase" refers to the roadmap. IDs are used in plans and commit discussions.

| ID | Component | Responsibility | Main files | Depends on | Phase | Plan |
| --- | --- | --- | --- | --- | --- | --- |
| C1 | Native backdrop | Apply and refresh the DWM backdrop; report success. | `platform/win/window_material_win.*` | — | 1 | [prototype](plans/2026-10-28-native-backdrop-prototype.md) |
| C2 | Material state | Policy, user setting, startup capability and live state producer; production backdrop lifecycle. | `window/window_material_policy.*`, `window/window_material.*` | C1 | 2 | [material state](plans/2026-10-29-material-state.md) |
| C3 | Material fills | Contrast-driven fill colors and derived border colors from the active palette. | `window/window_material_fills.*` | C2 | 2 | [material fills](plans/2026-10-29-material-fills.md) |
| C4 | Surface roles | Role painting helper, paint-site conversion, opaque-paint audit, window palette. | `ui/ui_material.*` (`td_ui`), existing paint sites | C2, C3 | 2 | [surface roles](plans/2026-10-29-surface-roles.md) |
| C5 | Transparent grabs | Section and slide animations without opaque flashes. | `Ui::GrabWidget` callers, `window/window_slide_animation.*` | C4 | 2 | [transparent grabs](plans/2026-10-29-transparent-grabs.md) |
| C6 | Toolkit fork | Forked `lib_ui` / `lib_base` remotes, submodule pointers and the rule for upstream merges. | `.gitmodules`, `Telegram/lib_ui` | — | before 3 | [toolkit fork](plans/2026-10-29-toolkit-fork.md) |
| C7 | Shell layout | Soft Panels switch, outer frame, gaps and rounded panel containers in one-, two- and three-column layouts. | `window/window_soft_panels.*`, `mainwidget.cpp`, `window/main_window.cpp` | C4, C5 | 3 | [shell layout](plans/2026-10-29-shell-layout.md) |
| C8 | Title bar | Transparent title area with the existing caption buttons, drag regions and inactive state. | forked `lib_ui/ui/platform/win/`, `platform/win/main_window_win.cpp` | C3, C6, C7 | 3 | [title bar](plans/2026-10-29-title-bar.md) |
| C9 | Navigation rail | Folder rail and main menu entry on the shared navigation panel. | `window/window_filters_menu.*`, `window/window.style`, `window/window_main_menu.*` | C4, C7 | 4 | [navigation rail](plans/2026-10-29-navigation-rail.md) |
| C10 | Chat list and search | Search field, rows, states, archive, topics and stories strip. | `dialogs/`, `dialogs/dialogs.style` | C4, C7 | 4 | [chat list](plans/2026-10-29-chat-list.md) |
| C11 | Conversation chrome | Header, pinned and contextual bars, "Window material" chat background. | `history/view/history_view_top_bar_widget.*`, `ui/chat/*_bar.*`, `window/section_widget.cpp` | C4, C7 | 5 | [conversation chrome](plans/2026-10-29-conversation-chrome.md) |
| C12 | Composer | Floating rounded composer in both implementations with every state. | `history/view/controls/`, `history/history_widget.cpp`, `chat_helpers/chat_helpers.style` | C4, C7 | 6 | [composer](plans/2026-10-29-composer.md) |
| C13 | Messages | Bubbles, service messages, date pills, reactions and attachment cards. | `ui/chat/chat.style`, `history/view/`, `history/view/media/` | C3, C11 | 7 | [messages](plans/2026-10-29-messages.md) |
| C14 | Transient surfaces | Menus, tooltips, pickers, boxes and toasts; optional popup Acrylic. | forked `lib_ui/ui/widgets/`, `lib_ui/ui/layers/`, `chat_helpers/`, `boxes/` | C6, C9–C13 | 8 | [transient surfaces](plans/2026-10-29-transient-surfaces.md) |
| C15 | Info, settings and specialized screens | Info pane, settings, lock and intro screens, final accessibility pass. | `info/`, `settings/`, `intro/`, `window/window_lock_widgets.*`, forked `lib_ui` settings rows | C14 and every earlier plan | 9 | [info and settings](plans/2026-10-29-info-settings.md) |
| C16 | Verification tooling | Backdrop, contrast and screenshot checks reused by every phase. | `docs/ui-modernization/tools/`, `tests/` | C1 | 1–9 | [verification tooling](plans/2026-10-29-verification-tooling.md) |

Order: C1 → C2 → C3 → C4 → C5; C6 any time before C8; C7 → C8; then C9–C12
in parallel, with C13 after C11; C14; C15. C16 grows alongside: its contrast tool is needed by C3
and its screenshot matrix by C7.

## Coverage of the element map

Every family of the [element map](element-map.md) has an owner, or a stated
reason why no plan changes it.

| Element family | Owner | Notes |
| --- | --- | --- |
| Visual foundations | C2, C3, C4, C7 | Roles, tint, radii and borders. Typography, icons and motion keep Telegram's values. |
| Window shell | C7, C8 | Frame, gaps, panels, title area and caption buttons. |
| Shared controls | None | Buttons, fields, toggles, checkboxes, sliders and scrollbars keep their upstream look over the role of their panel. Changed only where an element plan needs it: the search field (C10), settings rows (C15) and the history scroll colors (C13). |
| Navigation and accounts | C9 | The account switcher lives in the main menu; the narrow chats list is C10. |
| Chat list and search | C10 | |
| Conversation header and bars | C11 | |
| Messages and conversation canvas | C11, C13 | C11 owns the chat background option, C13 the colors over it. |
| Message attachments | C13 | Verified, not restyled; GPU children keep opaque fills. |
| Composer | C12 | Autocomplete above the composer is C14. |
| Transient surfaces | C14 | |
| Info and settings | C15 | |
| Secondary and specialized screens | C15 | Lock and sign-in only. Media viewer, stories, calls, Mini Apps and notifications are separate windows and stay opaque (C4, C15). The audio player and call bars sit in the conversation panel (C7). |

## Shared contracts

Plans refer to these names. A plan that needs a different shape amends this
section in the same change.

### Two namespaces, two targets

Most widgets to restyle live in the `Telegram` target, but `ui/chat/` bars,
`ui/controls/`, `dialogs/ui/dialogs_stories_list.cpp`,
`window/window_slide_animation.cpp`, `ui/chat/chat_theme_readability.cpp` and
`ui/color_contrast.cpp` are built in the separate `td_ui` library
(`Telegram/cmake/td_ui.cmake`), which cannot call into the `Telegram` target.
The material code is therefore split:

- `Window::Material` (`window/window_material*`, `Telegram` target) decides:
  policy, setting, capability, live refresh, fill calculation and palette
  adjustment. It is the only writer of paint-time state.
- `Ui::Material` (`ui/ui_material.{h,cpp}`, `td_ui` target) holds paint-time
  state and every painting helper, readable from both targets.

### Material state (C2)

`Window::Material` in `window/window_material.h`:

| Name | Meaning |
| --- | --- |
| `bool Capable()` | Fixed at startup: the main window was created translucent and the backdrop call succeeded. |
| `bool Active()` / `rpl::producer<bool> ActiveValue()` | `Capable()` and the live environment still allows material (transparency effects on, high contrast off). Changes at runtime without restart. |
| `void Refresh()` | Re-read the environment after `WM_SETTINGCHANGE` and update `ActiveValue()`. |

Every `ActiveValue()` change is forwarded to `Ui::Material::SetActive`.

### Paint-time state (C4)

`Ui::Material` in `ui/ui_material.h` (`td_ui`):

| Name | Meaning |
| --- | --- |
| `enum class Surface` | The roles listed under Material fills. |
| `bool Active()` / `rpl::producer<bool> ActiveValue()` | Mirror of `Window::Material::ActiveValue()`; `false` until set. |
| `QColor Fill(Surface)` / `QColor Border(Surface)` / `rpl::producer<> FillsChanged()` | Cached fills published by C3. |
| `bool SoftPanels()` / `rpl::producer<bool> SoftPanelsValue()` | The Soft Panels layout switch (C7), fixed at startup because the switch needs a restart; `false` on platforms other than Windows. |
| `void SetActive(bool)`, `void SetFills(...)`, `void SetSoftPanels(bool)` | Writers, called only from the `Telegram` target. |

The policy (`window/window_material_policy.h`, from Phase 1) keeps its pure
`Decide(const Environment&)`. C2 replaces the prototype option with the user
setting and adds the runtime fields used by `Active()`.

The setting is the KV pref `window_material` read through
`Core::Settings::readPref<bool>`; it requires a restart because the window's
translucency is fixed before its native handle exists.

### Material fills (C3)

`Surface` is declared in `ui/ui_material.h`; the pure calculator is
`Window::Material` in `window/window_material_fills.h`:

```cpp
enum class Surface { // Ui::Material::Surface
	Window,       // outer frame and gaps
	Navigation,   // folder rail and chats list, one shared panel
	Conversation, // message canvas, header area included
	Bar,          // pinned and contextual bars, search field, date pills
	Composer,     // floating composer
	Popup,        // menus and tooltips when they are in-app layers
};

struct FillInputs {
	QColor surface;       // today's opaque palette color for the role
	QColor text;          // primary text drawn on it
	QColor secondaryText; // smallest secondary text drawn on it
	bool dark = false;
};

[[nodiscard]] QColor ComputeFill(Surface surface, const FillInputs &inputs);
[[nodiscard]] QColor BorderColor(const QColor &text);
```

A fill is the role's total coverage over the backdrop, never a layer on top
of another role, which is why `PaintSurface` paints with
`CompositionMode_Source`. `ComputeFill` returns the role color with the
lowest alpha, not below the role's floor, at which both text colors reach
their target over the fill composited on black and on white. The target is
4.5:1, or the text's contrast on the opaque surface when that is lower, so
material is never less readable than solid mode. Floors (light / dark) are the
exploration's effective coverage: Window 0.72 / 0.74, Navigation 0.78 / 0.82,
Conversation 0.79 / 0.81; Bar 0.90, Composer 0.96 and Popup 0.97 are
proposals from its pill, composer and popup fills.

Telegram's default secondary text (`windowSubTextFg`) is below 4.5:1 even on
its opaque background, so with unchanged text colors every fill would be
opaque. While material is active, C3 therefore raises the contrast of a fixed
list of secondary text palette entries against the role floors, through the
mechanism `Ui::EnsureBubblesReadable` already uses in
`ui/chat/chat_theme_readability.cpp`. The exploration does the same with its
stronger text colors. Results are recomputed on `style::PaletteChanged()` and
on every `ActiveValue()` change. Each adjusted entry has its roles and its
target; the list is under [Adjusted palette entries](#adjusted-palette-entries).

`Border(surface)` is `BorderColor` of the role's text in both modes. The
shell's panel frames (C7) use `st::shadowFg` instead while material is
inactive.

### Surface roles (C4)

`Ui::Material` in `ui/ui_material.h`:

```cpp
void PaintFill(QPainter &p, const QRect &rect, const QColor &fill);
void PaintSurface(
	QPainter &p,
	const QRect &rect,
	Surface surface,
	const style::color &solid);
void PaintSurface(
	QPainter &p,
	const QPainterPath &shape,
	Surface surface,
	const style::color &solid);
void SetOpaquePaint(not_null<QWidget*> widget, bool opaqueInSolidMode);
void RepaintOnFillsChanged(not_null<QWidget*> widget);
```

`PaintSurface` paints `solid` when `Active()` is false. When active it paints
the cached `Fill(surface)` with `QPainter::CompositionMode_Source`, so the
alpha reaches the window surface instead of blending over a parent.
`SetOpaquePaint` keeps `WA_OpaquePaintEvent` in solid mode and clears it while
material is active, following `ActiveValue()`.

A widget that sits fully inside a region already painted by its parent's role
paints nothing for its background in material mode; plans name these widgets
explicitly.

### Grabs (C5)

While material is active, animation caches hold content only:
`Ui::Material::GrabScope` makes `PaintSurface` skip role fills during a grab,
`Ui::Material::Grab` and `GrabOpaque` wrap `Ui::GrabWidget` / `Ui::GrabOpaque`
with a transparent background, and each animation owner paints its role once
with `PaintAnimationBackground` before drawing the caches. With material off
these functions call the `lib_ui` grabs unchanged.

### Shell geometry (C7)

`Window::SoftPanels`: the pure `Layout` in `window/window_soft_panels_layout.h`
(unit-tested without the app), `PanelFrame` in `window/window_soft_panels.h`:

```cpp
struct LayoutArgs {
	QRect main;        // MainWidget rect, already inset by the outer frame
	int dialogsWidth = 0;
	int thirdWidth = 0;
	bool oneColumn = false;
	int gap = 0;
};

struct Geometry {
	QRect dialogs;      // chats list part of the navigation panel
	QRect conversation; // main section
	QRect third;        // info or tabbed selector column, may be empty
};

[[nodiscard]] Geometry Layout(const LayoutArgs &args);

class PanelFrame; // paints one panel's outside corners and border
```

The navigation panel is the folder rail (a child of the window body, left of
`MainWidget`) plus the chats list (inside `MainWidget`); it touches the
window's left edge and rounds only its right corners. The conversation and
third panels round all corners. `PanelFrame` is a mouse-transparent overlay
that paints the area outside the rounded shape with the Window role and the
thin border on the shape, so panel children need no clipping.

Sizes live in `window/window_soft_panels.style`: `softPanelsFrame` (margins,
from the exploration: 0, 0, 6, 6), `softPanelsGap` (4), `softPanelsRadius`
(14), `softPanelsNavigationRadius` (12) and `softPanelsBorderWidth` (1). They
are style values, so they follow the interface scale.

### Element styling (C9–C15)

- Change existing `.style` values or add new style objects in the owning
  area's `.style` file. Never hardcode sizes.
- Where Soft Panels needs a different value than upstream, add a parallel
  style object (for example `dialogsRowSoftPanels`) and choose between them by
  `SoftPanels()` at the one place the style is passed in. Do not branch inside
  painting code.
- Every background goes through `PaintSurface` with the matching role, or
  paints nothing because a container already does.
- Text and icon colors stay palette keys. Readability is reached by fills and
  by C3's runtime adjustment of existing entries, not by new palette keys, so
  custom and cloud themes work unchanged.

### Adjusted palette entries

While material is active, C3 raises each entry until it reaches its target
over the lowest floor among its roles, composited on black and on white, and
restores it exactly when material turns off. A plan that adds an entry adds
its row here and to C3's table in `window/window_material.cpp`.

| Entries | Roles | Target | Plan |
| --- | --- | --- | --- |
| `windowSubTextFg`, `dialogsTextFg`, `dialogsDateFg`, `dialogsTextFgService`, `windowActiveTextFg`, `historyComposeAreaFgService` | Navigation, Conversation | 4.5:1 | C3 |
| `titleButtonFg`, `titleButtonFgActive`, `titleButtonCloseFg`, `titleButtonCloseFgActive` | Window | 3:1 | C8 |
| `titleFg`, `titleFgActive` (the canary build label) | Window | 4.5:1 | C8 |
| `placeholderFg` | Navigation, Bar | 4.5:1 | C10 |
| `dialogsUnreadBgMuted`, only if the user chooses to adjust the badges | Navigation | 3:1 | C10 |

Service messages, date pills and the unread bar are not palette entries here:
C13 overrides them per chat style, and only while the material chat background
is on.

### Conversation bars and chat background (C11)

`Ui::Material` (`td_ui`) adds `BarPillRect(row)`, `PaintBar(p, row, clip,
solid)` and `PaintBarContent(p, row, solid)`; the sizes are
`softPanelsBarMargins` and `softPanelsBarRadius` in
`window/window_soft_panels.style`. `Window::Material` adds
`ChatBackgroundEnabled()`, `SetChatBackgroundEnabled(bool)`,
`ChatBackgroundActive()` (enabled and `Active()`) and
`ChatBackgroundActiveValue()`.

### Chat colors on the material background (C13)

`Ui::ChatStyle` gains `applyAdjustedServiceSelected(QColor bg)`,
`applyAdjustedUnreadBar(QColor bg, QColor border, QColor fg)` and
`applyScrollColors(not_null<const style::palette*>)`. Each resets the service
corner caches, increments the palette version and fires `paletteChanged()`.
`Window::SessionController::applyMaterialChatColors()` reapplies the theme
with `_chatStyle->apply()`, then the overrides while
`ChatBackgroundActive()`.

### Popup radius (C14)

`[[nodiscard]] int Ui::PopupPanelRadius()` in `ui/chat/chat_style_radius.h`
returns `st::softPanelsPopupRadius` (12) under Soft Panels and
`st::emojiPanRadius` (8) otherwise. It replaces the direct reads of
`st::emojiPanRadius` in transient surfaces; the lock and sign-in cards (C15)
use `softPanelsPopupRadius` directly.

### Toolkit fork hooks

Every `lib_ui` change is one of these hooks. Unset, each one leaves upstream
behavior unchanged.

| Hook | Set by | Plan |
| --- | --- | --- |
| `RpWindow::setTitleBackgroundPainter(Fn<void(QPainter&, const QRect&)>)` | `MainWindow::initHook()` when `Capable()` | C8 |
| `Ui::SetSoftMenus(bool)` / `Ui::SoftMenus()` | startup, from `SoftPanels()` | C14 |
| `Ui::Platform::SetPopupBackdrop(bool)` | the popup option and `ActiveValue()` | C14 |
| `Ui::SetTransparentSettingsRows(bool)` / `Ui::TransparentSettingsRows()` | startup, from `SoftPanels()` | C15 |

### Settings and switches

| Name | Kind and place | Default | Applies | Plan |
| --- | --- | --- | --- | --- |
| `window_material`, "Use window transparency" | KV pref, Advanced > Window title bar | Off | After restart | C2; replaces the Phase 1 option |
| `soft-panels-layout`, "Soft Panels layout" | `base::options`, Experimental | Off | After restart | C7; C15 promotes it to the KV pref `soft_panels_layout`, "Use Soft Panels layout" |
| `window_material_chat_background`, "Show window material behind messages" | KV pref, Chat settings, only when `Capable()` | Off | Live | C11 |
| `popup-material-prototype` | `base::options`, Experimental | Off | After restart | C14; C15 promotes it to the KV pref `popup_material` if the record says go and the user agrees |

## Cross-cutting rules

| Concern | Rule | Checked by |
| --- | --- | --- |
| Behavior | No action, menu, shortcut or state is removed or moved behind a different gesture. | Each plan's behavior checklist |
| Palette changes | Fills, borders and opaque attributes update on `style::PaletteChanged()` and night mode without restart. | C3 unit tests, C16 matrix |
| Runtime fallback | Turning off transparency effects or turning on high contrast switches to solid mode live. | C2 task, C16 matrix |
| Interface scale and DPI | All sizes are style values; panel shapes are anti-aliased paths at device pixel ratio. | C16 screenshots at 100–200% |
| RTL | Geometry uses `setGeometryToLeft` / `myrtlrect` like the code it replaces. | C16 matrix |
| Accessibility | Keyboard focus rings stay visible on every fill; contrast measured on native screenshots. | C16 contrast tool |
| Performance | No live blur inside the app; translucent regions are painted once. | C1 matrix, C7 scroll check |
| GPU children | Video, calls and Mini App surfaces keep opaque fills under them. | C1 matrix, C13 task |
| Rebase hygiene | New logic in new files; existing files get role calls and style object switches only; one area per commit. | Code review |
| Platform guards | Windows code under `platform/win/` or `#ifdef Q_OS_WIN`; other platforms see `Capable() == false` and `SoftPanels() == false`. | Build on Windows; code review for others |
| Comments | Repository comment ration applies to product code; rationale belongs in these documents. | Hooks |

## What a Phase 1 no-go changes

| Phase 1 result | Change |
| --- | --- |
| Go with Acrylic | Plans apply as written. |
| Mica matches the mockup better | C1 sets the Mica backdrop type; C3 floors are re-measured. |
| Backdrop fails with the QRhi backing store | C2 gains a task to create the main window without QRhi; repeat Phase 1 checks. |
| Backdrop impossible | C1 and C2's backdrop tasks are dropped; `Capable()` is always false; C3–C15 still deliver Soft Panels in solid mode. |

## How to use the plans

Each component plan follows the Phase 1 plan's format: header, global
constraints, review focus, then tasks with files, interfaces, steps and a
commit. Plans name exact new files and functions; for existing code they name
functions and style keys, never line numbers. Steps that touch `lib_ui` say to
recheck the named function at the pinned submodule commit first, because this
survey could not read it.

Execute one plan at a time, in the order above, with
superpowers:subagent-driven-development or superpowers:executing-plans.
Before starting a plan, reread its "Depends on" results: a plan written now
may need small corrections from what earlier phases found, and those
corrections are made in the plan before execution.
