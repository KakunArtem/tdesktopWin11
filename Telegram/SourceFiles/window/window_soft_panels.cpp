/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "window/window_soft_panels.h"

#include "base/options.h"
#include "styles/palette.h"
#include "ui/style/style_core_palette.h"
#include "ui/ui_material.h"

#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include "styles/style_info.h"
#include "styles/style_window_soft_panels.h"

namespace Window::SoftPanels {
namespace {

auto ContentGrabDepth = 0;

base::options::toggle LayoutOption({
	.id = kOptionLayout,
	.name = "Soft Panels layout",
	.description = "Rounded panels and gaps in the main window.",
	.scope = base::options::windows,
	.restartRequired = true,
});

} // namespace

const char kOptionLayout[] = "soft-panels-layout";

void Initialize() {
	Ui::Material::SetSoftPanels(LayoutOption.value());
}

QPixmap GrabContent(not_null<QWidget*> widget, QRect rect) {
	++ContentGrabDepth;
	const auto restore = gsl::finally([] {
		--ContentGrabDepth;
	});
	return Ui::Material::Grab(widget, rect);
}

int TopBarHeight() {
	return Ui::Material::SoftPanels()
		? st::softPanelsTopBarHeight
		: st::topBarHeight;
}

int TopBarShift() {
	return (TopBarHeight() - st::topBarHeight) / 2;
}

int TopBarShadowHeight() {
	return Ui::Material::SoftPanels() ? 0 : st::lineWidth;
}

PanelFrame::PanelFrame(not_null<QWidget*> parent, Corners corners)
: RpWidget(parent)
, _corners(corners) {
	setAttribute(Qt::WA_TransparentForMouseEvents);
	Ui::Material::RepaintOnFillsChanged(this);
	style::PaletteChanged() | rpl::on_next([=] {
		update();
	}, lifetime());
}

void PanelFrame::setPanelGeometry(QRect panel) {
	setGeometry(panel);
}

void PanelFrame::resizeEvent(QResizeEvent *event) {
	const auto radius = (_corners == Corners::Right)
		? st::softPanelsNavigationRadius
		: st::softPanelsRadius;
	const auto bounds = QRectF(rect());
	auto outside = QPainterPath();
	outside.addRect(bounds);
	_outside = outside.subtracted(PanelShape(bounds, radius, _corners));
	const auto inset = st::softPanelsBorderWidth / 2.;
	_outline = PanelShape(
		bounds.adjusted(inset, inset, -inset, -inset),
		std::max(radius - inset, 0.),
		_corners);
	_inner = rect().marginsRemoved(QMargins(radius, radius, radius, radius));
}

void PanelFrame::paintEvent(QPaintEvent *event) {
	if (ContentGrabDepth || _inner.contains(event->rect())) {
		return;
	}
	auto p = QPainter(this);
	p.setClipRegion(event->region());
	p.setRenderHint(QPainter::Antialiasing);
	Ui::Material::PaintSurface(
		p,
		_outside,
		Ui::Material::Surface::Window,
		st::windowBg);
	p.setPen(QPen(
		Ui::Material::Active()
			? Ui::Material::Border(Ui::Material::Surface::Window)
			: st::shadowFg->c,
		st::softPanelsBorderWidth));
	p.setBrush(Qt::NoBrush);
	p.drawPath(_outline);
}

} // namespace Window::SoftPanels
