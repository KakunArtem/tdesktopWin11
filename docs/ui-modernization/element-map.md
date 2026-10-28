# Proposed UI element map

Date: 2026-10-28. This is a high-level breakdown, not an implementation plan.
The window appearance baseline is now [approved](reference/README.md), while
detailed element states remain to be refined. This breakdown builds on the
[reference research](research-translucent-explorer11.md).

The user selected Soft Panels and then requested thinner borders and
translucency in both contact and conversation panels. This supersedes the
earlier opaque-canvas preference. Exact materials and element treatments
remain under refinement.

## Proposed surface hierarchy

| Surface | Initial proposal |
| --- | --- |
| Window shell and navigation | One coordinated Windows material with transparent or gently tinted child backgrounds. |
| Contact panel | Let the common material show through the list; retain clear selected, hover and unread states. |
| Conversation canvas | Translucent panel following the revised direction; retain readable message surfaces and resolve chat-wallpaper behavior later. |
| Content and controls | Legible fills for messages and inputs; full-strength text, icons, focus and selection. |
| Menus, pickers and dialogs | Consistent elevated surfaces. Decide separately which are real popup windows and which are in-app layers. |
| Fallback appearance | Equivalent solid surfaces when effects are unavailable or disabled. |

Material opacity belongs to surfaces; reducing the opacity of an entire window
would also fade its text and controls. Independent blur on every element is
not the proposed design model.

## Elements to handle

Paths below are relative to `Telegram/`; ownership follows the
[source atlas](../../Telegram/SourceFiles/AGENTS.md).

| Family | Elements to redesign together | Main source areas |
| --- | --- | --- |
| Visual foundations | Surface roles, tint, light/dark palettes, typography roles, spacing, radii, borders, shadows, icon treatment and motion. Reuse the existing style and scale system. | `SourceFiles/ui/`, `SourceFiles/window/themes/`, `lib_ui/ui/style/` |
| Window shell | Title bar, caption buttons, drag regions, outer border, corners, resize, maximized/fullscreen and inactive states. | `SourceFiles/window/`, `SourceFiles/platform/win/`, `lib_ui/ui/platform/win/` |
| Shared controls | Icon and text buttons, text/search fields, toggles, checkboxes, radio buttons, tabs, sliders, scrollbars, progress and badges. | `lib_ui/ui/widgets/`, `SourceFiles/ui/controls/` |
| Navigation and accounts | Account switcher, folder rail or tabs, archive, navigation badges, main menu and collapsed navigation. | `SourceFiles/window/`, `SourceFiles/ui/widgets/` |
| Chat list and search | Search field/results, chat rows, avatars, names, previews, timestamps, unread/mute/pin states, topics and stories strip. | `SourceFiles/dialogs/` |
| Conversation header and bars | Chat title/status, back/search/call/menu actions, pinned messages, topic tabs, translation and other contextual bars. | `SourceFiles/history/view/`, `SourceFiles/ui/chat/` |
| Messages and conversation canvas | Incoming/outgoing/service messages, grouping, text, metadata, replies, forwards, reactions, selection, unread/date markers and jump controls. | `SourceFiles/history/view/`, `SourceFiles/ui/chat/` |
| Message attachments | Photos/albums, video, voice/audio, documents, links, polls, stickers and special message cards. | `SourceFiles/history/view/media/` |
| Composer | Message field, attach/emoji/send/record actions, reply/edit/forward strips, link previews, autocomplete, recording and drag-and-drop states. | `SourceFiles/history/view/controls/`, `SourceFiles/chat_helpers/` |
| Transient surfaces | Context/send menus, tooltips, toasts, emoji/sticker/GIF pickers, reaction selector, confirmations and share/send-file dialogs. | `lib_ui/ui/widgets/`, `lib_ui/ui/layers/`, `SourceFiles/menu/`, `SourceFiles/boxes/`, `SourceFiles/chat_helpers/` |
| Info and settings | Profile/details pane, shared-media tabs, settings navigation, section groups, rows and forms. | `SourceFiles/info/`, `SourceFiles/settings/` |
| Secondary and specialized screens | Sign-in and locks, media viewer/stories, audio player, calls, notifications, editors, Mini Apps, payments and other specialist flows. Reuse foundations, then review scene-specific controls. | Relevant feature areas in the source atlas |

## Proposed sequence

1. **Refine materials and foundations.** Use the selected Soft Panels direction
   across chrome, contacts and conversation, define surface and interaction
   roles, and establish a solid fallback.
2. **Design the main shell and reusable controls.** Settle the frame, navigation
   structure and control vocabulary before repeating them across screens.
3. **Redesign the everyday chat experience.** Work through chat list, header,
   composer, message presentation and attachments as separate element groups.
4. **Extend the vocabulary.** Apply it to transient surfaces, info, settings
   and then specialized scenes.

Before implementation, a small native-material feasibility prototype should
confirm that the chosen appearance works with the current Qt rendering path.
It has not been requested or built in this phase.

## What each element review should settle later

- Visual treatment, information priority, density and relationship to nearby
  surfaces, using a specific macOS reference where useful.
- Normal, hover, pressed, selected, focused, disabled, loading and error states
  where applicable; light, dark and solid-fallback appearances.
- Keyboard and screen-reader behavior, text scaling, narrow/wide layouts,
  translated text and RTL layout.
- Which behavior must be preserved and how the result will be checked.

The review should cover ordinary chats and the parallel replies, topics,
pinned and scheduled views. The atlas notes that multiple chat-screen and
message-list implementations mirror one another.

For translucent surfaces, later validation must include bright/busy backdrops,
window activation, system transparency/high-contrast preferences, DPI changes,
resize/scroll behavior and media surfaces. Existing image-blur helpers are not
evidence that live backdrop blur has an acceptable rendering cost.
