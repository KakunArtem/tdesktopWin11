/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "ui/ui_material.h"

#include "base/assertion.h"
#include "ui/qt_object_factory.h"
#include "ui/ui_utility.h"
#include "rpl/variable.h"
#include "rpl/event_stream.h"

#include <QtCore/QPointer>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>

namespace Ui::Material {
namespace {

auto ActiveState = rpl::variable<bool>(false);
auto Fills = base::flat_map<Surface, QColor>();
auto Borders = base::flat_map<Surface, QColor>();
auto FillChanges = rpl::event_stream<>();
auto GrabDepth = 0;
auto CurrentRevision = uint64(0);

constexpr auto kBindingProperty = "_td_material_binding";

struct WidgetBinding {
	bool opaqueInSolidMode = false;
	bool opaqueBound = false;
	bool repaintBound = false;
	rpl::lifetime lifetime;
};

[[nodiscard]] WidgetBinding *Binding(not_null<QWidget*> widget) {
	auto result = static_cast<WidgetBinding*>(
		widget->property(kBindingProperty).value<void*>());
	if (!result) {
		result = Ui::CreateChild<WidgetBinding>(widget.get());
		widget->setProperty(kBindingProperty,
			QVariant::fromValue(static_cast<void*>(result)));
	}
	return result;
}

} // namespace

bool Active() {
	return ActiveState.current();
}

uint64 Revision() {
	return CurrentRevision;
}

rpl::producer<bool> ActiveValue() {
	return ActiveState.value();
}

void SetActive(bool active) {
	if (ActiveState.current() != active) {
		++CurrentRevision;
		ActiveState = active;
	}
}

void SetFills(
		base::flat_map<Surface, QColor> fills,
		base::flat_map<Surface, QColor> borders) {
	Fills = std::move(fills);
	Borders = std::move(borders);
	if (Active()) {
		++CurrentRevision;
	}
	FillChanges.fire({});
}

QColor Fill(Surface surface) {
	const auto i = Fills.find(surface);
	return (i != Fills.end()) ? i->second : QColor();
}

QColor Border(Surface surface) {
	const auto i = Borders.find(surface);
	return (i != Borders.end()) ? i->second : QColor();
}

rpl::producer<> FillsChanged() {
	return FillChanges.events();
}

GrabScope::GrabScope() {
	++GrabDepth;
}

GrabScope::~GrabScope() {
	Expects(GrabDepth > 0);
	--GrabDepth;
}

bool Grabbing() {
	return GrabDepth > 0;
}

QPixmap Grab(not_null<QWidget*> widget, QRect rect) {
	if (!Active()) {
		return Ui::GrabWidget(widget, rect);
	}
	const auto scope = GrabScope();
	const auto opaque = widget->testAttribute(Qt::WA_OpaquePaintEvent);
	const auto weak = QPointer<QWidget>(widget.get());
	const auto restore = gsl::finally([=] {
		if (opaque && weak) {
			const auto binding = static_cast<WidgetBinding*>(
				weak->property(kBindingProperty).value<void*>());
			weak->setAttribute(Qt::WA_OpaquePaintEvent,
				(binding && binding->opaqueBound)
					? binding->opaqueInSolidMode && !Active()
					: true);
		}
	});
	if (opaque) {
		widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
	}
	return Ui::GrabWidget(widget, rect, QColor(0, 0, 0, 0));
}

QPixmap GrabOpaque(
		not_null<QWidget*> widget,
		QRect rect,
		const style::color &solid) {
	return Active()
		? Grab(widget, rect)
		: Ui::GrabOpaque(widget, rect, solid->c);
}

void PaintAnimationBackground(
		QPainter &p,
		const QRect &rect,
		Surface surface,
		const style::color &solid) {
	PaintSurface(p, rect, surface, solid);
}

void PaintFill(QPainter &p, const QRect &rect, const QColor &fill) {
	const auto mode = p.compositionMode();
	p.setCompositionMode(QPainter::CompositionMode_Source);
	p.fillRect(rect, fill);
	p.setCompositionMode(mode);
}

void PaintFill(QPainter &p, const QPainterPath &shape, const QColor &fill) {
	p.save();
	p.setCompositionMode(QPainter::CompositionMode_Source);
	p.setRenderHint(QPainter::Antialiasing);
	p.fillPath(shape, fill);
	p.restore();
}

void PaintSurface(
		QPainter &p,
		const QRect &rect,
		Surface surface,
		const style::color &solid) {
	if (Active()) {
		PaintFill(p, rect, Grabbing() ? QColor(0, 0, 0, 0) : Fill(surface));
	} else {
		p.fillRect(rect, solid);
	}
}

void PaintSurface(
		QPainter &p,
		const QPainterPath &shape,
		Surface surface,
		const style::color &solid) {
	if (Active()) {
		PaintFill(p, shape, Grabbing() ? QColor(0, 0, 0, 0) : Fill(surface));
	} else {
		p.fillPath(shape, solid);
	}
}

void SetOpaquePaint(not_null<QWidget*> widget, bool opaqueInSolidMode) {
	const auto binding = Binding(widget);
	binding->opaqueInSolidMode = opaqueInSolidMode;
	if (!binding->opaqueBound) {
		binding->opaqueBound = true;
		const auto weak = QPointer<QWidget>(widget.get());
		ActiveValue() | rpl::on_next([=](bool active) {
			if (weak) {
				weak->setAttribute(Qt::WA_OpaquePaintEvent,
					binding->opaqueInSolidMode && !active);
			}
		}, binding->lifetime);
	} else {
		widget->setAttribute(Qt::WA_OpaquePaintEvent,
			opaqueInSolidMode && !Active());
	}
}

void RepaintOnFillsChanged(not_null<QWidget*> widget) {
	const auto binding = Binding(widget);
	if (binding->repaintBound) {
		return;
	}
	binding->repaintBound = true;
	const auto weak = QPointer<QWidget>(widget.get());
	FillsChanged() | rpl::on_next([=] {
		if (weak) {
			weak->update();
		}
	}, binding->lifetime);
}

} // namespace Ui::Material
