/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "window/window_material_policy.h"

namespace Window::Material {

Decision Decide(const Environment &environment) {
	const auto refuse = [](Refusal refusal) {
		return Decision{ .backdrop = Backdrop::None, .refusal = refusal };
	};
	if (!environment.requested) {
		return refuse(Refusal::NotRequested);
	} else if (environment.windowsBuild < kMinimumBackdropBuild) {
		return refuse(Refusal::OldWindows);
	} else if (!environment.transparencyEffects) {
		return refuse(Refusal::TransparencyOff);
	} else if (environment.highContrast) {
		return refuse(Refusal::HighContrast);
	} else if (environment.nativeFrame) {
		return refuse(Refusal::NativeFrame);
	}
	return { .backdrop = Backdrop::Acrylic, .refusal = Refusal::None };
}

bool LiveAllows(const Environment &environment) {
	return environment.transparencyEffects && !environment.highContrast;
}

const char *RefusalName(Refusal refusal) {
	switch (refusal) {
	case Refusal::None: return "none";
	case Refusal::NotRequested: return "not requested";
	case Refusal::OldWindows: return "old windows";
	case Refusal::TransparencyOff: return "transparency effects off";
	case Refusal::HighContrast: return "high contrast";
	case Refusal::NativeFrame: return "native frame";
	}
	return "unknown";
}

QColor PrototypeFill(QColor opaque) {
	opaque.setAlphaF(kPrototypeFillAlpha);
	return opaque;
}

} // namespace Window::Material
