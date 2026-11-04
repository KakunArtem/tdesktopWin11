/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "window/window_material_policy.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

using namespace Window::Material;

int FailedChecks = 0;
int TotalChecks = 0;

void Check(bool condition, const char *name) {
	++TotalChecks;
	if (!condition) {
		++FailedChecks;
		std::cout << "FAILED: " << name << std::endl;
	}
}

[[nodiscard]] Environment Supported() {
	return {
		.requested = true,
		.windowsBuild = kMinimumBackdropBuild,
		.transparencyEffects = true,
		.highContrast = false,
		.nativeFrame = false,
	};
}

void TestDecide() {
	const auto accepted = Decide(Supported());
	Check(accepted.backdrop == Backdrop::Acrylic, "supported system gets acrylic");
	Check(accepted.refusal == Refusal::None, "supported system has no refusal");

	auto notRequested = Supported();
	notRequested.requested = false;
	Check(
		Decide(notRequested).refusal == Refusal::NotRequested,
		"option off refuses");

	auto old = Supported();
	old.windowsBuild = 22620;
	Check(
		Decide(old).refusal == Refusal::OldWindows,
		"build below 22621 refuses");

	auto opaque = Supported();
	opaque.transparencyEffects = false;
	Check(
		Decide(opaque).refusal == Refusal::TransparencyOff,
		"transparency effects off refuses");

	auto contrast = Supported();
	contrast.highContrast = true;
	Check(
		Decide(contrast).refusal == Refusal::HighContrast,
		"high contrast refuses");

	auto native = Supported();
	native.nativeFrame = true;
	Check(
		Decide(native).refusal == Refusal::NativeFrame,
		"native frame refuses");

	const auto empty = Decide(Environment());
	Check(empty.backdrop == Backdrop::None, "default environment refuses");
	Check(
		empty.refusal == Refusal::NotRequested,
		"the request is checked first");
}

void TestNames() {
	Check(
		std::string(RefusalName(Refusal::OldWindows)) == "old windows",
		"refusal names are readable");
	Check(
		std::string(RefusalName(Refusal::None)) == "none",
		"an accepted decision is named");
}

void TestPrototypeFill() {
	const auto fill = PrototypeFill(QColor(10, 20, 30));
	Check(
		fill.red() == 10 && fill.green() == 20 && fill.blue() == 30,
		"fill keeps the palette color");
	Check(
		std::abs(fill.alphaF() - kPrototypeFillAlpha) < 0.01,
		"fill uses the prototype alpha");
}

} // namespace

int main() {
	TestDecide();
	TestNames();
	TestPrototypeFill();
	std::cout << "Checks: " << TotalChecks
		<< ", failures: " << FailedChecks << std::endl;
	return FailedChecks ? 1 : 0;
}
