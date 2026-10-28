# Native Backdrop Prototype Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Prove or disprove that Telegram's main window can show the Windows 11 Acrylic system backdrop through a translucent region, and record the result as a go/no-go decision for the redesign.

**Architecture:** A pure decision policy (`Window::Material`) decides from the system environment whether a backdrop is allowed, and is unit-tested in its own executable. A Windows-only layer reads the environment and applies the DWM backdrop to the main window, which becomes translucent only behind an experimental option. One existing fill, the area above the chat list, paints a semi-transparent color so the backdrop can show through. A PowerShell script then measures from the screen whether pixels actually change when the content behind the window changes.

**Tech Stack:** C++20, Qt 6.11 Widgets, Win32 DWM (`DwmSetWindowAttribute`, `DwmExtendFrameIntoClientArea`), CMake with the Visual Studio generator, PowerShell 5.1.

**Roadmap:** Phase 1 of the [implementation roadmap](../implementation-plan.md).

---

## Ground rules for this repository

- Read the root `AGENTS.md` before starting, in particular `## Commits`, the
  comment rules and `## Text File Format`. Source files use tabs, CRLF line
  endings and no BOM.
- Commits: one plain subject in the style of `git log` (for example "Added
  window material policy."). No `Co-Authored-By` or tool trailers, and no
  `[ai]` prefix, because this is product code.
- Build Debug only. Never build Release.
- If `out\Debug\Telegram.exe` is running, ask the user to close it before
  building; do not kill it.
- Everything here sits behind the "Window material prototype" option, off by
  default. With the option off, the app must behave exactly as before.

## File structure

| File | Responsibility |
| --- | --- |
| Create `Telegram/SourceFiles/window/window_material_policy.h` | Pure types and decision function, no platform headers. |
| Create `Telegram/SourceFiles/window/window_material_policy.cpp` | Decision order, refusal names and the prototype fill. |
| Create `Telegram/SourceFiles/tests/test_window_material.cpp` | Unit tests for the policy. |
| Create `Telegram/cmake/test_window_material.cmake` | Test executable target. |
| Create `Telegram/SourceFiles/platform/win/window_material_win.h` | Windows environment reader and backdrop application. |
| Create `Telegram/SourceFiles/platform/win/window_material_win.cpp` | Registry, high contrast, DWM calls and logging. |
| Create `docs/ui-modernization/tools/Test-Backdrop.ps1` | Screen-level check that the window shows what is behind it. |
| Create `docs/ui-modernization/decisions/2026-10-native-backdrop.md` | Go/no-go decision record. |
| Modify `Telegram/cmake/telegram_options.cmake` | `TDESKTOP_BUILD_MATERIAL_TESTS` option. |
| Modify `Telegram/CMakeLists.txt` | Register the new sources and include the test target. |
| Modify `Telegram/SourceFiles/window/main_window.h` | Declare the option id. |
| Modify `Telegram/SourceFiles/window/main_window.cpp` | Define the option and make the window translucent when allowed. |
| Modify `Telegram/SourceFiles/settings/settings_experimental.cpp` | List the option in experimental settings. |
| Modify `Telegram/SourceFiles/platform/win/main_window_win.cpp` | Apply the backdrop once the native window exists and on night mode changes. |
| Modify `Telegram/SourceFiles/dialogs/dialogs_widget.cpp` | Paint the translucent prototype region. |

Local, git-ignored helper scripts live in `.ai\setup\`.

---

### Task 0: Local build helpers

**Files:**
- Create: `.ai/setup/build-target.cmd` (ignored by git)
- Create: `.ai/setup/configure-material-tests.cmd` (ignored by git)

- [ ] **Step 1: Confirm `.ai` is ignored**

Run: `git check-ignore -v .ai/setup/build-target.cmd`
Expected: a line ending in `.ai/setup/build-target.cmd` that names `.gitignore`.

- [ ] **Step 2: Create `.ai/setup/build-target.cmd`**

```bat
@echo off
setlocal
set "PATH=C:\Users\artemKakun\AppData\Local\Programs\Python\Python310;C:\Users\artemKakun\AppData\Local\Programs\Python\Python310\Scripts;C:\Program Files\Git\cmd;%PATH%"
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.44
if errorlevel 1 exit /b %errorlevel%
cd /d "C:\Users\artemKakun\Projects\tdesktopWin11"
if errorlevel 1 exit /b %errorlevel%
cmake --build out --config Debug --target %1 --parallel 2 -- /p:CL_MPCount=2
exit /b %errorlevel%
```

- [ ] **Step 3: Create `.ai/setup/configure-material-tests.cmd`**

```bat
@echo off
setlocal
set "PATH=C:\Users\artemKakun\AppData\Local\Programs\Python\Python310;C:\Users\artemKakun\AppData\Local\Programs\Python\Python310\Scripts;C:\Program Files\Git\cmd;%PATH%"
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.44
if errorlevel 1 exit /b %errorlevel%
cd /d "C:\Users\artemKakun\Projects\tdesktopWin11"
if errorlevel 1 exit /b %errorlevel%
cmake -DTDESKTOP_BUILD_MATERIAL_TESTS=ON out
exit /b %errorlevel%
```

- [ ] **Step 4: Check the existing build still works**

Run (PowerShell, repository root): `cmd /c .ai\setup\build-target.cmd Telegram`
Expected: ends with `Telegram.vcxproj -> C:\Users\artemKakun\Projects\tdesktopWin11\out\Debug\Telegram.exe`. This is an incremental build of the unchanged tree; run it in the background if it takes longer than two minutes.

No commit: these files are ignored.

---

### Task 1: Material policy with unit tests

**Files:**
- Create: `Telegram/SourceFiles/tests/test_window_material.cpp`
- Create: `Telegram/cmake/test_window_material.cmake`
- Create: `Telegram/SourceFiles/window/window_material_policy.h`
- Create: `Telegram/SourceFiles/window/window_material_policy.cpp`
- Modify: `Telegram/cmake/telegram_options.cmake`
- Modify: `Telegram/CMakeLists.txt`

- [ ] **Step 1: Write the failing test**

Create `Telegram/SourceFiles/tests/test_window_material.cpp`:

```cpp
/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "window/window_material_policy.h"

#include <QtCore/QtGlobal>

#include <iostream>
#include <string>

namespace {

using namespace Window::Material;

int FailedChecks = 0;
int TotalChecks = 0;

void Check(bool condition, const char *name) {
	++TotalChecks;
	if (!condition) {
		++FailedChecks;
		std::cout << "FAILED: " << name << std::endl;
	}
}

[[nodiscard]] Environment Supported() {
	return {
		.requested = true,
		.windowsBuild = kMinimumBackdropBuild,
		.transparencyEffects = true,
		.highContrast = false,
		.nativeFrame = false,
	};
}

void TestDecide() {
	const auto accepted = Decide(Supported());
	Check(accepted.backdrop == Backdrop::Acrylic, "supported system gets acrylic");
	Check(accepted.refusal == Refusal::None, "supported system has no refusal");

	auto notRequested = Supported();
	notRequested.requested = false;
	Check(
		Decide(notRequested).refusal == Refusal::NotRequested,
		"option off refuses");

	auto old = Supported();
	old.windowsBuild = 22620;
	Check(
		Decide(old).refusal == Refusal::OldWindows,
		"build below 22621 refuses");

	auto opaque = Supported();
	opaque.transparencyEffects = false;
	Check(
		Decide(opaque).refusal == Refusal::TransparencyOff,
		"transparency effects off refuses");

	auto contrast = Supported();
	contrast.highContrast = true;
	Check(
		Decide(contrast).refusal == Refusal::HighContrast,
		"high contrast refuses");

	auto native = Supported();
	native.nativeFrame = true;
	Check(
		Decide(native).refusal == Refusal::NativeFrame,
		"native frame refuses");

	const auto empty = Decide(Environment());
	Check(empty.backdrop == Backdrop::None, "default environment refuses");
	Check(
		empty.refusal == Refusal::NotRequested,
		"the request is checked first");
}

void TestNames() {
	Check(
		std::string(RefusalName(Refusal::OldWindows)) == "old windows",
		"refusal names are readable");
	Check(
		std::string(RefusalName(Refusal::None)) == "none",
		"an accepted decision is named");
}

void TestPrototypeFill() {
	const auto fill = PrototypeFill(QColor(10, 20, 30));
	Check(
		fill.red() == 10 && fill.green() == 20 && fill.blue() == 30,
		"fill keeps the palette color");
	Check(
		qAbs(fill.alphaF() - kPrototypeFillAlpha) < 0.01,
		"fill uses the prototype alpha");
}

} // namespace

int main() {
	TestDecide();
	TestNames();
	TestPrototypeFill();
	std::cout << "Checks: " << TotalChecks
		<< ", failures: " << FailedChecks << std::endl;
	return FailedChecks ? 1 : 0;
}
```

- [ ] **Step 2: Add the test target**

Create `Telegram/cmake/test_window_material.cmake`:

```cmake
# This file is part of Telegram Desktop,
# the official desktop application for the Telegram messaging service.
#
# For license and copyright information please follow this link:
# https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL

add_executable(test_window_material)
init_target(test_window_material "(tests)")

target_include_directories(test_window_material PRIVATE ${src_loc})

nice_target_sources(test_window_material ${src_loc}
PRIVATE
    window/window_material_policy.cpp
    window/window_material_policy.h
    tests/test_window_material.cpp
)

target_link_libraries(test_window_material
PRIVATE
    desktop-app::external_qt
)

set_target_properties(test_window_material PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
```

In `Telegram/cmake/telegram_options.cmake`, after the
`TDESKTOP_BUILD_UPDATE_TESTS` line, add:

```cmake
option(TDESKTOP_BUILD_MATERIAL_TESTS "Build the window material policy test." OFF)
```

In `Telegram/CMakeLists.txt`, directly after the `endif()` that closes the
`if (DESKTOP_APP_TEST_APPS OR TDESKTOP_BUILD_UPDATE_TESTS)` block, add:

```cmake
if (DESKTOP_APP_TEST_APPS OR TDESKTOP_BUILD_MATERIAL_TESTS)
    include(cmake/test_window_material.cmake)
endif()
```

- [ ] **Step 3: Configure and confirm the test fails to build**

Run: `cmd /c .ai\setup\configure-material-tests.cmd`
Expected: CMake fails, because `window/window_material_policy.cpp` does not exist yet ("Cannot find source file").

- [ ] **Step 4: Write the policy header**

Create `Telegram/SourceFiles/window/window_material_policy.h`:

```cpp
/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include <QtGui/QColor>

namespace Window::Material {

inline constexpr auto kMinimumBackdropBuild = 22621;
inline constexpr auto kPrototypeFillAlpha = 0.6;

enum class Backdrop {
	None,
	Acrylic,
};

enum class Refusal {
	None,
	NotRequested,
	OldWindows,
	TransparencyOff,
	HighContrast,
	NativeFrame,
};

struct Environment {
	bool requested = false;
	int windowsBuild = 0;
	bool transparencyEffects = false;
	bool highContrast = false;
	bool nativeFrame = false;
};

struct Decision {
	Backdrop backdrop = Backdrop::None;
	Refusal refusal = Refusal::NotRequested;
};

[[nodiscard]] Decision Decide(const Environment &environment);
[[nodiscard]] const char *RefusalName(Refusal refusal);
[[nodiscard]] QColor PrototypeFill(QColor opaque);

} // namespace Window::Material
```

- [ ] **Step 5: Write the policy implementation**

Create `Telegram/SourceFiles/window/window_material_policy.cpp`:

```cpp
/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "window/window_material_policy.h"

namespace Window::Material {

Decision Decide(const Environment &environment) {
	const auto refuse = [](Refusal refusal) {
		return Decision{ .backdrop = Backdrop::None, .refusal = refusal };
	};
	if (!environment.requested) {
		return refuse(Refusal::NotRequested);
	} else if (environment.windowsBuild < kMinimumBackdropBuild) {
		return refuse(Refusal::OldWindows);
	} else if (!environment.transparencyEffects) {
		return refuse(Refusal::TransparencyOff);
	} else if (environment.highContrast) {
		return refuse(Refusal::HighContrast);
	} else if (environment.nativeFrame) {
		return refuse(Refusal::NativeFrame);
	}
	return { .backdrop = Backdrop::Acrylic, .refusal = Refusal::None };
}

const char *RefusalName(Refusal refusal) {
	switch (refusal) {
	case Refusal::None: return "none";
	case Refusal::NotRequested: return "not requested";
	case Refusal::OldWindows: return "old windows";
	case Refusal::TransparencyOff: return "transparency effects off";
	case Refusal::HighContrast: return "high contrast";
	case Refusal::NativeFrame: return "native frame";
	}
	return "unknown";
}

QColor PrototypeFill(QColor opaque) {
	opaque.setAlphaF(kPrototypeFillAlpha);
	return opaque;
}

} // namespace Window::Material
```

- [ ] **Step 6: Configure, build and run the test**

Run: `cmd /c .ai\setup\configure-material-tests.cmd`
Expected: ends with `-- Generating done` and `-- Build files have been written to: C:/Users/artemKakun/Projects/tdesktopWin11/out`.

Run: `cmd /c .ai\setup\build-target.cmd test_window_material`
Expected: ends with `test_window_material.vcxproj -> ...\out\Debug\test_window_material.exe`.

Run: `.\out\Debug\test_window_material.exe; $LASTEXITCODE`
Expected:

```
Checks: 13, failures: 0
0
```

- [ ] **Step 7: Prove the test can fail**

Temporarily change `kMinimumBackdropBuild` in `window_material_policy.h` to
`22000` (so build 22620 is wrongly accepted), rebuild with `cmd /c .ai\setup\build-target.cmd test_window_material`
and run `.\out\Debug\test_window_material.exe; $LASTEXITCODE`.
Expected: `FAILED: build below 22621 refuses`, then `Checks: 13, failures: 1`
and exit code `1`. Restore `22621`, rebuild and rerun; expect `failures: 0`.

- [ ] **Step 8: Commit**

```bash
git add Telegram/SourceFiles/window/window_material_policy.h Telegram/SourceFiles/window/window_material_policy.cpp Telegram/SourceFiles/tests/test_window_material.cpp Telegram/cmake/test_window_material.cmake Telegram/cmake/telegram_options.cmake Telegram/CMakeLists.txt
git commit -m "Added window material policy with tests."
```

---

### Task 2: Experimental option and Windows environment reader

System calls are not unit-testable here; their logic is covered by Task 1, and
their real values are checked through the log in Step 7.

**Files:**
- Create: `Telegram/SourceFiles/platform/win/window_material_win.h`
- Create: `Telegram/SourceFiles/platform/win/window_material_win.cpp`
- Modify: `Telegram/SourceFiles/window/main_window.h`
- Modify: `Telegram/SourceFiles/window/main_window.cpp`
- Modify: `Telegram/SourceFiles/settings/settings_experimental.cpp`
- Modify: `Telegram/CMakeLists.txt`

- [ ] **Step 1: Declare the option id**

In `Telegram/SourceFiles/window/main_window.h`, after
`extern const char kOptionDisableTouchbar[];`, add:

```cpp
extern const char kOptionWindowMaterialPrototype[];
```

- [ ] **Step 2: Define the option**

In `Telegram/SourceFiles/window/main_window.cpp`, inside the anonymous
namespace and directly after the `OptionDisableTouchbar` definition, add:

```cpp
base::options::toggle OptionWindowMaterialPrototype({
	.id = kOptionWindowMaterialPrototype,
	.name = "Window material prototype",
	.description = "Show the Windows 11 Acrylic backdrop above the chats list."
		" For testing the redesign.",
	.scope = base::options::windows,
	.restartRequired = true,
});
```

After `const char kOptionDisableTouchbar[] = "touchbar-disabled";`, add:

```cpp
const char kOptionWindowMaterialPrototype[] = "window-material-prototype";
```

- [ ] **Step 3: List the option in experimental settings**

In `Telegram/SourceFiles/settings/settings_experimental.cpp`, in the category
that contains `Window::kOptionNewWindowsSizeAsFirst,`, add after that line:

```cpp
				Window::kOptionWindowMaterialPrototype,
```

- [ ] **Step 4: Write the Windows header**

Create `Telegram/SourceFiles/platform/win/window_material_win.h`:

```cpp
/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "window/window_material_policy.h"

#include <windows.h>

namespace Platform::Material {

[[nodiscard]] Window::Material::Environment ReadEnvironment();
[[nodiscard]] Window::Material::Decision Decide();
bool ApplyBackdrop(HWND handle, bool dark);

} // namespace Platform::Material
```

- [ ] **Step 5: Write the Windows implementation**

Create `Telegram/SourceFiles/platform/win/window_material_win.cpp`:

```cpp
/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "platform/win/window_material_win.h"

#include "base/options.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "window/main_window.h"

#include <QtCore/QOperatingSystemVersion>

#include <dwmapi.h>

namespace Platform::Material {
namespace {

constexpr auto kDWMWA_USE_IMMERSIVE_DARK_MODE = DWORD(20);
constexpr auto kDWMWA_SYSTEMBACKDROP_TYPE = DWORD(38);
constexpr auto kDWMSBT_TRANSIENTWINDOW = DWORD(3);

[[nodiscard]] bool TransparencyEffectsEnabled() {
	auto value = DWORD(1);
	auto size = DWORD(sizeof(value));
	const auto result = RegGetValueW(
		HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
		L"EnableTransparency",
		RRF_RT_REG_DWORD,
		nullptr,
		&value,
		&size);
	return (result != ERROR_SUCCESS) || (value != 0);
}

[[nodiscard]] bool HighContrastEnabled() {
	auto info = HIGHCONTRASTW{ .cbSize = sizeof(HIGHCONTRASTW) };
	return SystemParametersInfoW(SPI_GETHIGHCONTRAST, info.cbSize, &info, 0)
		&& (info.dwFlags & HCF_HIGHCONTRASTON);
}

[[nodiscard]] QString Hex(HRESULT result) {
	return QString::number(quint32(result), 16);
}

} // namespace

Window::Material::Environment ReadEnvironment() {
	return {
		.requested = base::options::value<bool>(
			Window::kOptionWindowMaterialPrototype),
		.windowsBuild = QOperatingSystemVersion::current().microVersion(),
		.transparencyEffects = TransparencyEffectsEnabled(),
		.highContrast = HighContrastEnabled(),
		.nativeFrame = Core::App().settings().nativeWindowFrame(),
	};
}

Window::Material::Decision Decide() {
	const auto environment = ReadEnvironment();
	const auto result = Window::Material::Decide(environment);
	LOG(("Material: build %1, transparency %2, high contrast %3, "
		"native frame %4, refusal: %5"
		).arg(environment.windowsBuild
		).arg(Logs::b(environment.transparencyEffects)
		).arg(Logs::b(environment.highContrast)
		).arg(Logs::b(environment.nativeFrame)
		).arg(Window::Material::RefusalName(result.refusal)));
	return result;
}

bool ApplyBackdrop(HWND handle, bool dark) {
	const auto margins = MARGINS{ -1, -1, -1, -1 };
	const auto extended = DwmExtendFrameIntoClientArea(handle, &margins);
	const auto darkValue = BOOL(dark ? TRUE : FALSE);
	const auto darkened = DwmSetWindowAttribute(
		handle,
		kDWMWA_USE_IMMERSIVE_DARK_MODE,
		&darkValue,
		sizeof(darkValue));
	const auto type = kDWMSBT_TRANSIENTWINDOW;
	const auto backdrop = DwmSetWindowAttribute(
		handle,
		kDWMWA_SYSTEMBACKDROP_TYPE,
		&type,
		sizeof(type));
	LOG(("Material: extend frame 0x%1, dark mode 0x%2, backdrop 0x%3"
		).arg(Hex(extended)
		).arg(Hex(darkened)
		).arg(Hex(backdrop)));
	return SUCCEEDED(extended) && SUCCEEDED(backdrop);
}

} // namespace Platform::Material
```

- [ ] **Step 6: Register the new sources in the Telegram target**

In `Telegram/CMakeLists.txt`, in the `platform/win/` block, add after
`    platform/win/webauthn_win.cpp` and before
`    platform/win/windows_app_user_model_id.cpp`:

```cmake
    platform/win/window_material_win.cpp
    platform/win/window_material_win.h
```

In the `window/` block, add after `    window/window_main_menu_helpers.h`:

```cmake
    window/window_material_policy.cpp
    window/window_material_policy.h
```

- [ ] **Step 7: Build and check the option and log**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`
Expected: ends with `-> ...\out\Debug\Telegram.exe`, no errors.

Start `out\Debug\Telegram.exe`, open Settings > Advanced > Experimental
settings and confirm "Window material prototype" appears with its
description. Leave it off and quit Telegram.

Nothing calls `Decide()` yet, so no `Material:` log line is expected.

- [ ] **Step 8: Commit**

```bash
git add Telegram/SourceFiles/platform/win/window_material_win.h Telegram/SourceFiles/platform/win/window_material_win.cpp Telegram/SourceFiles/window/main_window.h Telegram/SourceFiles/window/main_window.cpp Telegram/SourceFiles/settings/settings_experimental.cpp Telegram/CMakeLists.txt
git commit -m "Added window material prototype option."
```

---

### Task 3: Translucent main window with Acrylic backdrop

**Files:**
- Modify: `Telegram/SourceFiles/window/main_window.cpp`
- Modify: `Telegram/SourceFiles/platform/win/main_window_win.cpp`

- [ ] **Step 1: Choose translucency before the native window exists**

In `Telegram/SourceFiles/window/main_window.cpp`, add with the other
includes:

```cpp
#ifdef Q_OS_WIN
#include "platform/win/window_material_win.h"
#endif // Q_OS_WIN
```

Inside the anonymous namespace, after the `OverridenIcon()` function, add:

```cpp
[[nodiscard]] bool UseMaterialPrototype() {
#ifdef Q_OS_WIN
	return Platform::Material::Decide().backdrop
		!= Window::Material::Backdrop::None;
#else // Q_OS_WIN
	return false;
#endif // Q_OS_WIN
}
```

Change the start of the `MainWindow::MainWindow` initializer list from:

```cpp
MainWindow::MainWindow(not_null<Controller*> controller)
: _controller(controller)
```

to:

```cpp
MainWindow::MainWindow(not_null<Controller*> controller)
: RpWindow(UseMaterialPrototype())
, _controller(controller)
```

`RpWindow(bool translucent)` already sets `Qt::WA_TranslucentBackground` and
`Qt::WA_NoSystemBackground`; the window handle is created later, so the
attribute takes effect.

- [ ] **Step 2: Apply the backdrop once the window handle exists**

In `Telegram/SourceFiles/platform/win/main_window_win.cpp`, add with the
other includes:

```cpp
#include "platform/win/window_material_win.h"
#include "ui/gl/gl_surface.h"
```

At the end of `MainWindow::initHook()`, after the `validateWindowTheme(...)`
call, add:

```cpp
	if (testAttribute(Qt::WA_TranslucentBackground)) {
		Platform::Material::ApplyBackdrop(
			_hWnd,
			Window::Theme::IsNightMode());
		Ui::GL::LogWindowRhi("material", this);
	}
```

- [ ] **Step 3: Follow night mode changes**

In `MainWindow::setupNativeWindowFrame()`, change the subscriber from:

```cpp
	) | rpl::skip(1) | rpl::on_next([=](bool native, bool night) {
		validateWindowTheme(native, night);
	}, lifetime());
```

to:

```cpp
	) | rpl::skip(1) | rpl::on_next([=](bool native, bool night) {
		validateWindowTheme(native, night);
		if (testAttribute(Qt::WA_TranslucentBackground)) {
			Platform::Material::ApplyBackdrop(_hWnd, night);
		}
	}, lifetime());
```

- [ ] **Step 4: Build**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`
Expected: ends with `-> ...\out\Debug\Telegram.exe`, no errors.

- [ ] **Step 5: Check the log with the option off**

Start `out\Debug\Telegram.exe`, wait for the chats list, quit. Then run:

```powershell
Get-ChildItem out\Debug, "$env:APPDATA\Telegram Desktop" -Filter log.txt -Recurse -ErrorAction SilentlyContinue | Sort-Object LastWriteTime | Select-Object -Last 1 | Get-Content | Select-String "Material:"
```

Expected: one line containing `refusal: not requested`, and no
`extend frame` line.

- [ ] **Step 6: Check the log with the option on**

Start Telegram, enable "Window material prototype" in experimental settings,
accept the restart, wait for the chats list, quit, and rerun the command from
Step 5.

Expected on Windows 11 22H2 or later with transparency effects on: a line
with `refusal: none`, a line `Material: extend frame 0x0, dark mode 0x0,
backdrop 0x0`, and a `QRhi-Check [material]` line describing the surface. A
non-zero code or a different refusal is a finding; record it in Task 6.

- [ ] **Step 7: Commit**

```bash
git add Telegram/SourceFiles/window/main_window.cpp Telegram/SourceFiles/platform/win/main_window_win.cpp
git commit -m "Applied Acrylic backdrop to the material prototype window."
```

---

### Task 4: Translucent prototype region above the chats list

**Files:**
- Modify: `Telegram/SourceFiles/dialogs/dialogs_widget.cpp`

- [ ] **Step 1: Paint the region with the prototype fill**

In `Telegram/SourceFiles/dialogs/dialogs_widget.cpp`, add with the other
`window/` includes:

```cpp
#include "window/window_material_policy.h"
```

In `Widget::paintEvent`, replace:

```cpp
	auto above = QRect(0, 0, width(), _scroll->y());
	if (above.intersects(r)) {
		p.fillRect(above.intersected(r), bg);
	}
```

with:

```cpp
	auto above = QRect(0, 0, width(), _scroll->y());
	if (above.intersects(r)) {
		if (window()->testAttribute(Qt::WA_TranslucentBackground)) {
			p.setCompositionMode(QPainter::CompositionMode_Source);
			p.fillRect(
				above.intersected(r),
				Window::Material::PrototypeFill(st::dialogsBg->c));
			p.setCompositionMode(QPainter::CompositionMode_SourceOver);
		} else {
			p.fillRect(above.intersected(r), bg);
		}
	}
```

`CompositionMode_Source` replaces whatever a parent painted there, so the
alpha reaches the window surface instead of blending over an opaque parent.

- [ ] **Step 2: Build and look**

Run: `cmd /c .ai\setup\build-target.cmd Telegram`
Expected: ends with `-> ...\out\Debug\Telegram.exe`.

Start Telegram with the option on and drag a brightly colored window behind
it. Expected if the prototype works: the strip around the chats search field
shows a blurred, tinted version of what is behind. Other areas stay opaque.

- [ ] **Step 3: Commit**

```bash
git add Telegram/SourceFiles/dialogs/dialogs_widget.cpp
git commit -m "Added translucent material prototype region above chats list."
```

---

### Task 5: Screen-level backdrop check

**Files:**
- Create: `docs/ui-modernization/tools/Test-Backdrop.ps1`

- [ ] **Step 1: Write the script**

Create `docs/ui-modernization/tools/Test-Backdrop.ps1`:

```powershell
# Places a solid window behind Telegram, captures Telegram's window in red
# and in blue, and reports how many pixels changed between the captures.
param(
  [ValidateSet('Transparent', 'Opaque')][string]$Expect = 'Transparent',
  [string]$OutDir = (Join-Path $env:TEMP 'tdesktop-backdrop-check'),
  [int]$Step = 3
)
Add-Type -AssemblyName System.Windows.Forms, System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class BackdropWin32 {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr hWnd, IntPtr after, int x, int y, int cx, int cy, uint flags);
}
'@
[BackdropWin32]::SetProcessDPIAware() | Out-Null

$process = Get-Process Telegram -ErrorAction SilentlyContinue |
  Where-Object { $_.Path -like '*tdesktopWin11*' -and $_.MainWindowHandle -ne 0 } |
  Select-Object -First 1
if (-not $process) { Write-Error 'Start out\Debug\Telegram.exe first.'; exit 2 }
$telegram = $process.MainWindowHandle
$rect = New-Object BackdropWin32+RECT
[BackdropWin32]::GetWindowRect($telegram, [ref]$rect) | Out-Null
$width = $rect.Right - $rect.Left
$height = $rect.Bottom - $rect.Top

$form = New-Object System.Windows.Forms.Form
$form.FormBorderStyle = 'None'
$form.ShowInTaskbar = $false
$form.StartPosition = 'Manual'
$form.Bounds = New-Object System.Drawing.Rectangle(($rect.Left - 40), ($rect.Top - 40), ($width + 80), ($height + 80))
$form.BackColor = [System.Drawing.Color]::Red
$form.Show()
$SWP_NOACTIVATE = 0x0010
[BackdropWin32]::SetWindowPos($form.Handle, $telegram, $form.Left, $form.Top, $form.Width, $form.Height, $SWP_NOACTIVATE) | Out-Null
[BackdropWin32]::SetForegroundWindow($telegram) | Out-Null

function Capture([string]$name) {
  [System.Windows.Forms.Application]::DoEvents()
  Start-Sleep -Milliseconds 900
  $bitmap = New-Object System.Drawing.Bitmap($width, $height)
  $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
  $graphics.CopyFromScreen($rect.Left, $rect.Top, 0, 0, $bitmap.Size)
  $graphics.Dispose()
  $bitmap.Save((Join-Path $OutDir "$name.png"))
  return $bitmap
}

New-Item -ItemType Directory -Force $OutDir | Out-Null
$red = Capture 'behind-red'
$form.BackColor = [System.Drawing.Color]::Blue
$form.Refresh()
$blue = Capture 'behind-blue'
$form.Close()

$changed = 0; $total = 0
for ($y = 0; $y -lt $height; $y += $Step) {
  for ($x = 0; $x -lt $width; $x += $Step) {
    $a = $red.GetPixel($x, $y); $b = $blue.GetPixel($x, $y)
    $total++
    if ([math]::Abs($a.R - $b.R) + [math]::Abs($a.G - $b.G) + [math]::Abs($a.B - $b.B) -gt 30) { $changed++ }
  }
}
$red.Dispose(); $blue.Dispose()
$fraction = $changed / [math]::Max(1, $total)
"Changed pixels: $changed of $total sampled ({0:P2}). Captures: $OutDir" -f $fraction
if ($Expect -eq 'Transparent') { if ($fraction -gt 0.002) { 'PASS: backdrop shows what is behind the window'; exit 0 } else { 'FAIL: window is opaque'; exit 1 } }
else { if ($fraction -le 0.002) { 'PASS: window is opaque'; exit 0 } else { 'FAIL: window shows what is behind it'; exit 1 } }
```

- [ ] **Step 2: Run it with the option off**

Start `out\Debug\Telegram.exe` with "Window material prototype" off and
leave it in a normal, non-maximized state. Run:

`powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tools\Test-Backdrop.ps1 -Expect Opaque`

Expected: `PASS: window is opaque` and exit code `0`. The check of the script
itself: the window is unchanged, so this proves the script does not report
false transparency.

- [ ] **Step 3: Run it with the option on**

Enable the option, let Telegram restart, then run:

`powershell -NoProfile -ExecutionPolicy Bypass -File docs\ui-modernization\tools\Test-Backdrop.ps1 -Expect Transparent`

Expected if the prototype works: `PASS: backdrop shows what is behind the
window`. Open `behind-red.png` and `behind-blue.png` in the reported folder
and confirm the change is in the strip above the chats list, not in random
places. A `FAIL` here is a valid prototype result; record it in Task 6.

- [ ] **Step 4: Commit**

```bash
git add docs/ui-modernization/tools/Test-Backdrop.ps1
git commit -m "Added screen-level backdrop check for the material prototype."
```

---

### Task 6: Run the prototype matrix and record the decision

**Files:**
- Create: `docs/ui-modernization/decisions/2026-10-native-backdrop.md`
- Modify: `docs/ui-modernization/README.md`

- [ ] **Step 1: Create the decision record**

Create `docs/ui-modernization/decisions/2026-10-native-backdrop.md` with this
content, then fill every "Result" cell while running Steps 2–4:

```markdown
# Native backdrop prototype results

Date: (date of the run). Build: Debug, commit (output of `git rev-parse --short HEAD`).
Windows: (output of `cmd /c ver`). GPU: (adapter name from the `QRhi` log line).

## Log

Paste the `Material:` and `QRhi-Check [material]` lines from `log.txt`.

## Matrix

| Check | How | Result |
| --- | --- | --- |
| Option off is unchanged | `Test-Backdrop.ps1 -Expect Opaque` |  |
| Backdrop visible | `Test-Backdrop.ps1 -Expect Transparent` |  |
| Matches the mockup | Put a similar blue wallpaper behind the window and compare the strip with `docs/ui-modernization/explorations/soft-panels-readable-light.png` and `-dark.png`: similar tint, blur and visibility of the desktop |  |
| Light and dark | Toggle night mode; backdrop tint follows |  |
| Maximized | Maximize, rerun the transparent check |  |
| Snapped | Snap left with Win+Left, look at the strip |  |
| Full screen | Enter and leave full screen; no flicker or black area |  |
| Inactive window | Click another app; note how the strip looks |  |
| DPI change | Move between 100% and 150% monitors, or change scale |  |
| Text quality | Zoom a screenshot of the search placeholder; ClearType or grayscale |  |
| Holes | Any area other than the strip that became see-through or black |  |
| Scrolling cost | Scroll the chats list fast; compare smoothness with the option off |  |
| Video | Play a video message in a chat; check it draws correctly |  |
| Calls panel | Open a call panel or group call window if available |  |
| Transparency effects off | Windows Settings > Personalization > Colors; restart Telegram; log shows `transparency effects off` |  |

## Decision

Go, go with conditions, or no-go, with the reasons from the matrix. For
no-go, name which alternative from the roadmap's risk table to try next.
```

The parenthesized fields are the values to copy from the run; replace each
one with the observed value.

- [ ] **Step 2: Run the checks with the option on**

Work through every matrix row in order, writing what you observed in the
"Result" column: "pass", or the exact symptom such as "black rectangle behind
the title bar when maximized".

- [ ] **Step 3: Run the transparency-effects-off row**

Turn off Windows Settings > Personalization > Colors > Transparency effects,
restart Telegram and check the log shows `refusal: transparency effects off`
and that `Test-Backdrop.ps1 -Expect Opaque` passes. Turn transparency effects
back on.

- [ ] **Step 4: Write the decision**

Fill in "Decision". Go requires: the backdrop check passes, no holes, and
maximized and full-screen states draw correctly. Anything else is a
conditional go or a no-go.

The mockup is the target and Acrylic only the working guideline. If the
"Matches the mockup" row fails while everything else passes, compare Mica:
temporarily set `kDWMSBT_TRANSIENTWINDOW` in `window_material_win.cpp` to
`DWORD(2)` (the Mica value), rebuild, repeat that row, record both results
under "Decision", and restore `DWORD(3)` without committing the change.

- [ ] **Step 5: Link the record from the design docs**

In `docs/ui-modernization/README.md`, under "Working documents", add:

```markdown
- [Native backdrop prototype results](decisions/2026-10-native-backdrop.md).
```

Convert the new Markdown file to CRLF line endings:

```powershell
$p = 'docs\ui-modernization\decisions\2026-10-native-backdrop.md'; $t = [IO.File]::ReadAllText($p) -replace "`r?`n", "`r`n"; [IO.File]::WriteAllText($p, $t, (New-Object Text.UTF8Encoding $false))
```

- [ ] **Step 6: Commit**

```bash
git add docs/ui-modernization/decisions/2026-10-native-backdrop.md docs/ui-modernization/README.md
git commit -m "Recorded native backdrop prototype results."
```

---

## Done when

- `test_window_material.exe` reports `failures: 0`.
- With the option off, `Test-Backdrop.ps1 -Expect Opaque` passes and the app
  looks unchanged.
- The decision record is complete, and its decision determines whether
  Phase 2 of the roadmap starts as planned or with an alternative.
