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

[[nodiscard]] float64 Floor(Surface surface, bool dark);
[[nodiscard]] BackdropRange AcrylicRange(bool dark);
[[nodiscard]] QColor Composite(const QColor &over, const QColor &under);
[[nodiscard]] QColor ComputeFill(Surface surface, const FillInputs &inputs);
[[nodiscard]] QColor BorderColor(const QColor &text);
[[nodiscard]] QColor ReadableOver(
	const QColor &text,
	const QColor &surface,
	Surface role,
	bool dark,
	float64 target = 4.5);

} // namespace Window::Material
