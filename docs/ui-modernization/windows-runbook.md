# Windows runbook

Date: 2026-10-31. The first steps on the Windows machine, in order, from a
fresh setup to the first task of Phase 1. The design and plans are done on
macOS; everything from C1 on needs Windows 11.

Paths follow the C1 helper scripts: the checkout is
`C:\Users\artemKakun\Projects\tdesktopWin11`, so the build folder is
`C:\Users\artemKakun\Projects`. Adjust both if the machine differs.

## 1. Check the machine

- Windows 11 build 22621 or later (`winver`): material needs it, and an older
  build makes C1 refuse by design.
- Settings > Personalization > Colors > Transparency effects on.
- About 55 GB free for the libraries and the build.
- Telegram API credentials (`api_id`, `api_hash`), from
  [api_credentials.md](../api_credentials.md).

## 2. Install the toolchain

Follow [building-win.md](../building-win.md):

- Visual Studio 2026 with the 10.0.26100.0 SDK. The build uses the
  `-vcvars_ver=14.44` toolset, which the root `AGENTS.md` still calls
  Visual Studio 2022; `building-win.md` is current.
- Python 3.10 added to `PATH`. `python3` must start a real Python 3, not the
  Microsoft Store alias, for the AI workflow helpers.
- Git.

## 3. Get the checkout

If the checkout already exists, update it; otherwise clone it into
`C:\Users\artemKakun\Projects`:

```bash
git clone --recursive git@github.com:KakunArtem/tdesktopWin11.git
```

An existing checkout pulls `dev` and syncs the submodules, since `lib_ui` now
comes from the fork:

```bash
git pull origin dev
git submodule sync --recursive
git submodule update --init --recursive
```

Then set up `lib_ui` for commits, as [toolkit-fork.md](toolkit-fork.md)
describes:

```bash
git -C Telegram/lib_ui switch soft-panels
git -C Telegram/lib_ui remote set-url --push origin git@github.com:KakunArtem/lib_ui.git
```

`* text=auto` checks text files out with CRLF, as the root `AGENTS.md` asks;
nothing to convert.

## 4. Build the libraries (hours)

Open an x64 terminal:

```bat
%comspec% /k "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.44
```

From `C:\Users\artemKakun\Projects`, run the prepare script. It fills
`ThirdParty` and `win64\Libraries` next to the checkout, which the build
expects:

```bat
tdesktopWin11\Telegram\build\prepare\win.bat
```

Start it first: it is the longest step, and nothing below needs to wait for
anything else.

## 5. Configure and build upstream once

In the same terminal, from `tdesktopWin11\Telegram`:

```bat
configure.bat x64 -D TDESKTOP_API_ID=YOUR_API_ID -D TDESKTOP_API_HASH=YOUR_API_HASH
```

Build Debug only, never Release, from the checkout root:

```bat
cmake --build out --config Debug --target Telegram
```

Run `out\Debug\Telegram.exe`, sign in to a test account, and check that it
looks exactly like upstream. This is the baseline every solid-parity check
compares with.

## 6. Start Phase 1

1. C1 [native backdrop](plans/2026-10-28-native-backdrop-prototype.md),
   Task 0: create the ignored `.ai\setup\build-target.cmd` and
   `configure-material-tests.cmd`. From then on every plan builds with
   `cmd /c .ai\setup\build-target.cmd <target>`.
2. C1 Tasks 1–6, then the go/no-go record in
   `decisions/2026-10-native-backdrop.md`. Its result is the first open
   decision in [architecture.md](architecture.md#open-decisions).
3. C16 [verification tooling](plans/2026-10-29-verification-tooling.md) can
   run alongside from Task 1. Its `tools/wallpapers.html` is already written;
   Task 1 continues with `Export-Wallpapers.ps1`.
4. After a go: C2 onwards, in the order of the
   [plans index](plans/README.md). C6 is done for `lib_ui`.

Each plan starts by reading the root `AGENTS.md` and the plan's Global
Constraints. Update its row in the [progress table](architecture.md#progress)
as tasks commit.

## If something goes wrong

- Libraries not found: the checkout must sit next to `win64\Libraries` and
  `ThirdParty`, in the folder the prepare script ran from.
- Wrong command prompt: use the x64 terminal from step 4.
- A locked `Telegram.exe` or `.pdb`: close the running Debug Telegram, or
  ask before stopping it.
