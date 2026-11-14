/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "ui/ui_material.h"
#include "base/basic_types.h"

#include <QtGui/QColor>

namespace Window::Material {

using Ui::Material::Surface;

struct FillInputs {
	QColor surface;
	QColor text;
	QColor secondaryText;
	bool dark = false;
};

struct Adjustment {
	QColor original;
	QColor adjusted;
};

struct BackdropRange {
	QColor darkest;
	QColor brightest;
};

struct ChatColorInputs {
	QColor barFill;
	QColor barBorder;
	QColor text;
	QColor unreadText;
	QColor accent;
	bool dark = false;
};

struct ChatColors {
	QColor serviceBg;
	QColor serviceBgSelected;
	QColor serviceFg;
	QColor unreadBg;
	QColor unreadBorder;
	QColor unreadFg;
};

inline constexpr auto kDefaultTransparency = 0.5;

[[nodiscard]] float64 Floor(Surface surface, bool dark);

// WHY: 0 is solid, kDefaultTransparency keeps Floor() as is, 1 halves it.
// Popups keep their own floor so menus stay readable over anything.
void SetTransparency(float64 value);
[[nodiscard]] float64 Transparency();
[[nodiscard]] float64 EffectiveFloor(Surface surface, bool dark);
[[nodiscard]] BackdropRange AcrylicRange(bool dark);
[[nodiscard]] QColor Composite(const QColor &over, const QColor &under);
[[nodiscard]] QColor ComputeFill(Surface surface, const FillInputs &inputs);
[[nodiscard]] QColor BorderColor(const QColor &text);
[[nodiscard]] ChatColors ComputeChatColors(const ChatColorInputs &inputs);
[[nodiscard]] QColor ReadableOver(
	const QColor &text,
	const QColor &surface,
	Surface role,
	bool dark,
	float64 target = 4.5);

} // namespace Window::Material
