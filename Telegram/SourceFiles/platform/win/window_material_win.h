/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "window/window_material_policy.h"

#include <windows.h>

namespace style {
struct WindowTitle;
} // namespace style

namespace Ui {
class RpWindow;
} // namespace Ui

namespace Platform::Material {

[[nodiscard]] bool Supported();
[[nodiscard]] bool BackdropDark();
[[nodiscard]] const style::WindowTitle &TitleStyle(
	bool material,
	bool softPanels);
void SetupWindowMaterial(not_null<Ui::RpWindow*> window);
[[nodiscard]] Window::Material::Environment ReadEnvironment();
[[nodiscard]] Window::Material::Decision Decide();
bool ApplyBackdrop(HWND handle, bool dark);

} // namespace Platform::Material
