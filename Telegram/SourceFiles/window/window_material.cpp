/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "window/window_material.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "ui/ui_material.h"
#include "rpl/variable.h"

#include <optional>

#ifdef Q_OS_WIN
#include "platform/win/window_material_win.h"
#endif // Q_OS_WIN

namespace Window::Material {
namespace {

auto StartupCapability = std::optional<bool>();
auto ActiveState = rpl::variable<bool>(false);
auto StateLifetime = rpl::lifetime();

} // namespace

bool SettingAvailable() {
#ifdef Q_OS_WIN
	return Platform::Material::Supported();
#else // Q_OS_WIN
	return false;
#endif // Q_OS_WIN
}

bool SettingEnabled() {
	return Core::App().settings().readPref<bool>("window_material", false);
}

void SetSettingEnabled(bool enabled) {
	Core::App().settings().writePref<bool>("window_material", enabled);
	Core::App().saveSettingsDelayed();
}

bool Capable() {
	if (!StartupCapability.has_value()) {
#ifdef Q_OS_WIN
		StartupCapability = Platform::Material::Decide().backdrop
			== Backdrop::Acrylic;
#else // Q_OS_WIN
		StartupCapability = false;
#endif // Q_OS_WIN
		ActiveState.value() | rpl::on_next([](bool active) {
			Ui::Material::SetActive(active);
		}, StateLifetime);
#ifdef Q_OS_WIN
		Core::App().settings().nativeWindowFrameChanges(
		) | rpl::on_next([](bool) {
			Refresh();
		}, StateLifetime);
#endif // Q_OS_WIN
		Refresh();
	}
	return *StartupCapability;
}

bool Active() {
	return Capable() && ActiveState.current();
}

rpl::producer<bool> ActiveValue() {
	if (!Capable()) {
		return rpl::single(false);
	}
	return ActiveState.value();
}

void Refresh() {
	if (!StartupCapability.has_value()) {
		return;
	}
#ifdef Q_OS_WIN
	const auto environment = Platform::Material::ReadEnvironment();
	ActiveState = *StartupCapability
		&& LiveAllows(environment)
		&& !environment.nativeFrame;
#else // Q_OS_WIN
	ActiveState = false;
#endif // Q_OS_WIN
}

void ReportBackdropFailure() {
	if (!Capable()) {
		return;
	}
	StartupCapability = false;
	LOG(("Material: backdrop failure; using solid rendering until restart."));
	Refresh();
}

} // namespace Window::Material
