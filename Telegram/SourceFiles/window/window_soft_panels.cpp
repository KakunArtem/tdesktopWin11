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
#include "ui/vertical_list.h"
#include "ui/widgets/box_content_divider.h"
#include "ui/widgets/separate_panel.h"
#include "ui/widgets/soft_controls.h"

#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include "styles/style_info.h"
#include "styles/style_layers.h"
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

constexpr auto kCardFillAlpha = 0.045;
constexpr auto kCardBorderAlpha = 0.08;

[[nodiscard]] QRect VisibleRect(
		not_null<QWidget*> widget,
		not_null<QWidget*> root) {
	auto result = widget->rect();
	for (auto w = widget.get(); w != root; w = w->parentWidget()) {
		const auto parent = w->parentWidget();
		if (!parent || w->isHidden()) {
			return QRect();
		}
		result = result.translated(w->pos()).intersected(parent->rect());
		if (result.isEmpty()) {
			return QRect();
		}
	}
	return result;
}

[[nodiscard]] std::vector<QRect> CollectCards(not_null<QWidget*> content) {
	struct Cut {
		int top = 0;
		int bottom = 0;
	};
	auto cuts = std::vector<Cut>();
	const auto &padding = st::defaultSubsectionTitlePadding;
	for (const auto child : content->findChildren<QWidget*>()) {
		if (dynamic_cast<Ui::BoxContentDivider*>(child)
			|| child->property(kCardGapProperty).toBool()) {
			const auto r = VisibleRect(child, content);
			if (!r.isEmpty()) {
				cuts.push_back({ r.y(), r.y() + r.height() });
			}
		} else if (child->property(Ui::kCardTitleProperty).toBool()) {
			const auto r = VisibleRect(child, content);
			if (!r.isEmpty()) {
				cuts.push_back({
					r.y() - padding.top(),
					r.y() + r.height() + st::settingsCardTitleSkip,
				});
			}
		}
	}
	ranges::sort(cuts, ranges::less(), &Cut::top);

	auto result = std::vector<QRect>();
	auto top = 0;
	const auto add = [&](int bottom) {
		if (bottom - top >= st::settingsCardMinHeight) {
			result.push_back(QRect(0, top, content->width(), bottom - top));
		}
	};
	for (const auto &cut : cuts) {
		if (cut.top > top) {
			add(cut.top);
		}
		top = std::max(top, cut.bottom);
	}
	add(content->height());
	return result;
}

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

void SetupCards(
		not_null<QWidget*> host,
		not_null<Ui::RpWidget*> content,
		CardsArgs args) {
	content->setProperty(Ui::kCardsProperty, true);

	const auto cards = Ui::CreateChild<Ui::RpWidget>(host.get());
	cards->setAttribute(Qt::WA_TransparentForMouseEvents);
	cards->setGeometry(host->rect());
	cards->lower();
	cards->show();

	content->geometryValue(
	) | rpl::on_next([=] {
		cards->setGeometry(host->rect());
		cards->update();
	}, cards->lifetime());

	cards->paintRequest(
	) | rpl::on_next([=] {
		auto p = QPainter(cards);
		auto hq = PainterHighQualityEnabler(p);
		auto fill = st::windowFg->c;
		fill.setAlphaF(kCardFillAlpha);
		auto border = st::windowFg->c;
		border.setAlphaF(kCardBorderAlpha);
		p.setPen(QPen(border, st::lineWidth));
		p.setBrush(fill);
		const auto radius = float64(Ui::PopupPanelRadius());
		const auto half = st::lineWidth / 2.;
		const auto shift = (content.get() == host.get())
			? QPoint()
			: content->mapTo(host, QPoint());
		const auto margins = QMarginsF(
			args.inset + half,
			args.shrink + half,
			args.inset + half,
			args.shrink + half);
		for (const auto &card : CollectCards(content)) {
			p.drawRoundedRect(
				QRectF(card.translated(shift)).marginsRemoved(margins),
				radius,
				radius);
		}
	}, cards->lifetime());
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
