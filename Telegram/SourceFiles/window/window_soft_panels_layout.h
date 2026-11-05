/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "base/basic_types.h"
#include <QtCore/QRect>

class QPainterPath;

namespace Window::SoftPanels {

struct LayoutArgs {
	QRect main;
	int dialogsWidth = 0;
	int thirdWidth = 0;
	bool oneColumn = false;
	int gap = 0;
};

struct Geometry {
	QRect dialogs;
	QRect conversation;
	QRect third;
};

[[nodiscard]] Geometry Layout(const LayoutArgs &args);

enum class Corners {
	All,
	Right,
};

[[nodiscard]] QPainterPath PanelShape(
	QRectF rect,
	float64 radius,
	Corners corners);

} // namespace Window::SoftPanels
