/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "rpl/producer.h"
#include "base/flat_map.h"

#include <QtGui/QColor>

namespace Ui::Material {

enum class Surface {
	Window,
	Navigation,
	Conversation,
	Bar,
	Composer,
	Popup,
};

[[nodiscard]] bool Active();
[[nodiscard]] rpl::producer<bool> ActiveValue();
void SetActive(bool active);
void SetFills(
	base::flat_map<Surface, QColor> fills,
	base::flat_map<Surface, QColor> borders);
[[nodiscard]] QColor Fill(Surface surface);
[[nodiscard]] QColor Border(Surface surface);
[[nodiscard]] rpl::producer<> FillsChanged();

} // namespace Ui::Material
