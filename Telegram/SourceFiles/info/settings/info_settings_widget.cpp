/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "info/settings/info_settings_widget.h"

#include "info/info_memento.h"
#include "settings/sections/settings_main.h"
#include "settings/sections/settings_information.h"
#include "settings/settings_common_session.h"
#include "menu/menu_send.h"
#include "ui/chat/chat_style_radius.h"
#include "ui/widgets/box_content_divider.h"
#include "ui/widgets/labels.h"
#include "ui/painter.h"
#include "ui/ui_material.h"
#include "ui/ui_utility.h"
#include "ui/vertical_list.h"
#include "styles/style_info.h"
#include "styles/style_layers.h"

namespace Info {
namespace Settings {
namespace {

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
		if (dynamic_cast<Ui::BoxContentDivider*>(child)) {
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

void SetupCards(not_null<QWidget*> wrap, not_null<Ui::RpWidget*> content) {
	content->setProperty(Ui::kCardsProperty, true);

	const auto cards = Ui::CreateChild<Ui::RpWidget>(wrap.get());
	cards->setAttribute(Qt::WA_TransparentForMouseEvents);
	cards->setGeometry(wrap->rect());
	cards->lower();
	cards->show();

	content->geometryValue(
	) | rpl::on_next([=] {
		cards->setGeometry(wrap->rect());
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
		const auto shift = content->pos();
		for (const auto &card : CollectCards(content)) {
			p.drawRoundedRect(
				QRectF(card.translated(shift)).marginsRemoved(
					{ half, half, half, half }),
				radius,
				radius);
		}
	}, cards->lifetime());
}

} // namespace

Memento::Memento(not_null<UserData*> self, Type type)
: ContentMemento(Tag{ self })
, _type(type) {
}

Section Memento::section() const {
	return Section(_type);
}

object_ptr<ContentWidget> Memento::createWidget(
		QWidget *parent,
		not_null<Controller*> controller,
		const QRect &geometry) {
	auto result = object_ptr<Widget>(
		parent,
		controller);
	result->setInternalState(geometry, this);
	return result;
}

Memento::~Memento() = default;

Widget::Widget(
	QWidget *parent,
	not_null<Controller*> controller)
: ContentWidget(parent, controller)
, _self(controller->key().settingsSelf())
, _type(controller->section().settingsType())
, _inner(setupFlexibleInnerWidget(
		_type->create(
			this,
			controller->parentController(),
			scroll(),
			controller->wrapValue(
			) | rpl::map([](Wrap wrap) { return (wrap == Wrap::Layer)
				? ::Settings::Container::Layer
				: ::Settings::Container::Section; })),
		_flexibleScroll,
		[=](Ui::RpWidget*) {
			controller->stepDataReference() = SectionCustomTopBarData{
				.backButtonEnables = _flexibleScroll.backButtonEnables.events(),
				.wrapValue = controller->wrapValue(),
			};
		}))
, _pinnedToTop(_inner->createPinnedToTop(this))
, _pinnedToBottom(_inner->createPinnedToBottom(this)) {
	if (Ui::Material::SoftPanels() && !_inner->hasFlexibleTopBar()) {
		setInnerSideInset(st::settingsCardInset);
		SetupCards(innerWrap(), _inner);
	}

	_inner->sectionShowOther(
	) | rpl::on_next([=](Type type) {
		controller->showSettings(type);
	}, _inner->lifetime());

	_inner->sectionShowBack(
	) | rpl::on_next([=] {
		controller->showBackFromStack();
	}, _inner->lifetime());

	_inner->setStepDataReference(controller->stepDataReference());

	_removesFromStack.events(
	) | rpl::on_next([=](const std::vector<Type> &types) {
		const auto sections = ranges::views::all(
			types
		) | ranges::views::transform([](Type type) {
			return Section(type);
		}) | ranges::to_vector;
		controller->removeFromStack(sections);
	}, _inner->lifetime());

	if (_pinnedToTop) {
		_inner->widthValue(
		) | rpl::on_next([=](int w) {
			_pinnedToTop->resizeToWidth(w);
			setScrollTopSkip(_pinnedToTop->height());
		}, _pinnedToTop->lifetime());

		_pinnedToTop->heightValue(
		) | rpl::on_next([=](int h) {
			setScrollTopSkip(h);
		}, _pinnedToTop->lifetime());
	}

	if (_pinnedToBottom) {
		const auto processHeight = [=] {
			setScrollBottomSkip(_pinnedToBottom->height());
			_pinnedToBottom->moveToLeft(
				_pinnedToBottom->x(),
				height() - _pinnedToBottom->height());
		};

		_inner->sizeValue(
		) | rpl::on_next([=](const QSize &s) {
			_pinnedToBottom->resizeToWidth(s.width());
			//processHeight();
		}, _pinnedToBottom->lifetime());

		rpl::combine(
			_pinnedToBottom->heightValue(),
			heightValue()
		) | rpl::on_next(processHeight, _pinnedToBottom->lifetime());
	}

	if (_pinnedToTop
		&& _pinnedToTop->minimumHeight()
		&& _inner->hasFlexibleTopBar()) {
		const auto heightDiff = [=] {
			return _pinnedToTop->maximumHeight()
				- _pinnedToTop->minimumHeight();
		};

		rpl::combine(
			_pinnedToTop->heightValue(),
			_inner->heightValue()
		) | rpl::on_next([=](int, int h) {
			_flexibleScroll.contentHeightValue.fire(h + heightDiff());
		}, _pinnedToTop->lifetime());

		scrollTopValue(
		) | rpl::on_next([=](int top) {
			if (!_pinnedToTop) {
				return;
			}
			const auto current = heightDiff() - top;
			_inner->moveToLeft(0, std::min(0, current));
			_pinnedToTop->resize(
				_pinnedToTop->width(),
				std::max(current + _pinnedToTop->minimumHeight(), 0));
		}, _inner->lifetime());

		_flexibleScroll.fillerWidthValue.events(
		) | rpl::on_next([=](int w) {
			_inner->resizeToWidth(w);
		}, _inner->lifetime());

		setPaintPadding({ 0, _pinnedToTop->minimumHeight(), 0, 0 });

		setViewport(_pinnedToTop->events(
		) | rpl::filter([](not_null<QEvent*> e) {
			return e->type() == QEvent::Wheel;
		}));
	}
}

Widget::~Widget() = default;

not_null<UserData*> Widget::self() const {
	return _self;
}

bool Widget::showInternal(not_null<ContentMemento*> memento) {
	//if (const auto myMemento = dynamic_cast<Memento*>(memento.get())) {
	//	Assert(myMemento->self() == self());

	//	if (_inner->showInternal(myMemento)) {
	//		return true;
	//	}
	//}
	return false;
}

void Widget::setInternalState(
		const QRect &geometry,
		not_null<Memento*> memento) {
	setGeometry(geometry);
	Ui::SendPendingMoveResizeEvents(this);
	restoreState(memento);
}

void Widget::saveChanges(FnMut<void()> done) {
	_inner->sectionSaveChanges(std::move(done));
}

SendMenu::Details Widget::sendMenuDetails() const {
	return _inner->sendMenuDetails();
}

bool Widget::processChosenSticker(ChatHelpers::FileChosen &&chosen) {
	return _inner->processChosenSticker(std::move(chosen));
}

void Widget::showFinished() {
	_inner->showFinished();

	_inner->removeFromStack(
	) | rpl::start_to_stream(_removesFromStack, lifetime());
}

void Widget::setInnerFocus() {
	_inner->setInnerFocus();
}

const Ui::RoundRect *Widget::bottomSkipRounding() const {
	return _inner->bottomSkipRounding();
}

rpl::producer<bool> Widget::desiredShadowVisibility() const {
	return (_type == ::Settings::MainId()
		|| _type == ::Settings::InformationId())
		? ContentWidget::desiredShadowVisibility()
		: rpl::single(true);
}

bool Widget::closeByOutsideClick() const {
	return _inner->closeByOutsideClick();;
}

void Widget::checkBeforeClose(Fn<void()> close) {
	_inner->checkBeforeClose(std::move(close));
}

void Widget::checkBeforeCloseByEscape(Fn<void()> close) {
	ContentWidget::checkBeforeCloseByEscape([&] {
		_inner->checkBeforeClose(std::move(close));
	});
}

rpl::producer<QString> Widget::title() {
	return _inner->title();
}

rpl::producer<QString> Widget::titleBadge() {
	return _inner->titleBadge();
}

void Widget::paintEvent(QPaintEvent *e) {
	if (!_inner->paintOuter(this, maxVisibleHeight(), e->rect())) {
		ContentWidget::paintEvent(e);
	}
}

std::shared_ptr<ContentMemento> Widget::doCreateMemento() {
	auto result = std::make_shared<Memento>(self(), _type);
	saveState(result.get());
	return result;
}

void Widget::enableBackButton() {
	_flexibleScroll.backButtonEnables.fire({});
}

rpl::producer<SelectedItems> Widget::selectedListValue() const {
	return _inner->selectedListValue();
}

void Widget::selectionAction(SelectionAction action) {
	_inner->selectionAction(action);
}

void Widget::fillTopBarMenu(const Ui::Menu::MenuCallback &addAction) {
	_inner->fillTopBarMenu(addAction);
}

void Widget::saveState(not_null<Memento*> memento) {
	memento->setScrollTop(scrollTopSave());
	auto sectionState = std::any();
	_inner->sectionSaveState(sectionState);
	memento->setSectionState(std::move(sectionState));
}

void Widget::restoreState(not_null<Memento*> memento) {
	_inner->sectionRestoreState(memento->sectionState());
	scrollTopRestore(memento->scrollTop());
}

} // namespace Settings
} // namespace Info
