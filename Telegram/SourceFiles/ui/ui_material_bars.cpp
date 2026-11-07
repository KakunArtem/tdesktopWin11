/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "ui/ui_material_bars.h"

#include "ui/rp_widget.h"
#include "ui/ui_utility.h"
#include "styles/palette.h"
#include "styles/style_window_soft_panels.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>

namespace Ui::Material {
namespace {

[[nodiscard]] const style::color &RowSolid(Surface surface) {
	return (surface == Surface::Navigation) ? st::dialogsBg : st::windowBg;
}

[[nodiscard]] QPainterPath PillShape(const QRect &pill) {
	const auto radius = float64(st::softPanelsBarRadius);
	auto result = QPainterPath();
	result.addRoundedRect(QRectF(pill), radius, radius);
	return result;
}

void PaintPillOutline(QPainter &p, const QRect &pill) {
	PaintSurfaceOutline(p, pill, st::softPanelsBarRadius, Surface::Bar);
}

} // namespace

QMargins BarMargins() {
	return SoftPanels() ? st::softPanelsBarMargins : QMargins();
}

QRect BarPillRect(const QRect &row) {
	return row.marginsRemoved(BarMargins());
}

int BarContentWidth(int rowWidth) {
	const auto margins = BarMargins();
	return rowWidth - margins.left() - margins.right();
}

int BarRowHeight(int contentHeight) {
	return contentHeight + BarMargins().bottom();
}

QRect BarShadowGeometry(QRect regular) {
	if (SoftPanels()) {
		regular.setHeight(0);
	}
	return regular;
}

void PaintBar(
		QPainter &p,
		const QRect &row,
		const QRect &clip,
		const style::color &solid,
		Surface rowSurface) {
	if (!SoftPanels()) {
		p.fillRect(clip, solid);
		return;
	}
	PaintSurface(p, clip, rowSurface, RowSolid(rowSurface));
	const auto pill = BarPillRect(row);
	if (pill.isEmpty()) {
		return;
	}
	const auto shape = PillShape(pill);
	if (Active()) {
		// A section grab keeps the pill, the panel under it is repainted.
		PaintFill(p, shape, Fill(Surface::Bar));
	} else {
		p.save();
		p.setRenderHint(QPainter::Antialiasing);
		p.fillPath(shape, solid);
		p.restore();
	}
	PaintPillOutline(p, pill);
}

void PaintBarContent(
		QPainter &p,
		const QRect &rect,
		const style::color &solid) {
	if (!SoftPanels()) {
		p.fillRect(rect, solid);
	}
}

not_null<RpWidget*> CreateBarContent(not_null<RpWidget*> row) {
	if (!SoftPanels()) {
		return row;
	}
	RepaintOnFillsChanged(row);
	const auto result = CreateChild<RpWidget>(row.get());
	result->show();
	row->sizeValue() | rpl::on_next([=](QSize size) {
		result->setGeometry(BarPillRect(QRect(QPoint(), size)));
	}, result->lifetime());
	return result;
}

void AddBarFrame(not_null<RpWidget*> row) {
	if (!SoftPanels()) {
		return;
	}
	const auto frame = CreateChild<RpWidget>(row.get());
	frame->setAttribute(Qt::WA_TransparentForMouseEvents);
	frame->show();
	RepaintOnFillsChanged(frame);
	row->sizeValue() | rpl::on_next([=](QSize size) {
		frame->setGeometry(QRect(QPoint(), size));
		frame->raise();
	}, frame->lifetime());
	frame->paintRequest() | rpl::on_next([=] {
		auto p = QPainter(frame);
		const auto pill = BarPillRect(frame->rect());
		auto outside = QPainterPath();
		outside.addRect(QRectF(frame->rect()));
		if (!pill.isEmpty()) {
			outside = outside.subtracted(PillShape(pill));
		}
		p.setRenderHint(QPainter::Antialiasing);
		PaintSurface(p, outside, Surface::Conversation, st::windowBg);
		if (!pill.isEmpty()) {
			PaintPillOutline(p, pill);
		}
	}, frame->lifetime());
}

void PaintSurfaceOutline(
		QPainter &p,
		const QRect &rect,
		int radius,
		Surface surface) {
	const auto width = float64(st::softPanelsBorderWidth);
	const auto inset = width / 2.;
	const auto inner = std::max(radius - inset, 0.);
	p.save();
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(QPen(Active() ? Border(surface) : st::shadowFg->c, width));
	p.setBrush(Qt::NoBrush);
	p.drawRoundedRect(
		QRectF(rect).adjusted(inset, inset, -inset, -inset),
		inner,
		inner);
	p.restore();
}

void PaintRoundedSurface(
		QPainter &p,
		const QRect &rect,
		int radius,
		Surface surface,
		const style::color &solid) {
	if (rect.isEmpty()) {
		return;
	}
	auto shape = QPainterPath();
	shape.addRoundedRect(QRectF(rect), radius, radius);
	p.save();
	p.setRenderHint(QPainter::Antialiasing);
	PaintSurface(p, shape, surface, solid);
	p.restore();
	if (!Grabbing()) {
		PaintSurfaceOutline(p, rect, radius, surface);
	}
}

} // namespace Ui::Material
