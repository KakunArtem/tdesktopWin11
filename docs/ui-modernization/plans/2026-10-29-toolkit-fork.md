# Toolkit Fork Implementation Plan (C6)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give `lib_ui` and `lib_base` changes a home: forks under the user's GitHub account, a fork branch pinned to the commits tdesktop uses today, submodule URLs pointing at the forks, and a written rule for taking upstream updates.

**Architecture:** Each fork keeps upstream `master` untouched and carries the redesign on a `soft-panels` branch that starts at the commit tdesktop currently pins. tdesktop's `.gitmodules` points at the forks; submodule pointer updates are separate tdesktop commits. Upstream updates are rebased onto, never merged into, the fork branch.

**Tech Stack:** Git submodules, GitHub forks.

**Spec:** [architecture.md, C6](../architecture.md#components); roadmap decision 2 and risk "Submodule changes have no home".

**Visual reference:** [C6 toolkit fork](../component-references/c6-toolkit-fork/README.md): the branch layout, the `.gitmodules` change, the upstream update and the outputs that prove each step.

**Status (2026-10-31):** `lib_ui` is done: `soft-panels` pushed, `.gitmodules` pointed at the fork, [procedure](../toolkit-fork.md) written. `lib_base` keeps upstream until the user forks it.

**Depends on:** nothing. Must finish before [title bar](2026-10-29-title-bar.md) and [transient surfaces](2026-10-29-transient-surfaces.md).

## Global Constraints

- All [ground rules](README.md#ground-rules).
- Fork owner: the account that owns `origin` (`git@github.com:KakunArtem/tdesktopWin11.git`); forks are `KakunArtem/lib_ui` and `KakunArtem/lib_base`.
- Fork branch: `soft-panels`, created at the commit tdesktop pins at the time of this plan (`git submodule status Telegram/lib_ui Telegram/lib_base`). After an upstream rebase the current branch is `soft-panels-<date>`; `.gitmodules` always names the current one.
- Creating the forks and pushing to them are the user's actions or need the user's explicit approval at that moment.
- Never force-push `soft-panels`; never create backup branches or tags.

## Review Focus

- A fresh clone with `git clone --recursive`: must fetch the fork commits, not fail on a missing upstream commit. Pinned by Task 2 Step 4.
- An existing checkout after the URL change: `git submodule sync` must be run, or fetches still go to upstream. Pinned by Task 2 Step 3.
- An upstream tdesktop merge that bumps `lib_ui`: the fork branch must be rebased onto the new upstream commit before the pointer is accepted. Pinned by Task 3's procedure.
- A fork commit that exists only locally: the tdesktop pointer must never reference it. Pinned by Task 2 Step 2.
- `lib_base` unchanged by the redesign: the fork may stay at the pinned commit with no extra commits; that is valid. Pinned by Task 1 Step 3.

---

### Task 1: Create the forks and branches

**Files:** none in tdesktop.

- [ ] **Step 1: Record the pinned commits**

Run: `git submodule status Telegram/lib_ui Telegram/lib_base`
Expected: two lines with the pinned SHAs. Write them down for Step 3.

- [ ] **Step 2: Ask the user to fork**

Ask the user to fork `https://github.com/desktop-app/lib_ui` and `https://github.com/desktop-app/lib_base` under their account, or to approve doing it with `gh repo fork desktop-app/lib_ui --clone=false` and the same for `lib_base`.

- [ ] **Step 3: Create `soft-panels` at the pinned commits**

In a clone of each fork, whose `origin` is the fork:

```bash
git branch soft-panels <pinned sha>
git push origin soft-panels
```

Done for `lib_ui` on 2026-10-31 at `79c846fd`. `lib_base` waits for its fork (pinned `95a543e4`). Pushing needs the user's approval.

- [ ] **Step 4: Verify**

Run: `git ls-remote git@github.com:KakunArtem/lib_ui.git refs/heads/soft-panels`
Expected: the pinned `lib_ui` SHA. Same for `lib_base`.

---

### Task 2: Point tdesktop at the forks

**Files:**
- Modify: `.gitmodules`
- Create: `docs/ui-modernization/toolkit-fork.md`

- [ ] **Step 1: Change the URLs and branches**

For `Telegram/lib_ui` and `Telegram/lib_base` in `.gitmodules`, set `url` to `https://github.com/KakunArtem/lib_ui.git` / `lib_base.git` and add `branch = soft-panels`. HTTPS keeps anonymous clones working.

- [ ] **Step 2: Check the pointers are reachable from the fork**

Run in each submodule: `git branch -r --contains $(git rev-parse HEAD)`
Expected: `origin/soft-panels` is listed, since `origin` is the fork after Step 3's sync.

- [ ] **Step 3: Sync**

Run: `git submodule sync Telegram/lib_ui Telegram/lib_base` then `git -C Telegram/lib_ui remote get-url origin`
Expected: the fork URL.

- [ ] **Step 4: Fresh clone check**

In a scratch directory outside the checkout: `git clone --recursive <origin url> clone-check` after pushing this commit to a branch (user approval), or with `--branch` on the pushed branch. Expected: `Telegram/lib_ui` checks out at the pinned SHA with no errors. Delete the scratch clone.

- [ ] **Step 5: Write the fork procedure**

`docs/ui-modernization/toolkit-fork.md` records: the fork URLs, the branch rule, how to land a change (commit in the fork on `soft-panels`, push, then a tdesktop commit "Updated lib_ui for <change>." that moves the pointer), and Task 3's update procedure. Link it from `docs/ui-modernization/README.md` under "Working documents".

- [ ] **Step 6: Commit**

```bash
git add .gitmodules docs/ui-modernization/toolkit-fork.md docs/ui-modernization/README.md
git commit -m "Pointed lib_ui and lib_base at the redesign forks."
```

---

### Task 3: Upstream update procedure (documentation only)

Written into `docs/ui-modernization/toolkit-fork.md` in Task 2 Step 5; executed whenever upstream tdesktop is merged into this fork.

- [ ] **Step 1: Document the procedure**

1. After merging upstream tdesktop, read the new pinned SHA from the merge (`git diff HEAD^1 HEAD -- Telegram/lib_ui`).
2. In the submodule: `git fetch https://github.com/desktop-app/lib_ui.git master`, then `git rebase --onto <new upstream sha> <old upstream sha> soft-panels`. Resolve conflicts by reading both sides' history.
3. Push `soft-panels` as a new branch name `soft-panels-<date>` (no force-push), and point `.gitmodules`'s `branch` at it.
4. Move the tdesktop pointer to the rebased tip in its own commit and build Debug.

- [ ] **Step 2: Commit** (if not already part of Task 2's commit)

```bash
git add docs/ui-modernization/toolkit-fork.md
git commit -m "Documented the toolkit fork update procedure."
```

## Done when

- Both forks exist with `soft-panels` at the pinned commits.
- `.gitmodules` points at the forks, and a fresh recursive clone works.
- The procedure document is linked from the README.
