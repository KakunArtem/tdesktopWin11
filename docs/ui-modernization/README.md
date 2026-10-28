# Telegram Desktop UI modernization

Status: the Soft Panels visual baseline is approved and
[saved in the repository](reference/README.md). Product implementation has not
started.

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

Use the saved reference as the appearance baseline. Detailed element states,
native materials and implementation details remain under discussion.

## Supplied reference

[Translucent Explorer11 - Windows 11 File Explorer styling guide](https://github.com/ramensoftware/windows-11-file-explorer-styling-guide/blob/main/Themes/Translucent%20Explorer11/README.md)

The user supplied this link as an example of their Windhawk theme setup. The
initial review is recorded in [the research brief](research-translucent-explorer11.md).
Windhawk is a visual reference; no new runtime dependency has been selected.

## Current scope

Refine the selected concept into visual elements, starting with translucent
panels, contacts/search, conversation chrome, messages and the composer. The
user explicitly chose visual refinement before changes to the Qt client.
Implementation plans, product source changes and builds remain for later
requests.

## Decisions for later

- Which macOS client screens and elements should guide the redesign.
- Which Telegram surfaces should change first. A proposed order is in
  [the design directions](design-directions.md).
- The native Windows material (Acrylic or Mica), tint strength and technical
  implementation. The overall appearance is approved.
- Minimum text contrast and material fill behind text; the reference falls
  short in places, as the [readability check](design-directions.md#readability-check-of-the-approved-reference)
  records.
- Chat wallpaper and custom theme behavior on the translucent conversation
  panel.
- Supported Windows versions and behavior when translucency is unavailable or
  disabled.

## Working documents

This brief is the entry point for the redesign documentation. Current documents:

- [Translucent Explorer11 research and library assessment](research-translucent-explorer11.md).
- [Proposed UI element map and redesign sequence](element-map.md).
- [Visual concept alternatives](design-directions.md).
- [Approved Soft Panels reference, preview and screenshots](reference/README.md).
- [Readable Soft Panels exploration](explorations/README.md), a proposal that
  addresses the readability check and behavior gaps.

The translucency scope above is confirmed. Other recommendations in these
documents remain proposals unless explicitly recorded as user decisions.
Add focused documents as future work produces designs or decisions.

Agent instructions for this directory are in [AGENTS.md](AGENTS.md).
