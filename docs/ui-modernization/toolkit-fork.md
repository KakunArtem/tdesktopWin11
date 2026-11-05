# Toolkit forks

Date: 2026-11-05. Status: C6 complete; both public forks publish `soft-panels`
at the unchanged source pins. Component: C6 in the
[architecture](architecture.md#components). Plan:
[toolkit fork](plans/2026-10-29-toolkit-fork.md).

## Repositories and pins

| Submodule | Public fetch URL | Current branch | Upstream base / C6 pin |
| --- | --- | --- | --- |
| `Telegram/lib_ui` | `https://github.com/KakunArtem/lib_ui.git` | `soft-panels` | `79c846fdf0327ff99d3efdc1cc4e11ef50bedc6d` |
| `Telegram/lib_base` | `https://github.com/KakunArtem/lib_base.git` | `soft-panels` | `95a543e426e35938dd632139d0bb828b268c3b4c` |

C6 changes fetch locations and branch setup, not source versions. Both
superproject gitlinks remain at the listed pins. The sibling clones
`../lib_ui` and `../lib_base` are available for fork development; the build
uses the pinned checkouts under `Telegram/`. Their existing `master`
branches are preserved. A fork can have no redesign commits yet; this is
valid for `lib_base`.

## Branch and publication rules

- Keep upstream history on `master`; redesign work belongs on the current
  branch named by `.gitmodules`.
- Initially that branch is `soft-panels`. An upstream rebase is published on
  a new active branch, `soft-panels-<date>`; choose a unique suffix if the
  date is already used. Update `.gitmodules` to the new current name.
- Never force-push, merge upstream into the redesign branch, or create
  backup branches/tags. Existing published branches remain untouched;
  reflogs retain prior local tips.
- Fork creation and every push require the user's explicit approval.
- Keep `lib_ui` hooks opt-in, preserving upstream behavior when unset; see
  [toolkit hooks](architecture.md#toolkit-fork-hooks).
- A tdesktop pointer must reference a commit already reachable from the
  fork's published current branch. Verify this before committing the pointer.

## Setting up an existing checkout

After a URL change, synchronize both submodules. Normal updates use the
superproject pins; `.gitmodules`'s branch field does not replace the gitlinks.
Do not use `submodule update --remote` as a shortcut for accepting new code.

```powershell
git submodule sync -- Telegram/lib_ui Telegram/lib_base
git submodule update --init --recursive -- Telegram/lib_ui Telegram/lib_base
git -C Telegram/lib_ui remote get-url origin
git -C Telegram/lib_base remote get-url origin
```

The remote URLs must match the public HTTPS URLs above. Fresh recursive
clones can read them without authentication. Worktree initialization can
leave submodules detached at the pin; switch to the current redesign branch
before editing. If it does not exist locally, create it to track the fetched
`origin/<current-branch>`. Preserve unrelated changes and local commits.

SSH can be configured as a push URL while retaining anonymous HTTPS fetches:

```powershell
git -C Telegram/lib_ui remote set-url --push origin git@github.com:KakunArtem/lib_ui.git
git -C Telegram/lib_base remote set-url --push origin git@github.com:KakunArtem/lib_base.git
```

## Landing a toolkit change

1. Check the selected clone is clean and on the current redesign branch.
   Fetch that branch and inspect divergence before integrating others' work.
2. Commit the toolkit change there and run the requested validation. Obtain
   approval, then push without force: `git push origin HEAD:<current-branch>`.
3. Fetch the published branch again. `git branch -r --contains HEAD` must
   include `origin/<current-branch>`; `git ls-remote` must advertise the
   expected published tip. A local-only commit is not ready for tdesktop.
4. Update the corresponding submodule checkout, then commit its gitlink in
   tdesktop separately from app code: `Updated lib_ui for <change>.` or
   `Updated lib_base for <change>.`

## Taking upstream updates

Apply this procedure to either toolkit when integrating upstream tdesktop.
Inspect its proposed new toolkit pin before accepting it; do not silently
replace our published redesign tip with a bare upstream pointer.

1. Read the upstream pin from the incoming superproject ref, for example
   `git rev-parse <upstream-tdesktop-ref>:Telegram/lib_ui`. Record the current
   published fork branch/tip and its old upstream base. The table above holds
   the initial bases; update the current base after each accepted rebase.
2. In the toolkit clone, inspect `git status` and remotes. Add `upstream` as
   `https://github.com/desktop-app/lib_ui.git` or `lib_base.git` if absent;
   verify an existing remote rather than overwriting an unrelated one.
3. Fetch the current fork branch and the exact incoming upstream commit.
   Create the intended new active branch from the published fork tip, then
   replay only the redesign tail:

```powershell
$module = 'Telegram/lib_ui'
$currentBranch = 'soft-panels'
$newBranch = 'soft-panels-YYYY-MM-DD'
$oldUpstream = '<recorded upstream base>'
$newUpstream = '<incoming upstream toolkit pin>'
git -C $module fetch origin $currentBranch
git -C $module fetch upstream $newUpstream
git -C $module switch -c $newBranch "origin/$currentBranch"
git -C $module rebase --onto $newUpstream $oldUpstream $newBranch
```

4. Resolve conflicts by reading both sides' history and preserving intent.
   The new branch is the next working/publication branch, not a backup.
   Neither the existing published branch nor local `master` is rewritten.
5. Obtain approval and push the new branch without force. Fetch it again
   and verify its tip and reachability from `origin/<new-branch>`.
6. Set the module's `.gitmodules` branch to the new name and accept the
   rebased fork tip as tdesktop's gitlink in its own pointer-update commit.
   Record the new upstream base in this document. Never land an unpublished
   rebased SHA or discard the fork tail during an upstream merge.
7. Build Debug for the changed source version. Never build Release. C6's
   initial URL/branch setup leaves source versions unchanged, so it needs Git
   clone/reachability checks rather than a new application build.

## C6 verification

On 2026-11-05, `git ls-remote` advertised both public `soft-panels` branches
at the exact pins in the table. `lib_base` publication was explicitly
approved by the user. No `master` branch or superproject gitlink changed.

The existing checkout was synchronized with `git submodule sync`; both
`origin` fetch URLs are the fork HTTPS URLs. Each pinned commit is reachable
from the fetched `origin/soft-panels`. Both sibling clones remain clean on
their original `master` checkouts; their local `soft-panels` branches track
the published branch.

A fresh scratch superproject clone of the local candidate configuration
initialized `Telegram/lib_ui` and `Telegram/lib_base` recursively through
anonymous public HTTPS, with global/system Git configuration disabled. Both
toolkits checked out at the unchanged pins without local object alternates.
Only the superproject borrowed local objects; no superproject branch was
published. This checks the two changed toolkit URLs and their recursive
contents, not the unchanged third-party submodule graph. Neither toolkit
contains nested submodules at these pins.

Source versions are unchanged, so C6 requires no application rebuild or
visible UI acceptance. Future source changes still require the validation
described above.
