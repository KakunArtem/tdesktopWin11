# UI modernization documentation guide

This directory holds the fork's UI direction and subsequent design documents.
Repository-wide instructions in the root `AGENTS.md` still apply.

## Start here

Read [README.md](README.md) before changing these documents or planning UI work.
It records the user's direction and the current scope.

The approved visual baseline is saved in [reference/README.md](reference/README.md).
Read it before refining elements. Keep later experiments distinct from the
approved snapshot.

## Current phase

- The user has requested reference research, a preliminary library assessment
  and a high-level breakdown of the interface into elements, followed by design
  ideas and visual examples.
- Read supplied references and relevant project code. Cite findings, and label
  proposed visual treatments and technical approaches as proposals.
- Visual concept mockups are now in scope.
- Implementation planning is in scope: the general plan in
  `implementation-plan.md`, then smaller per-phase plans. Keep product source
  changes and builds for later requests.

## Maintaining the documents

- Keep confirmed user decisions separate from proposals and open decisions.
- Treat macOS UI elements and the supplied Windhawk theme as inspiration;
  neither establishes a rendering technology or runtime dependency.
- When research is requested, cite sources and distinguish verified findings
  from assumptions. Do not invent platform support or compatibility claims.
- Add focused documents only as needed and link them from `README.md`.
- Use relative links within the repository and preserve CRLF line endings
  with UTF-8 encoding without a BOM in this native Windows checkout.
- For documentation-only changes, check links, formatting and the diff; a
  Telegram build is unnecessary.
