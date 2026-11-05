/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "window/window_material_policy.h"
#include "window/window_material_fills.h"
#include "ui/ui_material.h"
#include "ui/ui_utility.h"
#include "ui/color_contrast.h"
#include "ui/style/style_core_color.h"

#include "rpl/lifetime.h"
#include "rpl/never.h"

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

#include <cmath>
#include <iostream>
#include <string>
#include <type_traits>
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

void TestPaintFillReplaces() {
	auto image = QImage(8, 8, QImage::Format_ARGB32_Premultiplied);
	image.fill(QColor(255, 0, 0));
	{
		auto p = QPainter(&image);
		p.setCompositionMode(QPainter::CompositionMode_Multiply);
		for (auto i = 0; i != 5; ++i) {
			Ui::Material::PaintFill(p, image.rect(), QColor(0, 0, 255, 128));
		}
		Check(p.compositionMode() == QPainter::CompositionMode_Multiply,
			"fill restores the painter composition mode");
	}
	Check(image.pixelColor(3, 3) == QColor(0, 0, 255, 128),
		"repeated fills replace alpha and color");
	image.fill(QColor(255, 0, 0));
	{
		auto p = QPainter(&image);
		p.setClipRect(QRect(2, 2, 2, 2));
		Ui::Material::PaintFill(p, image.rect(), QColor(0, 0, 255, 128));
	}
	Check(image.pixelColor(0, 0) == QColor(255, 0, 0),
		"fill respects the existing clip");
	Check(image.pixelColor(2, 2) == QColor(0, 0, 255, 128),
		"fill covers the clipped region");
}

void TestShapeAndSurfaceFill() {
	auto image = QImage(12, 12, QImage::Format_ARGB32_Premultiplied);
	image.fill(Qt::transparent);
	auto shape = QPainterPath();
	shape.addRoundedRect(QRectF(2, 2, 8, 8), 3, 3);
	{
		auto p = QPainter(&image);
		Ui::Material::PaintFill(p, shape, QColor(10, 20, 30, 128));
		Check(!p.testRenderHint(QPainter::Antialiasing),
			"shape fill restores antialiasing state");
	}
	Check(image.pixelColor(6, 6).alpha() == 128,
		"path interior has the requested alpha");
	Check(image.pixelColor(0, 0).alpha() == 0,
		"path leaves outside pixels untouched");
	const auto solid = style::internal::OwnedColor(QColor(40, 50, 60));
	const auto material = QColor(10, 20, 30, 128);
	Ui::Material::SetFills({ { Surface::Navigation, material } }, {});
	for (const auto active : { false, true }) {
		Ui::Material::SetActive(active);
		image.fill(Qt::transparent);
		{
			auto p = QPainter(&image);
			Ui::Material::PaintSurface(p, image.rect(),
				Surface::Navigation, solid.color());
		}
		const auto expected = (active ? material : solid.color()->c).rgba();
		Check(image.pixelColor(6, 6).rgba() == expected,
			"surface chooses material or the exact solid color");
		image.fill(Qt::transparent);
		{
			auto p = QPainter(&image);
			Ui::Material::PaintSurface(p, shape,
				Surface::Navigation, solid.color());
		}
		Check(image.pixelColor(6, 6).rgba() == expected,
			"path surface chooses material or solid");
	}
	Ui::Material::SetActive(false);
}

