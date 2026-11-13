/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "window/window_soft_panels.h"

#include "base/options.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "styles/palette.h"
#include "ui/style/style_core_palette.h"
#include "ui/chat/chat_style_radius.h"
#include "ui/chat/chat_theme_readability.h"
#include "ui/layers/layer_widget.h"
#include "ui/painter.h"
#include "ui/ui_material.h"
#include "ui/widgets/separate_panel.h"
#include "ui/widgets/soft_controls.h"

#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include "styles/style_info.h"
#include "styles/style_window_main_menu.h"
#include "styles/style_window_soft_panels.h"

namespace Window::SoftPanels {
namespace {

constexpr auto kSoftControlHoverLight = 0.06;
constexpr auto kSoftControlHoverDark = 0.07;

auto ContentGrabDepth = 0;
auto SoftControlsLifetime = rpl::lifetime();

constexpr auto kLayoutPref = std::string_view("soft_panels_layout");

// WHY: the experimental option is no longer listed; it stays registered so
// a value stored by an older build is read once into the setting.
base::options::toggle LayoutOption({
	.id = kOptionLayout,
	.name = "Soft Panels layout",
	.description = "Rounded panels and gaps in the main window.",
	.defaultValue = true,
	.scope = base::options::windows,
	.restartRequired = true,
});

} // namespace

const char kOptionLayout[] = "soft-panels-layout";

bool LayoutEnabled() {
	auto &settings = Core::App().settings();
	const auto stored = settings.readPref<bool>(kLayoutPref, false);
	if (stored != settings.readPref<bool>(kLayoutPref, true)) {
		const auto migrated = LayoutOption.value();
		LOG(("Soft Panels: migrated layout setting %1."
			).arg(Logs::b(migrated)));
		settings.writePref<bool>(kLayoutPref, migrated);
		Core::App().saveSettingsDelayed();
		return migrated;
	}
	return stored;
}

void SetLayoutEnabled(bool enabled) {
	Core::App().settings().writePref<bool>(kLayoutPref, enabled);
	Core::App().saveSettingsDelayed();
}

void Initialize() {
	Ui::Material::SetSoftPanels(LayoutEnabled());
	RefreshSoftControls();
	style::PaletteChanged() | rpl::on_next([] {
		RefreshSoftControls();
	}, SoftControlsLifetime);
}

void RefreshSoftControls() {
	Ui::SetSoftControls(Ui::Material::SoftPanels());
	Ui::SetMainMenuCornerRadius(Ui::Material::SoftPanels()
		? st::mainMenuSoftPanelsRadius
		: 0);
	Ui::SetSeparatePanelRadius(Ui::Material::SoftPanels()
		? Ui::PopupPanelRadius()
		: 0);
	if (Ui::SoftControls()) {
		auto hover = st::windowFg->c;
		hover.setAlphaF(Ui::IsDarkPalette(*style::main_palette::get())
			? kSoftControlHoverDark
			: kSoftControlHoverLight);
		Ui::SetSoftControlHover(hover);
	}
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

void PaintCard(QPainter &p, QRect card) {
	using Ui::Material::Surface;
	const auto radius = Ui::PopupPanelRadius();
	auto shape = QPainterPath();
	shape.addRoundedRect(QRectF(card), radius, radius);
	auto hq = PainterHighQualityEnabler(p);
	Ui::Material::PaintSurface(p, shape, Surface::Popup, st::windowBg);
	const auto inset = st::lineWidth / 2.;
	auto outline = QPainterPath();
	outline.addRoundedRect(
		QRectF(card).adjusted(inset, inset, -inset, -inset),
		radius - inset,
		radius - inset);
	p.setPen(QPen(
		(Ui::Material::Active()
			? Ui::Material::Border(Surface::Popup)
			: st::shadowFg->c),
		st::lineWidth));
	p.setBrush(Qt::NoBrush);
	p.drawPath(outline);
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
