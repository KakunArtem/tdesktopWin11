# Telegram Desktop UI modernization

Status: the Soft Panels visual baseline is approved and
[saved in the repository](reference/README.md). An
[implementation architecture and phased plan](implementation-plan.md) is
proposed, with a [component architecture](architecture.md) and
[one plan per component](plans/README.md); product implementation has not
started. Progress and open decisions are tracked in the architecture's
[Progress](architecture.md#progress) section.

## Purpose

Modernize the UI of this fork of the official Telegram Desktop client, with
Windows 11 as the design target.

## Direction from the user

- Use Telegram's macOS client UI elements as visual inspiration.
- Use Windows 11-style translucency in place of Liquid Glass.
- Use the Soft Panels concept, with thinner borders and translucent contact
  and conversation panels. This revises the earlier opaque-canvas preference.
- Keep message content and inputs readable against the material.
- Use the user's existing Windhawk theme setup as a visual reference for the
  desired Windows appearance.
- Use the [readable Soft Panels mockup](explorations/README.md) as the
  appearance target, rather than choosing between Acrylic and Mica first.
  Acrylic is the working guideline for now (2026-10-28).
- Focus on Windows for now: the Soft Panels layout and material are judged and
  built for Windows, and macOS and Linux keep the upstream look (2026-10-30).
- Platform and setup (2026-10-31): material needs Windows 11 22H2 (build
  22621) or later; it is refused while the system window frame is on; `lib_ui`
  and `lib_base` are forked as `KakunArtem/lib_ui` and `KakunArtem/lib_base`,
  branch `soft-panels`; `lib_ui`'s default styles stay upstream, and Soft
  Panels uses parallel app styles and fork flags.
- Look (2026-10-31): under Soft Panels, Default font means the system font
  (Segoe UI); Telegram's notifications and separate panels take the popup
  shape (radius 12, the Popup border, opaque); the day theme rail becomes a
  light rail with readable labels and the mockup's light selected pill.
- Mockup values adopted (2026-10-31): rail width 64 px, chat header height
  56 px, bubble radius 14 px, and a light selected chat row with normal text
  colors instead of the accent-filled row.
- Kept as Telegram has them (2026-10-31): night theme unread badge fills,
  text fields with a bottom line, filled buttons 34 px high, and today's field
  bottom lines and slider tracks.

The approved reference remains the earlier baseline; the readable mockup
refines it. Detailed element states, the final native material and
implementation details remain under discussion.

## Supplied reference

[Translucent Explorer11 - Windows 11 File Explorer styling guide](https://github.com/ramensoftware/windows-11-file-explorer-styling-guide/blob/main/Themes/Translucent%20Explorer11/README.md)

The user supplied this link as an example of their Windhawk theme setup. The
initial review is recorded in [the research brief](research-translucent-explorer11.md).
Windhawk is a visual reference; no new runtime dependency has been selected.

## Current scope

Refine the selected concept into visual elements, starting with translucent
panels, contacts/search, conversation chrome, messages and the composer. The
user explicitly chose visual refinement before changes to the Qt client.
The user then requested a general implementation plan, to be split into
smaller plans later. Product source changes and builds remain for later
requests.

## Decisions for later

- Which macOS client screens and elements should guide the redesign.
- Which Telegram surfaces should change first. A proposed order is in
  [the design directions](design-directions.md).
- The final native Windows material, tint strength and technical
  implementation. The mockup is the target and Acrylic the working guideline.
- Minimum text contrast and material fill behind text; the reference falls
  short in places, as the [readability check](design-directions.md#readability-check-of-the-approved-reference)
  records.
- Chat wallpaper and custom theme behavior on the translucent conversation
  panel.
- Behavior when translucency is unavailable or disabled (the minimum build
  is decided above).

## Working documents

This brief is the entry point for the redesign documentation. Current documents:

- [Translucent Explorer11 research and library assessment](research-translucent-explorer11.md).
- [Proposed UI element map and redesign sequence](element-map.md).
- [Visual concept alternatives](design-directions.md).
- [Approved Soft Panels reference, preview and screenshots](reference/README.md).
- [Readable Soft Panels exploration](explorations/README.md), a proposal that
  addresses the readability check and behavior gaps.
- [Implementation architecture and phased plan](implementation-plan.md).
- [Component architecture](architecture.md): the 16 components, their
  contracts and dependencies, expanding the roadmap. It is the main document
  for tracking progress and open decisions.
- [Component plans](plans/README.md): one implementation plan per component,
  in execution order.
- [Component visual references](component-references/README.md): what each
  component changes, isolated from the mockup, for every planned component.
- [Shared controls, foundations and separate windows](shared-elements.md):
  the elements no plan covered, grouped as C17 to C19, each with a plan.
- [Windows runbook](windows-runbook.md): the first steps on the Windows
  machine, from the toolchain to Phase 1.
- [Plan preflight](preflight.md): the plans checked against the pinned
  sources before implementation.
- [Toolkit fork](toolkit-fork.md): the `lib_ui` and `lib_base` forks, the
  branch rule and how to land changes and take upstream updates.

The translucency scope above is confirmed. Other recommendations in these
documents remain proposals unless explicitly recorded as user decisions.
Add focused documents as future work produces designs or decisions.

Agent instructions for this directory are in [AGENTS.md](AGENTS.md).