void TestWidgetMaterialBindings() {
	auto widget = QWidget();
	Ui::Material::SetOpaquePaint(&widget, true);
	Check(widget.testAttribute(Qt::WA_OpaquePaintEvent), "solid widget opaque");
	Ui::Material::SetActive(true);
	Check(!widget.testAttribute(Qt::WA_OpaquePaintEvent), "material clears opaque");
	Ui::Material::SetOpaquePaint(&widget, false);
	Ui::Material::SetActive(false);
	Check(!widget.testAttribute(Qt::WA_OpaquePaintEvent),
		"fallback preserves animation's non-opaque intent");
	Ui::Material::SetOpaquePaint(&widget, true);
	const auto children = widget.children().size();
	Ui::Material::SetOpaquePaint(&widget, true);
	Ui::Material::RepaintOnFillsChanged(&widget);
	Ui::Material::RepaintOnFillsChanged(&widget);
	Check(widget.children().size() == children,
		"repeated binding reuses the widget's state");
	Check(widget.testAttribute(Qt::WA_OpaquePaintEvent),
		"animation end restores solid opacity");
	auto temporary = new QWidget();
	Ui::Material::SetOpaquePaint(temporary, true);
	Ui::Material::RepaintOnFillsChanged(temporary);
	delete temporary;
	Ui::Material::SetActive(true);
	Ui::Material::SetFills({ { Surface::Navigation, QColor(1, 2, 3, 128) } }, {});
	Ui::Material::SetActive(false);
}

class CaptureWidget final : public QWidget {
public:
	explicit CaptureWidget(const style::color &solid) : _solid(solid) {
		resize(8, 8);
		setAttribute(Qt::WA_NoSystemBackground);
	}

	Fn<void()> beforePaint;
	bool observedScope = false;

protected:
	void paintEvent(QPaintEvent *) override {
		auto p = QPainter(this);
		observedScope = Ui::Material::Grabbing();
		if (beforePaint) {
			beforePaint();
		}
		p.fillRect(rect(), QColor(90, 80, 70));
		Ui::Material::PaintSurface(p, rect(), Surface::Navigation, _solid);
		p.fillRect(QRect(2, 2, 2, 2), Qt::red);
	}

private:
	const style::color &_solid;

};

void TestGrabScopeNests() {
	using Ui::Material::GrabScope;
	using Ui::Material::Grabbing;
	static_assert(!std::is_copy_constructible_v<GrabScope>);
	static_assert(!std::is_move_constructible_v<GrabScope>);
	Check(!Grabbing(), "not grabbing initially");
	{
		auto outer = GrabScope();
		Check(Grabbing(), "grabbing inside a scope");
		{
			auto inner = GrabScope();
			Check(Grabbing(), "grabbing inside nested scopes");
		}
		Check(Grabbing(), "outer scope still grabbing");
	}
	Check(!Grabbing(), "not grabbing after scopes end");
}

void TestScopedSurfacePainting() {
	const auto solid = style::internal::OwnedColor(QColor(40, 50, 60));
	const auto sentinel = QColor(200, 10, 30);
	auto image = QImage(8, 8, QImage::Format_ARGB32_Premultiplied);
	image.fill(sentinel);
	Ui::Material::SetActive(true);
	Ui::Material::SetFills({ { Surface::Navigation, QColor(0, 0, 255, 128) } }, {});
	{
		auto scope = Ui::Material::GrabScope();
		auto p = QPainter(&image);
		Ui::Material::PaintSurface(p, image.rect(),
			Surface::Navigation, solid.color());
		Check(image.pixelColor(6, 6).alpha() == 0,
			"rect role fill is absent during a content grab");
		p.fillRect(image.rect(), sentinel);
		auto shape = QPainterPath();
		shape.addRect(image.rect());
		Ui::Material::PaintSurface(p, shape,
			Surface::Navigation, solid.color());
		Check(image.pixelColor(6, 6).alpha() == 0,
			"path role fill is absent during a content grab");
		p.fillRect(image.rect(), sentinel);
		Ui::Material::PaintAnimationBackground(p, image.rect(),
			Surface::Navigation, solid.color());
		Check(image.pixelColor(6, 6).alpha() == 0,
			"nested animation background is absent during a grab");
		Ui::Material::SetActive(false);
		Ui::Material::PaintSurface(p, image.rect(),
			Surface::Navigation, solid.color());
		Check(image.pixelColor(6, 6).rgba() == solid.color()->c.rgba(),
			"fallback paints solid even before a grab scope ends");
	}
}

