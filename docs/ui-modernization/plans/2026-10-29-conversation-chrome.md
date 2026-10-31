# Conversation Chrome Implementation Plan (C11)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the chat header part of the conversation panel and turn the pinned, call, requests, sponsored, translate and contact-status bars into inset rounded pills, and add a "Window material" chat background, all without changing any bar's behavior.

**Architecture:** Bars keep their widgets and slide animations. A shared helper in `Ui::Material` paints a bar's area as the conversation panel plus an inset Bar-role pill under Soft Panels, and today's fill otherwise. Header and bar shadows hide under Soft Panels. A new KV pref lets `SectionWidget::PaintBackground` paint the Conversation role instead of the wallpaper while material is active.

**Tech Stack:** C++20, Qt 6.11 Widgets, `td_ui` bars (`ui/chat/*_bar.*`), `Window::SectionWidget`, `Core::Settings` KV prefs.

**Spec:** [architecture.md, Element styling](../architecture.md#element-styling-c9c15); exploration CSS `.chat-header`, `.pinned`; the exploration's chat wallpaper note ("a chat wallpaper fills the message area below the header").

**Visual reference:** [C11 conversation chrome](../component-references/c11-conversation-chrome/README.md): the header and pinned pill from the mockup, several bars stacked as pills, and the chat background option with wallpaper and with material.

**Depends on:** [shell layout](2026-10-29-shell-layout.md), [surface roles](2026-10-29-surface-roles.md).

## Global Constraints

- All [ground rules](README.md#ground-rules).
- Header height under Soft Panels: `softPanelsTopBarHeight` 56 px, the exploration's (user decision 2026-10-31); upstream keeps `topBarHeight` (54 px). The chats list header aligns with it, as it does today, and C15 gives the info pane's top bar the same height.
- Bar pill at 100 % scale: margins `12, 0, 12, 4` inside the bar's row, radius `9`, Bar role, border `Border(Surface::Bar)`; the existing accent line of `Ui::MessageBar` stays.
- Chat background pref: key `window_material_chat_background`, `bool`, default `false`; label "Show window material behind messages"; shown only when `Window::Material::Capable()`.
- With the pref off, the user's wallpaper fills the message area exactly as today, also with material active.
- Every bar keeps its show and hide animation, close button, click target and context menu.

## Review Focus

- Several bars at once (group call, requests, pinned and translate in a large group): pills stack with the 4 px gap and no double borders. Pinned by Task 2 Step 4.
- Pinned bar with multiple pinned messages: the bar's vertical position indicator and message switching animation stay inside the pill. Pinned by Task 2 Step 4.
- Chat theme (per-chat theme set by the other side) with the material background pref on: the chat theme's bubble colors stay, but the canvas shows material. Pinned by Task 3 Step 4.
- Header selection mode and search mode: header still fills the panel, no square corners at the panel's top. Pinned by Task 1 Step 4.
- Topic tabs (`SubsectionTabs`, horizontal and vertical): their `st::windowBg` fills follow the Conversation role. Pinned by Task 2 Step 3.

---

### Task 1: Header inside the conversation panel

**Files:**
- Modify: `Telegram/SourceFiles/history/history_widget.cpp`
- Modify: `Telegram/SourceFiles/history/view/history_view_chat_section.cpp`, `history_view_pinned_section.cpp`, `history_view_scheduled_section.cpp`, `history_view_welcome_messages_section.cpp`
- Modify: `Telegram/SourceFiles/history/view/history_view_top_bar_widget.cpp`, `history/view/controls/history_view_compose_search.cpp`, `dialogs/dialogs_widget.cpp`, `profile/profile_back_button.cpp`
- Modify: `Telegram/SourceFiles/window/window_soft_panels.style`, `window_soft_panels.h`, `window_soft_panels.cpp`

- [ ] **Step 1: Hide header shadows under Soft Panels**

`HistoryWidget::_topShadow` and each section's `_topBarShadow` stay hidden while `Ui::Material::SoftPanels()`; wherever the code shows them, add the condition. The header's bottom edge is then the panel's continuous fill, as in the exploration.

- [ ] **Step 2: Header height**

Add `softPanelsTopBarHeight: 56px` to `window/window_soft_panels.style` and `[[nodiscard]] int Window::SoftPanels::TopBarHeight();` to `window_soft_panels.h`: the new value under Soft Panels, `st::topBarHeight` otherwise. Replace the reads of `st::topBarHeight` in `history_view_top_bar_widget.cpp`, `history_view_chat_section.cpp`, `history_view_compose_search.cpp`, `dialogs_widget.cpp` and `profile_back_button.cpp` with it; the theme preview (`window_theme_preview.cpp`) keeps the upstream value. Recheck at the pinned commit for reads through other styles that copy `topBarHeight`.

- [ ] **Step 3: Build**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`

- [ ] **Step 4: Check**

With Soft Panels on, material on and off: an ordinary chat, a topic, pinned messages, scheduled messages, header search mode and message selection mode. The header blends into the panel; its top corners are rounded by the panel frame in every mode.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/history/history_widget.cpp Telegram/SourceFiles/history/view Telegram/SourceFiles/dialogs/dialogs_widget.cpp Telegram/SourceFiles/profile/profile_back_button.cpp Telegram/SourceFiles/window/window_soft_panels.style Telegram/SourceFiles/window/window_soft_panels.h Telegram/SourceFiles/window/window_soft_panels.cpp
git commit -m "Merged the chat header into the Soft Panels conversation panel."
```

---

### Task 2: Bars as pills

**Files:**
- Modify: `Telegram/SourceFiles/ui/ui_material.h`, `ui_material.cpp`
- Modify: `Telegram/SourceFiles/window/window_soft_panels.style`
- Modify: `Telegram/SourceFiles/ui/chat/pinned_bar.cpp`, `group_call_bar.cpp`, `requests_bar.cpp`, `more_chats_bar.cpp`, `sponsored_message_bar.cpp`
- Modify: `Telegram/SourceFiles/history/view/history_view_translate_bar.cpp`, `history_view_contact_status.cpp`, `history_view_subsection_tabs.cpp`

**Interfaces:**
- Produces style values: `softPanelsBarMargins: margins(12px, 0px, 12px, 4px)`, `softPanelsBarRadius: 9px`.
- Produces, in `namespace Ui::Material`:
  - `[[nodiscard]] QRect BarPillRect(const QRect &row);` — `row` shrunk by `softPanelsBarMargins`.
  - `void PaintBar(QPainter &p, const QRect &row, const QRect &clip, const style::color &solid);` — without Soft Panels `p.fillRect(clip, solid)`; with Soft Panels the Conversation role over `clip`, then the Bar role as a rounded rect at `BarPillRect(row)` and its border.
  - `void PaintBarContent(QPainter &p, const QRect &row, const style::color &solid);` — for inner widgets that today fill `st::historyComposeAreaBg`: nothing under Soft Panels (the pill is already painted), else `fillRect(row, solid)`.

- [ ] **Step 1: Implement the helpers**

- [ ] **Step 2: Use them in the `td_ui` bars**

In `PinnedBar`, `GroupCallBar`, `RequestsBar` and `MoreChatsBar`: the `_wrap` paint becomes `PaintBar(p, _wrap->rect(), clip, st::historyPinnedBg)`, `_wrap`'s opaque attribute becomes `SetOpaquePaint(_wrap.entity(), !SoftPanels())`, the inner `historyComposeAreaBg` fill becomes `PaintBarContent`, and each bar's own bottom shadow stays hidden under Soft Panels. Offset the inner content by `BarPillRect`'s left and right margins so text and buttons sit inside the pill. Apply `PaintBar` in `SponsoredMessageBar` the same way.

- [ ] **Step 3: Use them in the app bars**

`TranslateBar` (`_st.itemBg` fills and the `historyComposeButtonBg` button fill), the contact status bars in `history_view_contact_status.cpp` (`historyContactStatusButton.bgColor` fills) and `SubsectionTabs` (`st::windowBg` fills, which become the Conversation role, not a pill).

- [ ] **Step 4: Build and check**

Open a group with an active call, join requests, several pinned messages and a foreign-language message; a sponsored channel message; a chat with a non-contact (add/block bar); a forum with topic tabs in both orientations. Show, switch, hide and close each bar; with Soft Panels on and off; material on and off. Stacked bars match panel B of the visual reference.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/ui/ui_material.h Telegram/SourceFiles/ui/ui_material.cpp Telegram/SourceFiles/window/window_soft_panels.style Telegram/SourceFiles/ui/chat Telegram/SourceFiles/history/view/history_view_translate_bar.cpp Telegram/SourceFiles/history/view/history_view_contact_status.cpp Telegram/SourceFiles/history/view/history_view_subsection_tabs.cpp
git commit -m "Turned conversation bars into Soft Panels pills."
```

---

### Task 3: "Window material" chat background

**Files:**
- Modify: `Telegram/SourceFiles/window/window_material.h`, `window_material.cpp`
- Modify: `Telegram/SourceFiles/window/section_widget.cpp`
- Modify: `Telegram/SourceFiles/settings/sections/settings_chat.cpp`
- Modify: `Telegram/Resources/langs/lang.strings`

**Interfaces:**
- Produces, in `namespace Window::Material`:
  - `[[nodiscard]] bool ChatBackgroundEnabled();` / `void SetChatBackgroundEnabled(bool enabled);` — the KV pref.
  - `[[nodiscard]] bool ChatBackgroundActive();` — `ChatBackgroundEnabled() && Active()`.
  - `[[nodiscard]] rpl::producer<bool> ChatBackgroundActiveValue();`

- [ ] **Step 1: Pref and phrase**

Add the functions; add `"lng_settings_bg_window_material" = "Show window material behind messages";` after `lng_settings_adaptive_wide` in `lang.strings`.

- [ ] **Step 2: Paint**

At the start of the innermost `SectionWidget::PaintBackground(QPainter&, ...)` overload, when `ChatBackgroundActive()`, paint `Ui::Material::PaintSurface(p, clip, Surface::Conversation, st::windowBg)` and return. Repaint the main section on `ChatBackgroundActiveValue()` changes.

- [ ] **Step 3: Setting**

In `SetupChatBackground`, add a checkbox after "Adaptive layout for wide screens", built the same way, visible only when `Window::Material::Capable()`, writing the pref and calling `Core::App().saveSettingsDelayed()`. It applies live.

- [ ] **Step 4: Build and check**

Turn the checkbox on and off with material on: the canvas switches between wallpaper and material immediately. With material off (transparency effects off), the wallpaper shows even with the checkbox on. Open a chat with a per-chat theme: bubbles keep that theme's colors. Service messages and date pills are handled by [messages](2026-10-29-messages.md); note how they look now in the check record.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/window/window_material.h Telegram/SourceFiles/window/window_material.cpp Telegram/SourceFiles/window/section_widget.cpp Telegram/SourceFiles/settings/sections/settings_chat.cpp Telegram/Resources/langs/lang.strings
git commit -m "Added window material chat background option."
```

---

### Task 4: Checks

**Files:**
- Create: `docs/ui-modernization/checks/conversation-chrome.md`

- [ ] **Step 1: Run the check rows**

Solid parity, Material (with wallpaper and with the material background), Contrast (header title and status, every bar's text), States (each bar shown, switching and dismissed; header search, selection, connecting status), Scale and RTL. Record each result.

- [ ] **Step 2: Commit**

```bash
git add docs/ui-modernization/checks/conversation-chrome.md
git commit -m "Recorded conversation chrome checks."
```

## Done when

- Every check row passes and is recorded.
- With Soft Panels off and the pref off, header, bars and wallpaper are unchanged.
