/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "ui/ui_material.h"

#include <QtCore/QMargins>

namespace Ui {
class RpWidget;
} // namespace Ui

namespace Ui::Material {

[[nodiscard]] QMargins BarMargins();
[[nodiscard]] QRect BarPillRect(const QRect &row);
[[nodiscard]] int BarContentWidth(int rowWidth);
[[nodiscard]] int BarRowHeight(int contentHeight);
[[nodiscard]] QRect BarShadowGeometry(QRect regular);
void PaintBar(
	QPainter &p,
	const QRect &row,
	const QRect &clip,
	const style::color &solid,
	Surface rowSurface = Surface::Conversation);
void PaintBarContent(
	QPainter &p,
	const QRect &rect,
	const style::color &solid);
[[nodiscard]] not_null<RpWidget*> CreateBarContent(not_null<RpWidget*> row);
void AddBarFrame(not_null<RpWidget*> row);

} // namespace Ui::Material