void TestContentGrabs() {
	const auto solid = style::internal::OwnedColor(QColor(40, 50, 60));
	auto widget = CaptureWidget(solid.color());
	Ui::Material::SetActive(true);
	Ui::Material::SetFills({ { Surface::Navigation, QColor(0, 0, 255, 128) } }, {});
	for (const auto opaque : { false, true }) {
		widget.setAttribute(Qt::WA_OpaquePaintEvent, opaque);
		const auto cache = Ui::Material::Grab(&widget).toImage();
		Check(widget.observedScope, "widget paints within the grab scope");
		Check(cache.pixelColor(6, 6).alpha() == 0,
			"content cache has no material background");
		Check(cache.pixelColor(2, 2) == QColor(Qt::red),
			"content cache retains foreground pixels");
		Check(widget.testAttribute(Qt::WA_OpaquePaintEvent) == opaque,
			"capture preserves the target's opaque attribute");
		Check(!Ui::Material::Grabbing(), "capture ends its scope");
		const auto cropped = Ui::Material::GrabOpaque(
			&widget, QRect(1, 1, 4, 4), solid.color()).toImage();
		Check(cropped.size() == QSize(4, 4), "content grab respects its crop");
		Check(cropped.pixelColor(0, 0).alpha() == 0,
			"material GrabOpaque also captures content only");
		Check(cropped.pixelColor(1, 1) == QColor(Qt::red),
			"cropped content retains the correct foreground position");
	}
	widget.beforePaint = [] { Ui::Material::SetActive(false); };
	const auto changed = Ui::Material::Grab(&widget).toImage();
	Check(changed.pixelColor(6, 6).rgba() == solid.color()->c.rgba(),
		"fallback during capture produces the solid surface");
	Check(!Ui::Material::Grabbing(), "state changes do not leak a grab scope");
	widget.beforePaint = nullptr;
	Check(Ui::Material::Grab(&widget).toImage()
			== Ui::GrabWidget(&widget).toImage(),
		"inactive capture matches the upstream capture");
	Check(Ui::Material::GrabOpaque(&widget, QRect(), solid.color()).toImage()
			== Ui::GrabOpaque(&widget, QRect(), solid.color()->c).toImage(),
		"inactive opaque capture matches the upstream capture");
}

void TestAnimationBackgroundDensity() {
	const auto solid = style::internal::OwnedColor(QColor(40, 50, 60));
	auto widget = CaptureWidget(solid.color());
	Ui::Material::SetActive(true);
	Ui::Material::SetFills({ { Surface::Navigation, QColor(0, 0, 255, 128) } }, {});
	const auto cache = Ui::Material::Grab(&widget);
	for (const auto progress : { 0., 0.25, 0.5, 0.75, 1. }) {
		auto image = QImage(8, 8, QImage::Format_ARGB32_Premultiplied);
		image.fill(Qt::transparent);
		{
			auto p = QPainter(&image);
			Ui::Material::PaintAnimationBackground(p, image.rect(),
				Surface::Navigation, solid.color());
			p.setOpacity(1. - progress);
			p.drawPixmap(0, 0, cache);
			p.setOpacity(progress);
			p.drawPixmap(0, 0, cache);
		}
		Check(image.pixelColor(6, 6) == QColor(0, 0, 255, 128),
			"cross-fade content keeps a constant material background");
	}
	Ui::Material::SetActive(false);
}

} // namespace

namespace crl {

rpl::producer<> on_main_update_requests() {
	return rpl::never<>();
}

} // namespace crl

int main(int argc, char *argv[]) {
	const auto application = QApplication(argc, argv);
	TestDecide();
	TestLiveAllows();
	TestMaterialMirror();
	TestNames();
	TestMaterialFills();
	TestReadableOver();
	TestMaterialFillMirror();
	TestPaintFillReplaces();
	TestShapeAndSurfaceFill();
	TestWidgetMaterialBindings();
	TestGrabScopeNests();
	TestScopedSurfacePainting();
	TestContentGrabs();
	TestAnimationBackgroundDensity();
	std::cout << "Checks: " << TotalChecks
		<< ", failures: " << FailedChecks << std::endl;
	return FailedChecks ? 1 : 0;
}
