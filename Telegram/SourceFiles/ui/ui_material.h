/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "rpl/producer.h"

namespace Ui::Material {

[[nodiscard]] bool Active();
[[nodiscard]] rpl::producer<bool> ActiveValue();
void SetActive(bool active);

} // namespace Ui::Material
