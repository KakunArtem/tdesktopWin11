# Material State Implementation Plan (C2)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the Phase 1 prototype option with a real "Window transparency" setting and a material state service that every later component can query and observe.

**Architecture:** The pure policy from Phase 1 keeps deciding capability at startup and gains a live check for settings that change while the app runs. A small service in `window/window_material.*` caches the startup capability, exposes `Active()` as an `rpl` value, and refreshes it on `WM_SETTINGCHANGE`. The Windows layer applies the backdrop to every main window, not only to the prototype.

**Tech Stack:** C++20, Qt 6.11 Widgets, `rpl`, `Core::Settings` KV prefs, Win32 DWM.

**Spec:** [architecture.md, Material state (C2)](../architecture.md#material-state-c2) and [implementation-plan.md, Phase 2](../implementation-plan.md#phase-2-material-foundation).

**Depends on:** the Phase 1 plan executed, with its decision record in `docs/ui-modernization/decisions/2026-10-native-backdrop.md` saying go or conditional go.

## Global Constraints

- All [ground rules](README.md#ground-rules).
- Minimum Windows build for material: `22621` (`kMinimumBackdropBuild`).
- Setting key: `window_material`, a `bool` KV pref, default `false`.
- Setting label: "Use window transparency"; changing it asks for a restart.
- Material is refused while the native window frame is on.
- With the setting off, the main window is created exactly as before Phase 1 (not translucent).

## Review Focus

- Transparency effects turned off in Windows Settings while the app runs: `ActiveValue()` must emit `false` without a restart. Pinned by Task 1 `TestLiveAllows` and Task 4 Step 4.
- High contrast turned on while the app runs: the same. Pinned by Task 1 and Task 4 Step 5.
- Native frame turned on while material is on: material must stay off after the restart the native frame setting already triggers. Pinned by Task 4 Step 6.
- A separate chat window (opened via "Open in new window"): it must get the same backdrop as the primary window or none, never a black background. Pinned by Task 3 Step 3.
- Settings opened on Windows 10 or an older Windows 11 build: the checkbox must not appear. Pinned by Task 2 Step 6.

---

### Task 1: Live environment check in the policy

**Files:**
- Modify: `Telegram/SourceFiles/window/window_material_policy.h`
- Modify: `Telegram/SourceFiles/window/window_material_policy.cpp`
- Test: `Telegram/SourceFiles/tests/test_window_material.cpp`

**Interfaces:**
- Consumes: `Environment`, `Decide`, `kMinimumBackdropBuild` from Phase 1.
- Produces: `[[nodiscard]] bool LiveAllows(const Environment &environment);` — true when transparency effects are on and high contrast is off; ignores `requested`, `windowsBuild` and `nativeFrame`, which are startup-only.

- [ ] **Step 1: Write the failing test**

Add to `test_window_material.cpp` and call it from `main()`:

```cpp
void TestLiveAllows() {
	Check(LiveAllows(Supported()), "supported system allows live");

	auto opaque = Supported();
	opaque.transparencyEffects = false;
	Check(!LiveAllows(opaque), "transparency off stops live material");

	auto contrast = Supported();
	contrast.highContrast = true;
	Check(!LiveAllows(contrast), "high contrast stops live material");

	auto notRequested = Supported();
	notRequested.requested = false;
	Check(LiveAllows(notRequested), "the request is a startup decision");
}
```

- [ ] **Step 2: Build the test and confirm it fails to compile**

Run: `cmd /c .ai\setup\build-target.cmd test_window_material`
Expected: error `'LiveAllows': identifier not found`.

- [ ] **Step 3: Declare and implement `LiveAllows` in `window_material_policy.{h,cpp}`**

- [ ] **Step 4: Run the tests**

Run: `cmd /c .ai\setup\build-target.cmd test_window_material` then `.\out\Debug\test_window_material.exe; $LASTEXITCODE`
Expected: `Checks: 17, failures: 0` and `0`.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/window/window_material_policy.h Telegram/SourceFiles/window/window_material_policy.cpp Telegram/SourceFiles/tests/test_window_material.cpp
git commit -m "Added live environment check to window material policy."
```

---

### Task 2: "Window transparency" setting replaces the prototype option

**Files:**
- Create: `Telegram/SourceFiles/window/window_material.h`
- Create: `Telegram/SourceFiles/window/window_material.cpp`
- Modify: `Telegram/SourceFiles/platform/win/window_material_win.h`, `window_material_win.cpp`
- Modify: `Telegram/SourceFiles/window/main_window.h`, `main_window.cpp`
- Modify: `Telegram/SourceFiles/settings/settings_experimental.cpp`
- Modify: `Telegram/SourceFiles/settings/sections/settings_advanced.cpp`
- Modify: `Telegram/Resources/langs/lang.strings`
- Modify: `Telegram/CMakeLists.txt`

**Interfaces:**
- Produces, in `namespace Window::Material` (`window_material.h`):
  - `[[nodiscard]] bool SettingAvailable();` — Windows with build `>= kMinimumBackdropBuild`; `false` elsewhere.
  - `[[nodiscard]] bool SettingEnabled();` — `Core::App().settings().readPref<bool>("window_material", false)`.
  - `void SetSettingEnabled(bool enabled);` — writes the pref and calls `Core::App().saveSettingsDelayed()`.
- Produces, in `namespace Platform::Material`: `[[nodiscard]] bool Supported();` — the build check used by `SettingAvailable()`.

- [ ] **Step 1: Create `window_material.{h,cpp}` with the three functions**

`SettingAvailable()` calls `Platform::Material::Supported()` under `#ifdef Q_OS_WIN` and returns `false` otherwise. Register both files in the `window/` block of `Telegram/CMakeLists.txt` after `window/window_material_policy.h`.

- [ ] **Step 2: Read the setting in the Windows environment reader**

In `Platform::Material::ReadEnvironment()`, set `.requested = Window::Material::SettingEnabled()` instead of reading `kOptionWindowMaterialPrototype`. Add `Supported()`.

- [ ] **Step 3: Remove the prototype option**

Delete `kOptionWindowMaterialPrototype` and `OptionWindowMaterialPrototype` from `main_window.{h,cpp}`, and its line from the `settings_experimental.cpp` list. Rename the anonymous `UseMaterialPrototype()` in `main_window.cpp` to `UseMaterial()`; its body is unchanged.

- [ ] **Step 4: Add the phrase**

In `lang.strings`, after `lng_settings_native_frame`:

```
"lng_settings_window_material" = "Use window transparency";
```

- [ ] **Step 5: Add the checkbox in both window-title builders**

In `BuildWindowTitleSection` (id `u"advanced/window_material"_q`, keywords `transparency`, `acrylic`, `window`) and in `SetupWindowTitleContent`, add a checkbox after the native frame checkbox, shown only when `Window::Material::SettingAvailable()`. On a change that differs from `SettingEnabled()`: call `SetSettingEnabled(checked)` and show the restart confirmation used by `BuildANGLEOption` (`Ui::MakeConfirmBox` with `tr::lng_settings_need_restart()`, `tr::lng_settings_restart_now()` and `Core::Restart()` when confirmed). Take the controller from `builder.controller()` or the `controller` argument; when it is null, skip the box and keep the saved value.

- [ ] **Step 6: Build and check the settings page**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`
Expected: ends with `-> ...\out\Debug\Telegram.exe`.

Start Telegram. Settings > Advanced > Window title bar shows "Use window transparency" below "Use system window frame". Experimental settings no longer list "Window material prototype". Toggle the checkbox on, confirm the restart, and check that `log.txt` contains `refusal: none`. On a machine below build 22621 (or with `Supported()` temporarily returning `false`, not committed) the checkbox is absent.

- [ ] **Step 7: Commit**

```bash
git add Telegram/SourceFiles/window/window_material.h Telegram/SourceFiles/window/window_material.cpp Telegram/SourceFiles/platform/win/window_material_win.h Telegram/SourceFiles/platform/win/window_material_win.cpp Telegram/SourceFiles/window/main_window.h Telegram/SourceFiles/window/main_window.cpp Telegram/SourceFiles/settings/settings_experimental.cpp Telegram/SourceFiles/settings/sections/settings_advanced.cpp Telegram/Resources/langs/lang.strings Telegram/CMakeLists.txt
git commit -m "Added window transparency setting."
```

---

### Task 3: Material state service

**Files:**
- Create: `Telegram/SourceFiles/ui/ui_material.h`, `ui_material.cpp` (`td_ui` target)
- Modify: `Telegram/cmake/td_ui.cmake`
- Modify: `Telegram/SourceFiles/window/window_material.h`, `window_material.cpp`
- Modify: `Telegram/SourceFiles/window/main_window.cpp`
- Modify: `Telegram/SourceFiles/platform/win/main_window_win.cpp`
- Modify: `Telegram/SourceFiles/platform/win/integration_win.cpp`

**Interfaces:**
- Consumes: `Platform::Material::Decide()`, `ReadEnvironment()`, `ApplyBackdrop(HWND, bool)` from Phase 1; `LiveAllows` from Task 1.
- Produces, in `namespace Window::Material`:
  - `[[nodiscard]] bool Capable();` — the startup decision was `Backdrop::Acrylic` and no `ApplyBackdrop` call has failed.
  - `[[nodiscard]] bool Active();` and `[[nodiscard]] rpl::producer<bool> ActiveValue();` — `Capable()` and `LiveAllows(ReadEnvironment())`; the producer starts with the current value and emits only on change.
  - `void Refresh();` — re-reads the environment and updates `Active()`.
  - `void ReportBackdropFailure();` — makes `Capable()` false for the rest of the process and logs once.
- Produces, in `namespace Ui::Material` (`ui/ui_material.h`, `td_ui`; see [Two namespaces, two targets](../architecture.md#two-namespaces-two-targets)): `[[nodiscard]] bool Active();`, `[[nodiscard]] rpl::producer<bool> ActiveValue();` and `void SetActive(bool active);` — a mirror that code in `td_ui` can read. Later plans add fills (C3), painting helpers (C4), grabs (C5) and the Soft Panels switch (C7) to this file.

- [ ] **Step 1: Implement the service in `window_material.cpp` and the mirror in `ui_material.cpp`**

State is a file-local `rpl::variable<bool>` plus a `std::optional<bool>` holding the startup decision, computed on the first `Capable()` call through `Platform::Material::Decide()` (Windows) and `false` elsewhere. Every change of `Window::Material::ActiveValue()` calls `Ui::Material::SetActive`. Register `ui/ui_material.cpp` and `.h` in `cmake/td_ui.cmake` next to the other `ui/` sources. `UseMaterial()` in `main_window.cpp` now returns `Window::Material::Capable()`, so the decision is logged once per process instead of once per window.

- [ ] **Step 2: Refresh on system setting changes**

In `WindowsIntegration`'s `WM_SETTINGCHANGE` case in `integration_win.cpp`, call `Window::Material::Refresh()` after `RefreshTaskbarThemeValue()`.

- [ ] **Step 3: Apply the backdrop to every main window**

In `MainWindow::initHook()` in `main_window_win.cpp`, replace the Phase 1 `testAttribute(Qt::WA_TranslucentBackground)` check with `Window::Material::Capable()`. When `ApplyBackdrop` returns `false`, call `ReportBackdropFailure()`. Keep the night-mode re-application in `setupNativeWindowFrame()` under the same condition. Separate chat windows are `Window::MainWindow` instances too, so they take this path; verify by opening a chat in a new window.

- [ ] **Step 4: Build and check startup logging**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`
Expected: builds. With the setting on, `log.txt` has exactly one `Material: ... refusal: none` line per launch and one `extend frame 0x0, dark mode 0x0, backdrop 0x0` line per window, including a separate chat window.

- [ ] **Step 5: Commit**

```bash
git add Telegram/SourceFiles/ui/ui_material.h Telegram/SourceFiles/ui/ui_material.cpp Telegram/cmake/td_ui.cmake Telegram/SourceFiles/window/window_material.h Telegram/SourceFiles/window/window_material.cpp Telegram/SourceFiles/window/main_window.cpp Telegram/SourceFiles/platform/win/main_window_win.cpp Telegram/SourceFiles/platform/win/integration_win.cpp
git commit -m "Added window material state service."
```

---

### Task 4: Live fallback checks

No code. Record results in `docs/ui-modernization/checks/material-state.md` (format from the [verification tooling plan](2026-10-29-verification-tooling.md)).

- [ ] **Step 1: Add a temporary observer**

In a local, uncommitted change to `window_material.cpp`, log every `ActiveValue()` emission as `Material: active <value>`.

- [ ] **Step 2: Setting off**

Setting off, start Telegram. `Test-Backdrop.ps1 -Expect Opaque` passes and the log shows `refusal: not requested`.

- [ ] **Step 3: Setting on**

Setting on, restart. The log shows `Material: active true`.

- [ ] **Step 4: Transparency effects toggled live**

Turn off Windows Settings > Personalization > Colors > Transparency effects without restarting Telegram. Expected log: `Material: active false`. Turn it back on: `Material: active true`.

- [ ] **Step 5: High contrast toggled live**

Press Left Alt + Left Shift + Print Screen to turn high contrast on, then off. Expected: `active false`, then `active true`.

- [ ] **Step 6: Native frame**

Turn on "Use system window frame" and restart. Expected: `refusal: native frame`, no `extend frame` line, and the window is opaque.

- [ ] **Step 7: Revert the observer and commit the record**

```bash
git checkout Telegram/SourceFiles/window/window_material.cpp
git add docs/ui-modernization/checks/material-state.md
git commit -m "Recorded window material state checks."
```

## Done when

- `test_window_material.exe` reports `failures: 0`.
- The prototype option no longer exists; the setting controls translucency.
- `Active()` follows transparency effects and high contrast without a restart.
- The check record in `docs/ui-modernization/checks/material-state.md` has every step passing.
