/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "rpl/producer.h"

#include <QtCore/QByteArray>

namespace Window::Material {

[[nodiscard]] bool SettingAvailable();
[[nodiscard]] bool SettingEnabled();
void SetSettingEnabled(bool enabled);

[[nodiscard]] bool ChatBackgroundEnabled();
void SetChatBackgroundEnabled(bool enabled);
[[nodiscard]] bool ChatBackgroundActive();
[[nodiscard]] rpl::producer<bool> ChatBackgroundActiveValue();

[[nodiscard]] bool Capable();
[[nodiscard]] bool Active();
[[nodiscard]] rpl::producer<bool> ActiveValue();
void Refresh();
void ReportBackdropFailure();
void PaletteReplaced();
[[nodiscard]] QByteArray UnadjustedPalette();

} // namespace Window::Material
