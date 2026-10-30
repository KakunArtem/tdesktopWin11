# Windows 11 Soft Panels Redesign Implementation Plan

> **For agentic workers:** This is a roadmap of separate plans, one per phase. Execute one phase plan at a time with superpowers:subagent-driven-development (recommended) or superpowers:executing-plans. Phase plans use checkbox (`- [ ]`) steps.

**Goal:** Bring the Soft Panels appearance to Telegram Desktop on Windows 11 (translucent window, readable panels, restyled elements) without changing Telegram's behavior, with a solid fallback wherever native material is unavailable.

**Architecture:** Five independent layers: a native DWM backdrop, a material state service that derives readable fills from the active palette, surface roles that make each region paint its background once, a Soft Panels shell layout, and per-element styling. Each layer can be switched off. New logic lives in new files; upstream files get small, local edits.

**Tech Stack:** C++20, Qt 6.11 Widgets with the Desktop App Toolkit (`lib_ui`, `lib_base`), Telegram `.style` and palette files, Win32 DWM, CMake with Visual Studio, PowerShell for screen-level checks.

Date: 2026-10-28. Status: proposal for review. Inputs: the
[approved reference](reference/README.md) and the
[readable exploration](explorations/README.md), which the user chose as the
appearance target. Acrylic is the working guideline for the native material;
native work aims to match the mockup, not a particular material.

---

## Scope check

The work spans independent subsystems, so it is split into one plan per
phase. Each phase plan produces working, testable software and leaves the
app usable. A phase plan is written just before the phase starts, using what
the previous phase established.

| Phase | Plan |
| --- | --- |
| 1. Native backdrop prototype | [2026-10-28-native-backdrop-prototype.md](plans/2026-10-28-native-backdrop-prototype.md) |
| 2–9 | One plan per component, listed with their phases in the [component plans index](plans/README.md) and specified by the [component architecture](architecture.md) (2026-10-29). Each is rechecked against the results of the phases before it when it starts. |

## Verified codebase facts

Paths are relative to `Telegram/`.

