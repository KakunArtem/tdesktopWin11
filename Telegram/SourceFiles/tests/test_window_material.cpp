/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "window/window_material_policy.h"
#include "window/window_material_fills.h"
#include "ui/ui_material.h"
#include "ui/color_contrast.h"

#include "rpl/lifetime.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

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

void TestLiveAllows() {
	Check(LiveAllows(Supported()), "supported system allows live");

	auto opaque = Supported();
	opaque.transparencyEffects = false;
	Check(!LiveAllows(opaque), "transparency off stops live material");

	auto contrast = Supported();
	contrast.highContrast = true;
	Check(!LiveAllows(contrast), "high contrast stops live material");

	auto notRequested = Supported();
	notRequested.requested = false;
	Check(LiveAllows(notRequested), "the request is a startup decision");
}

void TestMaterialMirror() {
	Check(!Ui::Material::Active(), "UI material starts inactive");
	auto values = std::vector<bool>();
	auto lifetime = rpl::lifetime();
	Ui::Material::ActiveValue() | rpl::on_next([&](bool active) {
		values.push_back(active);
	}, lifetime);
	Ui::Material::SetActive(true);
	Check(Ui::Material::Active(), "UI material publishes activation");
	Ui::Material::SetActive(true);
	Ui::Material::SetActive(false);
	Check(!Ui::Material::Active(), "UI material publishes fallback");
	Check(
		values == std::vector<bool>{ false, true, false },
		"UI material replays current state and suppresses duplicates");
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

[[nodiscard]] bool Near(float64 a, float64 b) {
	return std::abs(a - b) < 0.005;
}

void TestMaterialFills() {
	const auto light = FillInputs{
		.surface = QColor(0xe9, 0xee, 0xf5),
		.text = QColor(0x13, 0x23, 0x35),
		.secondaryText = QColor(0x3a, 0x4b, 0x5f),
	};
	Check(Near(ComputeFill(Surface::Navigation, light).alphaF(), 0.78),
		"mockup light navigation fill is 0.78");
	Check(Near(ComputeFill(Surface::Window, light).alphaF(), 0.78),
		"window floor is raised until text is readable");
	const auto dark = FillInputs{
		.surface = QColor(0x1b, 0x28, 0x38),
		.text = QColor(0xf2, 0xf6, 0xfb),
		.secondaryText = QColor(0xc2, 0xcf, 0xdd),
		.dark = true,
	};
	const auto fill = ComputeFill(Surface::Navigation, dark);
	Check(Near(fill.alphaF(), 0.82), "dark navigation keeps its floor");
	Check(fill.rgb() == dark.surface.rgb(), "fill keeps surface color");
	const auto weak = FillInputs{
		.surface = QColor(255, 255, 255),
		.text = QColor(0, 0, 0),
		.secondaryText = QColor(0x99, 0x99, 0x99),
	};
	Check(Near(ComputeFill(Surface::Navigation, weak).alphaF(), 1.),
		"weak solid text needs opaque fill");
	const auto unreadable = FillInputs{
		.surface = QColor(128, 128, 128),
		.text = QColor(128, 128, 128),
		.secondaryText = QColor(128, 128, 128),
	};
	Check(Near(ComputeFill(Surface::Bar, unreadable).alphaF(), 1.),
		"unreadable custom theme falls back to opaque");
	const auto translucentText = FillInputs{
		.surface = QColor(255, 255, 255),
		.text = QColor(0, 0, 0, 128),
		.secondaryText = QColor(0, 0, 0, 128),
	};
	Check(Near(ComputeFill(Surface::Navigation, translucentText).alphaF(), 1.),
		"text alpha participates in the contrast target");
	const auto roles = std::array{
		Surface::Window, Surface::Navigation, Surface::Conversation,
		Surface::Bar, Surface::Composer, Surface::Popup,
	};
	const auto floors = std::array{
		std::pair(0.72, 0.74), std::pair(0.78, 0.82),
		std::pair(0.79, 0.81), std::pair(0.90, 0.90),
		std::pair(0.96, 0.96), std::pair(0.97, 0.97),
	};
	for (auto i = 0; i != int(roles.size()); ++i) {
		Check(Near(Floor(roles[i], false), floors[i].first), "light floor");
		Check(Near(Floor(roles[i], true), floors[i].second), "dark floor");
	}
	const auto border = BorderColor(light.text);
	Check(border.rgb() == light.text.rgb() && Near(border.alphaF(), 0.10),
		"border is primary text at ten percent");
	Check(Composite(QColor(0, 0, 0, 128), QColor(255, 255, 255))
		== QColor(127, 127, 127), "straight alpha composite rounds channels");
}

void TestReadableOver() {
	const auto surface = QColor(255, 255, 255);
	const auto grey = QColor(0x99, 0x99, 0x99);
	const auto readable = ReadableOver(grey, surface, Surface::Navigation, false);
	auto fill = surface;
	fill.setAlphaF(Floor(Surface::Navigation, false));
	Check(Ui::CountContrast(readable, Composite(fill, QColor(0, 0, 0))) >= 4.5
		&& Ui::CountContrast(readable, Composite(fill, QColor(255, 255, 255))) >= 4.5,
		"adjusted text reads over both navigation backdrops");
	Check(readable.lightness() < grey.lightness(), "light theme text darkens");
	const auto strong = QColor(0x13, 0x23, 0x35);
	Check(ReadableOver(strong, QColor(0xe9, 0xee, 0xf5), Surface::Navigation, false)
		== strong, "already readable text is unchanged");
}

void TestMaterialFillMirror() {
	Ui::Material::SetActive(true);
	auto count = 0;
	auto lifetime = rpl::lifetime();
	Ui::Material::FillsChanged() | rpl::on_next([&] {
		++count;
	}, lifetime);
	auto fill = QColor(233, 238, 245);
	fill.setAlphaF(0.78);
	const auto border = BorderColor(QColor(19, 35, 53));
	Ui::Material::SetFills({ { Surface::Navigation, fill } },
		{ { Surface::Navigation, border } });
	Check(Ui::Material::Fill(Surface::Navigation) == fill,
		"UI mirror publishes the material fill");
	Check(Ui::Material::Border(Surface::Navigation) == border,
		"UI mirror publishes border");
	Ui::Material::SetActive(false);
	fill.setAlpha(255);
	Ui::Material::SetFills({ { Surface::Navigation, fill } },
		{ { Surface::Navigation, border } });
	Check(Ui::Material::Fill(Surface::Navigation) == fill,
		"UI mirror publishes opaque fallback");
	Check(count == 2, "each completed fill publication notifies once");
}

} // namespace

int main() {
	TestDecide();
	TestLiveAllows();
	TestMaterialMirror();
	TestNames();
	TestPrototypeFill();
	TestMaterialFills();
	TestReadableOver();
	TestMaterialFillMirror();
	std::cout << "Checks: " << TotalChecks
		<< ", failures: " << FailedChecks << std::endl;
	return FailedChecks ? 1 : 0;
}
