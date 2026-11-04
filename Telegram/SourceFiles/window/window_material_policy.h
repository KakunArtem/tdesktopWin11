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