| Area | Fact | Consequence |
| --- | --- | --- |
| Toolkit ownership | `lib_ui` and `lib_base` are submodules pointing at upstream `desktop-app` ([.gitmodules](../../.gitmodules)). | Changes there need forks of those repositories. |
| Window frame | [ui_window_win.cpp](../../Telegram/lib_ui/ui/platform/win/ui_window_win.cpp) draws a custom title, handles `WM_NCCALCSIZE`, rounds corners and sets caption colors. The window is not frameless, and its handle is created after construction. | A translucent flag set in the constructor takes effect; the custom frame stays. |
| App window | [main_window_win.cpp](../../Telegram/SourceFiles/platform/win/main_window_win.cpp) owns the window handle and already calls `DwmSetWindowAttribute`. | Native backdrop work starts app-side. |
| Rendering | [RpWindow](../../Telegram/lib_ui/ui/widgets/rp_window.cpp) primes a QRhi backing store via `EnsureWindowRhi` ([rhi_surface.cpp](../../Telegram/lib_ui/ui/rhi/rhi_surface.cpp)). `RpWindow(bool translucent)` already sets `WA_TranslucentBackground`. | Phase 1 tests whether DWM translucency works through this path. |
| Surfaces | The chat list fills `st::dialogsBg` ([dialogs_widget.cpp](../../Telegram/SourceFiles/dialogs/dialogs_widget.cpp)), the header fills `st::topBarBg` ([history_view_top_bar_widget.cpp](../../Telegram/SourceFiles/history/view/history_view_top_bar_widget.cpp)), and chat sections use `SectionWidget::PaintBackground` ([section_widget.cpp](../../Telegram/SourceFiles/window/section_widget.cpp)). | Opaque fills are spread over many paint sites. |
| Palette | [colors.palette](../../Telegram/lib_ui/ui/colors.palette) supports alpha; `style::PaletteChanged()` reports theme changes. | Fills can be derived from the active palette at runtime. |
| Animations | Section transitions grab widgets; `Ui::GrabWidget` takes a background color ([ui_utility.cpp](../../Telegram/lib_ui/ui/ui_utility.cpp)). The window, history and dialogs areas have 27 grab call sites (33 call lines with `mainwidget.cpp`, per the [architecture survey](architecture.md#verified-source-facts)). | Grabs need a transparent background in material mode. |
| Settings | `Core::Settings` uses append-only serialization and KV prefs; the native frame toggle is in [settings_advanced.cpp](../../Telegram/SourceFiles/settings/sections/settings_advanced.cpp). Experimental toggles use `base::options`. | New settings use KV prefs; prototypes use options. |
| Navigation | [FiltersMenu](../../Telegram/SourceFiles/window/window_filters_menu.h) already has the main menu button and unread counters. | The rail is restyled, not rebuilt. |
| Composer | [ComposeControls](../../Telegram/SourceFiles/history/view/controls/history_view_compose_controls.h) and [HistoryWidget](../../Telegram/SourceFiles/history/history_widget.h) each implement a composer. | Composer work covers both. |
| Tests | Standalone test executables follow `cmake/test_update_unpack.cmake` with `tests/test_*.cpp`, built when `DESKTOP_APP_TEST_APPS` or a focused option is on. | Pure logic gets unit tests; native rendering gets screen-level checks. |

The mockup gaps (main menu entry, folder counts, muted badges, send and
record button) already exist in Telegram and must be preserved.

## Architecture

1. **Native backdrop** (`platform/win/window_material_win.*`): applies the
   transient (Acrylic) system backdrop, extends the frame into the client
   area and follows dark mode. It reports success so the app can fall back.
2. **Material state** (`window/window_material_policy.*`,
   `window/window_material.*`, `window/window_material_fills.*`): the policy
   decides from the environment (Windows build 22621+, transparency effects,
   high contrast, native frame, user setting). The service exposes whether
   material is enabled as an `rpl` producer. The fill calculator picks, per
   surface, the lowest fill at which the palette's text and secondary text
   reach 4.5:1 against both a black and a white backdrop, so custom themes
   stay readable without new keys.
3. **Surface roles** (`ui/ui_material.*` in `td_ui`): window, navigation panel,
   conversation panel and content. A helper used at each existing fill site
   paints the role's fill, or nothing when a parent already painted it. In
   solid mode it paints today's opaque color, so behavior with material off is
   unchanged.
4. **Shell layout** (`window/window_soft_panels.*`, `mainwidget.cpp`): outer
   frame, gaps, rounded panel containers that paint their own anti-aliased
   shape, in one-, two- and three-column layouts. Applies in solid mode too.
5. **Element styling** (`.style` files and owning widgets): rows, search,
   header and bars, floating composer, bubbles, menus and boxes, keeping every
   Telegram state.

Chat wallpapers keep working and fill the conversation panel below the
header. A "Window material" chat background option paints the canvas fill
instead.

## File structure

| Path | Phase | Responsibility |
| --- | --- | --- |
| `Telegram/SourceFiles/window/window_material_policy.{h,cpp}` | 1 | Backdrop decision from the environment; prototype fill. |
| `Telegram/SourceFiles/platform/win/window_material_win.{h,cpp}` | 1 | Environment reader and DWM backdrop. |
| `Telegram/SourceFiles/tests/test_window_material.cpp`, `Telegram/cmake/test_window_material.cmake` | 1, extended in 2 | Unit tests for the policy and fill calculator. |
| `docs/ui-modernization/tools/Test-Backdrop.ps1` | 1 | Screen-level transparency check. |
| `Telegram/SourceFiles/window/window_material_fills.{h,cpp}` | 2 | Contrast-driven fill calculation from palette colors. |
| `Telegram/SourceFiles/window/window_material.{h,cpp}` | 2 | Material state service. |
| `Telegram/SourceFiles/ui/ui_material.{h,cpp}` | 2 | Paint-time state and the surface role painting helper, in `td_ui` ([why](architecture.md#two-namespaces-two-targets)). |
| `Telegram/SourceFiles/settings/sections/settings_advanced.cpp` | 2 | "Window transparency" setting next to the native frame option. |
| `Telegram/SourceFiles/window/window_soft_panels.{h,cpp}` | 3 | Rounded panel containers and frame geometry. |
| `Telegram/SourceFiles/mainwidget.cpp` | 3 | Panel insets and gaps in `MainWidget::updateControlsGeometry`. |
| `Telegram/lib_ui/ui/platform/win/ui_window_title_win.cpp` (forked `lib_ui`) | 3 | Transparent title area. |
| `Telegram/SourceFiles/dialogs/`, `Telegram/SourceFiles/window/window_filters_menu.cpp` | 4 | Chat list, search and folder rail. |
| `Telegram/SourceFiles/history/view/history_view_top_bar_widget.cpp`, `Telegram/SourceFiles/ui/chat/` bars, `Telegram/SourceFiles/window/section_widget.cpp` | 5 | Header, bars and chat background option. |
| `Telegram/SourceFiles/history/view/controls/`, `Telegram/SourceFiles/history/history_widget.cpp` | 6 | Floating composer in both implementations. |
| `Telegram/SourceFiles/ui/chat/chat.style`, `Telegram/SourceFiles/history/view/` | 7 | Bubbles, service messages, reactions, attachments. |
| `Telegram/lib_ui/ui/widgets/` (forked), `Telegram/SourceFiles/chat_helpers/`, `Telegram/SourceFiles/boxes/` | 8 | Menus, tooltips, pickers, boxes, optional popup Acrylic. |
| `Telegram/SourceFiles/info/`, `Telegram/SourceFiles/settings/` | 9 | Info pane, settings and specialized screens. |

## Phases

### Phase 1: Native backdrop prototype (go/no-go)

- **Produces:** Acrylic behind one translucent strip, behind an experimental
  option, plus a decision record.
- **Tests:** policy unit tests; `Test-Backdrop.ps1` opaque and transparent
  checks; a manual matrix of window states.
- **Plan:** [native backdrop prototype](plans/2026-10-28-native-backdrop-prototype.md).

### Phase 2: Material foundation

- **Depends on:** a go or conditional go from Phase 1.
- **Produces:** the material service, a "Window transparency" setting
  (KV pref) replacing the prototype option, palette-derived fills, the surface
  role helper, transparent grabs at every grab site (audited by file, see
  [C5](plans/2026-10-29-transparent-grabs.md)), and removal of the
  prototype strip. With the setting on, only areas that no longer paint show
  material.
- **Tests:** fill calculator unit tests using the exploration's colors as
  vectors; policy tests extended for the setting; opaque check with the
  setting off.

### Phase 3: Shell layout

- **Depends on:** Phase 2 and a `lib_ui` fork.
- **Produces:** Soft Panels frame, navigation and conversation panels in all
  adaptive layouts; transparent title area with existing caption buttons.
  Material becomes usable for daily testing.
- **Tests:** screenshots in each adaptive layout and window state, solid and
  material; `Test-Backdrop.ps1` transparent check across the panels.

### Phase 4: Navigation and chat list

- **Depends on:** Phase 3.
- **Produces:** folder rail, rows, search, archive and stories strip styles.
- **Tests:** contrast measured on native screenshots; hover, selected, unread
  and muted states; keyboard navigation.

### Phase 5: Conversation chrome

- **Depends on:** Phase 3.
- **Produces:** header, pinned and contextual bars, and the "Window material"
  chat background option.
- **Tests:** each bar type shown and dismissed; wallpaper and material
  backgrounds; contrast on native screenshots.

### Phase 6: Composer

- **Depends on:** Phase 3.
- **Produces:** floating rounded composer in `ComposeControls` and
  `HistoryWidget`, with reply, edit, forward, previews, autocomplete,
  recording and drag-and-drop states.
- **Tests:** every composer state in ordinary chats, replies, topics and
  scheduled views.

### Phase 7: Messages and attachments

- **Depends on:** Phase 3 and Phase 5's "Window material" chat background.
- **Produces:** bubbles, service messages, date pills, reactions and
  attachment cards across all mirrored chat sections.
- **Tests:** screenshots of each message type in light, dark and solid
  appearances; selection and reactions still work.

### Phase 8: Transient surfaces

- **Depends on:** Phases 4–7.
- **Produces:** menus, tooltips, emoji and sticker panels, boxes and toasts,
  optionally with Acrylic popups.
- **Tests:** open, keyboard-navigate and dismiss each surface; popup backdrop
  check.

### Phase 9: Info, settings and specialized screens

- **Depends on:** Phase 8.
- **Produces:** info pane, settings and remaining screens; final polish and
  accessibility pass.
- **Tests:** the full verification matrix below.

Phases 1–3 are sequential; 4–6 can run in parallel; 7 follows 5; 8–9 follow.

## Verification matrix for every phase

- Light, dark and solid appearances on the four test wallpapers from the
  exploration.
- Interface scale 100%, 125%, 150% and 200%, and moving between monitors with
  different DPI.
- Normal, maximized, snapped, full-screen and inactive windows.
- Windows transparency effects off, high contrast on, and Windows 10 or older
  Windows 11 builds showing the solid fallback.
- A custom theme and a cloud theme; right-to-left layout; keyboard focus.
- Contrast measured on native screenshots with the exploration's method.
- Behavior unchanged: the same actions, menus and states as before.

## Risks

| Risk | Handling |
| --- | --- |
| A translucent window with this backing store may not combine with a DWM backdrop; DWM ignores backdrops on layered windows. Unverified for this app. | Phase 1 go/no-go. Alternatives: disable QRhi for the main window, a different window configuration, or Mica only. |
| Inactive windows may lose the backdrop. | Measured in Phase 1; inactive appearance designed from the result. |
| Translucent text may lose ClearType. | Compared in Phase 1; the user decides whether it is acceptable. |
| Repaint cost while scrolling. | Profiled in Phases 1 and 3. |
| GPU child surfaces (video, calls) over a translucent parent. | Tested in Phase 1. |
| Opaque flashes from grabs and slide animations. | Grab audit in Phase 2. |
| Rebase conflicts from many paint-site edits. | One-line helper calls, new files, one area per commit. |
| Submodule changes have no home. | Fork `lib_ui` (and `lib_base` if needed) before Phase 3. |

## Decisions

Made on 2026-10-28:

- The readable mockup is the appearance target.
- Acrylic is the working guideline for the native material. Mica stays an
  alternative if Acrylic cannot match the mockup.

Still needed (tracked, with the plan each one blocks, in
[Open decisions](architecture.md#open-decisions)):

1. Minimum Windows build for material; proposed: Windows 11 22H2 (22621).
2. Fork `lib_ui` and `lib_base` under your account before Phase 3.
3. Whether transparency is on by default, and the default chat background in
   material mode.
4. Whether the Soft Panels layout applies on macOS and Linux or stays
   Windows-only.
5. Whether material is disabled when the native window frame is on. Phase 1
   assumes yes.

## Execution handoff

After each phase plan is approved, choose how to run it:

1. **Subagent-driven (recommended):** a fresh subagent per task, with review
   between tasks (superpowers:subagent-driven-development).
2. **Inline:** tasks run in one session with checkpoints
   (superpowers:executing-plans).

Both skills come from the Superpowers plugin, which must be enabled first. If
requested, phase plans can also become tasks in the AI queue.
