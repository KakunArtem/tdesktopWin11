# Toolkit fork

Date: 2026-10-31. Status: in use for `lib_ui`; `lib_base` pending. Component:
C6 in the [component architecture](architecture.md#components). Plan:
[toolkit fork](plans/2026-10-29-toolkit-fork.md).

Redesign changes to `lib_ui` and `lib_base` live in forks under the account
that owns `origin`. Each fork keeps upstream `master` untouched and carries the
redesign on its own branch.

## Forks

| Submodule | Fork | Branch | Starts at | State |
| --- | --- | --- | --- | --- |
| `Telegram/lib_ui` | `https://github.com/KakunArtem/lib_ui.git` | `soft-panels` | `79c846fd`, the commit tdesktop pinned on 2026-10-31 | Pushed; `.gitmodules` points at it. |
| `Telegram/lib_base` | `https://github.com/KakunArtem/lib_base.git` | `soft-panels` | `95a543e4` | Not forked yet. `.gitmodules` keeps upstream until it is; no plan changes `lib_base`. |

`.gitmodules` names the fork's HTTPS URL, so anonymous clones work, and the
current branch in `branch`.

## Branch rule

- The redesign branch is `soft-panels`. After an upstream rebase it is
  `soft-panels-<date>`, and `.gitmodules` names the current one.
- Never force-push a redesign branch, and never create backup branches or
  tags. The branch reflog keeps the old tip.
- Every `lib_ui` change is an opt-in hook that leaves upstream behavior
  unchanged when unset; the list is in
  [architecture.md](architecture.md#toolkit-fork-hooks).

## Landing a change

1. In `Telegram/lib_ui`, switch to the redesign branch, commit the change and
   push it to the fork. Pushing needs the user's approval.
2. Check that the commit is published: `git branch -r --contains HEAD` lists
   `origin/soft-panels`. The tdesktop pointer never references a commit that
   exists only locally.
3. In tdesktop, commit the pointer move on its own:
   `Updated lib_ui for <change>.`

## Setting up a checkout

An existing checkout keeps fetching from the old URL until it is synced:

```bash
git submodule sync Telegram/lib_ui
git submodule update --init Telegram/lib_ui
git -C Telegram/lib_ui remote get-url origin
```

The last command must print the fork URL. A fresh `git clone --recursive`
needs nothing extra.

## Taking upstream updates

Run this whenever an upstream tdesktop merge moves the `lib_ui` pointer.

1. Read the new upstream commit from the merge:
   `git diff HEAD^1 HEAD -- Telegram/lib_ui`.
2. In the submodule, fetch upstream and replay the redesign commits onto it:
   `git fetch https://github.com/desktop-app/lib_ui.git master`, then
   `git rebase --onto <new upstream sha> <old upstream sha> soft-panels`.
   Resolve conflicts by reading both sides' history.
3. Push the result as a new branch, `soft-panels-<date>`, with no force-push,
   and point `.gitmodules`'s `branch` at it.
4. Move the tdesktop pointer to the rebased tip in its own commit and build
   Debug.
