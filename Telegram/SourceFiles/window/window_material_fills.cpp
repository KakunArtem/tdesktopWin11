/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "window/window_material_fills.h"

#include "ui/color_contrast.h"
#include "ui/chat/chat_theme_readability.h"
#include "base/assertion.h"

#include <cmath>
#include <vector>

namespace Window::Material {
namespace {

[[nodiscard]] float64 ContrastOver(
		const QColor &text,
		const QColor &background) {
	return Ui::CountContrast(Composite(text, background), background);
}

} // namespace

float64 Floor(Surface surface, bool dark) {
	switch (surface) {
	case Surface::Window: return dark ? 0.50 : 0.45;
	case Surface::Navigation: return dark ? 0.60 : 0.52;
	case Surface::Conversation: return dark ? 0.60 : 0.52;
	case Surface::Bar: return 0.90;
	case Surface::Composer: return 0.96;
	case Surface::Popup: return 0.97;
	}
	Unexpected("Invalid material surface.");
}

BackdropRange AcrylicRange(bool dark) {
	return dark
		? BackdropRange{ QColor(54, 54, 54), QColor(169, 169, 169) }
		: BackdropRange{ QColor(132, 132, 132), QColor(255, 255, 255) };
}

QColor Composite(const QColor &over, const QColor &under) {
	const auto alpha = over.alphaF();
	const auto channel = [&](int above, int below) {
		return int(std::lround(above * alpha + below * (1. - alpha)));
	};
	return QColor(
		channel(over.red(), under.red()),
		channel(over.green(), under.green()),
		channel(over.blue(), under.blue()));
}

QColor ComputeFill(Surface surface, const FillInputs &inputs) {
	auto fill = inputs.surface;
	fill.setAlpha(255);
	const auto primary = ContrastOver(inputs.text, fill);
	const auto secondary = ContrastOver(inputs.secondaryText, fill);
	if (primary <= 1. || secondary <= 1.) {
		return fill;
	}
	const auto acceptable = [&](const QColor &background) {
		return ContrastOver(inputs.text, background) >= std::min(4.5, primary)
			&& ContrastOver(inputs.secondaryText, background)
				>= std::min(4.5, secondary);
	};
	const auto range = AcrylicRange(inputs.dark);
	const auto first = int(std::lround(Floor(surface, inputs.dark) * 100));
	for (auto step = first; step <= 100; ++step) {
		fill.setAlphaF(step / 100.);
		if (acceptable(Composite(fill, range.darkest))
			&& acceptable(Composite(fill, range.brightest))) {
			return fill;
		}
	}
	fill.setAlpha(255);
	return fill;
}

QColor BorderColor(const QColor &text) {
	auto result = text;
	result.setAlphaF(0.10);
	return result;
}

ChatColors ComputeChatColors(const ChatColorInputs &inputs) {
	const auto blend = [](int from, int to) {
		return int(std::lround(from + (to - from) * 0.15));
	};
	const auto &bar = inputs.barFill;
	const auto selected = QColor(
		blend(bar.red(), inputs.accent.red()),
		blend(bar.green(), inputs.accent.green()),
		blend(bar.blue(), inputs.accent.blue()),
		bar.alpha());
	const auto range = AcrylicRange(inputs.dark);
	const auto backgrounds = std::vector<QColor>{
		Composite(bar, range.darkest),
		Composite(bar, range.brightest),
		Composite(selected, range.darkest),
		Composite(selected, range.brightest),
	};
	return {
		.serviceBg = bar,
		.serviceBgSelected = selected,
		.serviceFg = Ui::EnsureTextContrast(inputs.text, backgrounds, 4.5),
		.unreadBg = bar,
		.unreadBorder = inputs.barBorder,
		.unreadFg = Ui::EnsureTextContrast(
			inputs.unreadText,
			backgrounds,
			4.5),
	};
}

QColor ReadableOver(
		const QColor &text,
		const QColor &surface,
		Surface role,
		bool dark,
		float64 target) {
	auto fill = surface;
	fill.setAlphaF(Floor(role, dark));
	const auto range = AcrylicRange(dark);
	return Ui::EnsureTextContrast(text, {
		Composite(fill, range.darkest),
		Composite(fill, range.brightest),
	}, target);
}

} // namespace Window::Material
