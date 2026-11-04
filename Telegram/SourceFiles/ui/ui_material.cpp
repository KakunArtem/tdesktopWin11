/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "ui/ui_material.h"

#include "rpl/variable.h"
#include "rpl/event_stream.h"

namespace Ui::Material {
namespace {

auto ActiveState = rpl::variable<bool>(false);
auto Fills = base::flat_map<Surface, QColor>();
auto Borders = base::flat_map<Surface, QColor>();
auto FillChanges = rpl::event_stream<>();

} // namespace

bool Active() {
	return ActiveState.current();
}

rpl::producer<bool> ActiveValue() {
	return ActiveState.value();
}

void SetActive(bool active) {
	ActiveState = active;
}

void SetFills(
		base::flat_map<Surface, QColor> fills,
		base::flat_map<Surface, QColor> borders) {
	Fills = std::move(fills);
	Borders = std::move(borders);
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

} // namespace Ui::Material
