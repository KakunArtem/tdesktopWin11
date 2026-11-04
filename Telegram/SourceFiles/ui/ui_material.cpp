/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "ui/ui_material.h"

#include "rpl/variable.h"

namespace Ui::Material {
namespace {

auto ActiveState = rpl::variable<bool>(false);

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

} // namespace Ui::Material
