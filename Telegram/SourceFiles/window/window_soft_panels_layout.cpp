/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "window/window_soft_panels_layout.h"

#include <algorithm>
#include <QtGui/QPainterPath>

namespace Window::SoftPanels {

Geometry Layout(const LayoutArgs &args) {
	if (args.main.isEmpty()) {
		return {};
	} else if (args.oneColumn) {
		return { args.main, args.main, {} };
	}
	const auto width = args.main.width();
	const auto dialogs = std::clamp(args.dialogsWidth, 0, width);
	const auto gap = std::max(args.gap, 0);
	const auto firstGap = dialogs ? std::min(gap, width - dialogs) : 0;
	const auto remaining = width - dialogs - firstGap;
	const auto third = std::clamp(args.thirdWidth, 0, remaining);
	const auto thirdGap = third ? std::min(gap, remaining - third) : 0;
	const auto conversation = remaining - third - thirdGap;
	const auto left = args.main.x();
	const auto top = args.main.y();
	const auto height = args.main.height();
	return {
		dialogs ? QRect(left, top, dialogs, height) : QRect(),
		QRect(left + dialogs + firstGap, top, conversation, height),
		third ? QRect(left + width - third, top, third, height) : QRect(),
	};
}

QPainterPath PanelShape(QRectF rect, float64 radius, Corners corners) {
	auto result = QPainterPath();
	if (rect.isEmpty()) {
		return result;
	}
	radius = std::clamp(radius, 0., std::min(rect.width(), rect.height()) / 2.);
	if (!radius) {
		result.addRect(rect);
	} else if (corners == Corners::All) {
		result.addRoundedRect(rect, radius, radius);
	} else {
		const auto diameter = 2 * radius;
		const auto topRight = QRectF(
			rect.right() - diameter,
			rect.top(),
			diameter,
			diameter);
		const auto bottomRight = QRectF(
			rect.right() - diameter,
			rect.bottom() - diameter,
			diameter,
			diameter);
		result.moveTo(rect.topLeft());
		result.lineTo(rect.right() - radius, rect.top());
		result.arcTo(topRight, 90, -90);
		result.lineTo(rect.right(), rect.bottom() - radius);
		result.arcTo(bottomRight, 0, -90);
		result.lineTo(rect.bottomLeft());
		result.closeSubpath();
	}
	return result;
}

} // namespace Window::SoftPanels
