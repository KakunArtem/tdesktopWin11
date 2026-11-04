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
