# Translucent Explorer11: initial research

Reviewed: 2026-10-28. Scope: the supplied theme, its screenshot, Telegram's UI
structure, and a preliminary technical shortlist. This is a research note;
no rendering approach has been tested or selected.

## What the reference actually does

The [theme README and configuration](https://github.com/ramensoftware/windows-11-file-explorer-styling-guide/blob/main/Themes/Translucent%20Explorer11/README.md)
describe a Windhawk theme targeting File Explorer's WinUI elements. Acrylic is
the default background effect. The rules make navigation, command, gallery and
details backgrounds transparent, retain a subtle command-bar divider, tint
the selected tab, and give tooltips an Acrylic brush.

The [supplied screenshot](https://github.com/ramensoftware/windows-11-file-explorer-styling-guide/blob/main/Themes/Translucent%20Explorer11/screenshot.png)
shows the title, navigation and command bars. It is a cropped chrome reference,
not a demonstration of a complete content layout.

The useful visual idea is a continuous material behind related controls, with
gentle separation and clear interaction states. Specific Telegram layouts,
floating controls and message bubbles below are proposals, not theme findings.

## How this maps to Telegram

Telegram's core interface uses Qt Widgets and its custom Desktop App Toolkit,
not Explorer's WinUI control tree. See the [source atlas](../../Telegram/SourceFiles/AGENTS.md),
[RpWidget](../../Telegram/lib_ui/ui/rp_widget.h) and
[style generation](../../Telegram/cmake/td_ui.cmake). Explorer's selectors and
AcrylicBrush declarations cannot directly style these controls.

There is already a [Windows window helper](../../Telegram/lib_ui/ui/platform/win/ui_window_win.cpp)
covering native hit testing, system buttons, corners and frame colors. The
[chat list](../../Telegram/SourceFiles/dialogs/dialogs_widget.cpp) and
[chat header](../../Telegram/SourceFiles/history/view/history_view_top_bar_widget.cpp)
paint their own backgrounds. This suggests extending the existing toolkit and
auditing surface painting, rather than replacing the client UI framework.

Three effects must remain distinct:

- A Windows window backdrop shows material behind the top-level window.
- Transparent or tinted child surfaces can reveal that common backdrop when
  the complete rendering path allows it.
- Blurring messages beneath an overlapping header or composer is an in-app
  rendering effect. A desktop backdrop alone does not provide it.

[Microsoft's Acrylic guidance](https://learn.microsoft.com/en-us/windows/apps/design/style/acrylic)
distinguishes desktop and in-app Acrylic. It favors desktop Acrylic for
transient UI and warns against large or adjacent Acrylic surfaces. The
Explorer theme is therefore a stronger transparency treatment than the normal
Fluent baseline; following its appearance is a deliberate visual proposal.

## Preliminary library and API assessment

| Candidate | Fit and limitations | Proposed role |
| --- | --- | --- |
| Existing Qt Widgets, lib_ui and Telegram styles | Already own controls, text, scaling, themes and window behavior. Custom-painted regions need individual review. | Keep as the foundation. |
| Windows DWM system backdrop APIs | The documented backdrop enum distinguishes Mica and Desktop Acrylic, and requires Windows 11 build 22621 or later. It does not restyle Qt controls. | First native route to evaluate through the existing Windows window layer. |
| QWindowKit | Supports Qt Widgets and Quick, custom frames, system buttons and Windows material attributes. Its README notes reliance on Qt private APIs. It overlaps existing window helpers. | Optional candidate if a concrete window-integration gap remains; no adoption proposed yet. |
| Qt Quick Controls FluentWinUI3 | A Qt Quick Controls style, not a style for Telegram's custom QWidget controls. Qt documents it since Qt 6.8. | A reference or future migration option; adds a separate UI architecture to this redesign. |
| Windows App SDK / WinUI materials | Offers backdrop controllers and XAML integrations; these are not automatic integrations with Telegram's widget hierarchy. | Revisit only if the existing Qt/native route proves insufficient. |

Primary references: [DWM backdrop enum](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwm_systembackdrop_type),
[QWindowKit README](https://github.com/stdware/qwindowkit),
[Qt FluentWinUI3](https://doc.qt.io/qt-6/qtquickcontrols-fluentwinui3.html),
[Windows system backdrop integration](https://learn.microsoft.com/en-us/windows/apps/develop/ui/system-backdrops).

Qt also documents [translucent widget requirements](https://doc.qt.io/qt-6.8/qwidget.html#creating-translucent-windows).
Setting a transparency flag alone is not proof that modern DWM Acrylic works
correctly with this application's backing store, native frame and media
surfaces. That compatibility needs a later, explicitly requested prototype.

## Material approaches considered

1. **Explorer-inspired selective Acrylic:** a common translucent shell and
   navigation surface, with a stable message canvas and readable content fills.
   This was the initial recommendation because it retains the supplied
   reference's character while protecting long reading sessions.
2. **Conservative Fluent:** Mica for the long-lived shell, Acrylic for popups,
   and solid content surfaces. This follows Microsoft's general material
   guidance more closely but gives a less transparent appearance.
3. **Broad Acrylic:** extend desktop translucency into the conversation canvas.
   This is closer to an all-translucent interpretation, with greater variation
   in message contrast and more rendering behavior to validate.

The user initially selected chrome and navigation translucency with a calm
message canvas, then revised that scope after reviewing the visual examples.
The current direction is Soft Panels with thinner borders and translucent
contact and conversation panels. Acrylic versus Mica, tint strength and the
technical implementation remain undecided.

The approved reference is therefore closest to approach 3. Its message and
composer fills stay dense, but list, header and metadata text sits directly
on the material; see the
[readability check](design-directions.md#readability-check-of-the-approved-reference).

## Questions reserved for the next phase

- Acrylic versus Mica and tint strength for chrome, contacts and conversation
  surfaces, plus their relationship to chat wallpapers.
- Which macOS Telegram client screens and elements should guide the shapes,
  spacing and grouping. No macOS layout study has been done in this pass.
- Supported Windows versions, opaque fallback behavior and custom-theme policy.
- Feasibility and cost of live in-app blur if an element later needs it.

The [element map](element-map.md) separates that later work into UI families.
