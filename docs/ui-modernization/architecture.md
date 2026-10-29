# Soft Panels redesign architecture

Date: 2026-10-29. Status: proposal for review. This document expands the
[implementation roadmap](implementation-plan.md) into the complete list of
components to build, the contracts between them and one plan per component.
It changes no confirmed user decision; proposals and assumptions are labeled.

Inputs: the [project brief](README.md), the
[readable Soft Panels exploration](explorations/README.md) (the appearance
target), the [approved reference](reference/README.md), the
[element map](element-map.md), the [design directions](design-directions.md)
and the [native backdrop prototype plan](plans/2026-10-28-native-backdrop-prototype.md).

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
| C7 | Shell layout | Soft Panels switch, outer frame, gaps and rounded panel containers in one-, two- and three-column layouts. | `window/window_soft_panels.*`, `mainwidget.cpp`, `window/main_window.cpp` | C4 | 3 | [shell layout](plans/2026-10-29-shell-layout.md) |
| C8 | Title bar | Transparent title area with the existing caption buttons, drag regions and inactive state. | forked `lib_ui/ui/platform/win/`, `platform/win/main_window_win.cpp` | C6, C7 | 3 | [title bar](plans/2026-10-29-title-bar.md) |
| C9 | Navigation rail | Folder rail and main menu entry on the shared navigation panel. | `window/window_filters_menu.*`, `window/window.style`, `window/window_main_menu.*` | C7 | 4 | [navigation rail](plans/2026-10-29-navigation-rail.md) |
| C10 | Chat list and search | Search field, rows, states, archive, topics and stories strip. | `dialogs/`, `dialogs/dialogs.style` | C7 | 4 | [chat list](plans/2026-10-29-chat-list.md) |
| C11 | Conversation chrome | Header, pinned and contextual bars, "Window material" chat background. | `history/view/history_view_top_bar_widget.*`, `ui/chat/*_bar.*`, `window/section_widget.cpp` | C7 | 5 | [conversation chrome](plans/2026-10-29-conversation-chrome.md) |
| C12 | Composer | Floating rounded composer in both implementations with every state. | `history/view/controls/`, `history/history_widget.cpp`, `chat_helpers/chat_helpers.style` | C7 | 6 | [composer](plans/2026-10-29-composer.md) |
| C13 | Messages | Bubbles, service messages, date pills, reactions and attachment cards. | `ui/chat/chat.style`, `history/view/`, `history/view/media/` | C7 | 7 | [messages](plans/2026-10-29-messages.md) |
| C14 | Transient surfaces | Menus, tooltips, pickers, boxes and toasts; optional popup Acrylic. | forked `lib_ui/ui/widgets/`, `lib_ui/ui/layers/`, `chat_helpers/`, `boxes/` | C6, C9–C13 | 8 | [transient surfaces](plans/2026-10-29-transient-surfaces.md) |
| C15 | Info, settings and specialized screens | Info pane, settings, lock and intro screens, final accessibility pass. | `info/`, `settings/`, `intro/`, `window/window_lock_widgets.*` | C14 | 9 | [info and settings](plans/2026-10-29-info-settings.md) |
| C16 | Verification tooling | Backdrop, contrast and screenshot checks reused by every phase. | `docs/ui-modernization/tools/`, `tests/` | C1 | 1–9 | [verification tooling](plans/2026-10-29-verification-tooling.md) |

Order: C1 → C2 → C3 → C4 → C5; C6 any time before C8; C7 → C8; then C9–C13
in parallel; C14; C15. C16 grows alongside: its contrast tool is needed by C3
and its screenshot matrix by C7.

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
| `bool SoftPanels()` / `rpl::producer<bool> SoftPanelsValue()` | The Soft Panels layout switch (C7); `false` on platforms other than Windows. |
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
on every `ActiveValue()` change.

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

`Window::SoftPanels` in `window/window_soft_panels.h`:

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
