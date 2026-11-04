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
