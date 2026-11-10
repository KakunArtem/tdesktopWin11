/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "platform/win/window_material_win.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "ui/chat/chat_theme_readability.h"
#include "ui/style/style_core_palette.h"
#include "ui/ui_material.h"
#include "ui/widgets/rp_window.h"
#include "window/window_material.h"
#include "styles/style_widgets.h"
#include "styles/style_window_soft_panels.h"

#include <QtCore/QOperatingSystemVersion>

#include <dwmapi.h>

namespace Platform::Material {
namespace {

constexpr auto kDWMWA_USE_IMMERSIVE_DARK_MODE = DWORD(20);
constexpr auto kDWMWA_SYSTEMBACKDROP_TYPE = DWORD(38);
constexpr auto kDWMSBT_TRANSIENTWINDOW = DWORD(3);

[[nodiscard]] bool TransparencyEffectsEnabled() {
	auto value = DWORD(1);
	auto size = DWORD(sizeof(value));
	const auto result = RegGetValueW(
		HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
		L"EnableTransparency",
		RRF_RT_REG_DWORD,
		nullptr,
		&value,
		&size);
	return (result != ERROR_SUCCESS) || (value != 0);
}

[[nodiscard]] bool HighContrastEnabled() {
	auto info = HIGHCONTRASTW{ .cbSize = sizeof(HIGHCONTRASTW) };
	return SystemParametersInfoW(SPI_GETHIGHCONTRAST, info.cbSize, &info, 0)
		&& (info.dwFlags & HCF_HIGHCONTRASTON);
}

[[nodiscard]] QString Hex(HRESULT result) {
	return QString::number(quint32(result), 16);
}

[[nodiscard]] style::WindowTitle MakeSoftPanelsTitleStyle(bool material) {
	auto result = st::defaultWindowTitle;
	result.shadow = false;
	if (material) {
		result.minimize.icon = st::materialTitleMinimize;
		result.minimizeIconActive = st::materialTitleMinimizeActive;
		result.maximize.icon = st::materialTitleMaximize;
		result.maximizeIconActive = st::materialTitleMaximizeActive;
		result.restoreIcon = st::materialTitleRestore;
		result.restoreIconActive = st::materialTitleRestoreActive;
		result.close.icon = st::materialTitleClose;
		result.closeIconActive = st::materialTitleCloseActive;
	}
	return result;
}

} // namespace

bool BackdropDark() {
	return Ui::IsDarkPalette(*style::main_palette::get());
}

const style::WindowTitle &TitleStyle(bool material, bool softPanels) {
	static const auto softPanelsTitle = MakeSoftPanelsTitleStyle(false);
	static const auto materialTitle = MakeSoftPanelsTitleStyle(true);
	return material
		? materialTitle
		: softPanels
		? softPanelsTitle
		: st::defaultWindowTitle;
}

void SetupWindowMaterial(not_null<Ui::RpWindow*> window) {
	rpl::combine(
		Ui::Material::ActiveValue(),
		Ui::Material::SoftPanelsValue()
	) | rpl::on_next([=](bool material, bool softPanels) {
		window->setTitleStyle(TitleStyle(material, softPanels));
	}, window->lifetime());
	if (!Window::Material::Capable()
		|| Core::App().settings().nativeWindowFrame()) {
		return;
	}
	window->setTitleBackgroundPainter([=](QPainter &p, const QRect &rect) {
		Ui::Material::PaintSurface(
			p,
			rect,
			Ui::Material::Surface::Window,
			window->isActiveWindow() ? st::titleBgActive : st::titleBg);
	});
	if (const auto title = window->titleWidget()) {
		Ui::Material::RepaintOnFillsChanged(title);
	}
	const auto apply = [=](bool dark) {
		if (!window->testAttribute(Qt::WA_WState_Created)) {
			return;
		}
		const auto handle = reinterpret_cast<HWND>(window->winId());
		if (handle && !ApplyBackdrop(handle, dark)) {
			Window::Material::ReportBackdropFailure();
		}
	};
	window->winIdValue() | rpl::filter([](WId id) {
		return id != 0;
	}) | rpl::on_next([=] {
		apply(BackdropDark());
	}, window->lifetime());
	Ui::Material::FillsChanged(
	) | rpl::map(
		BackdropDark
	) | rpl::distinct_until_changed(
	) | rpl::on_next(apply, window->lifetime());
}

bool Supported() {
	return QOperatingSystemVersion::current().microVersion()
		>= Window::Material::kMinimumBackdropBuild;
}

Window::Material::Environment ReadEnvironment() {
	return {
		.requested = Window::Material::SettingEnabled(),
		.windowsBuild = QOperatingSystemVersion::current().microVersion(),
		.transparencyEffects = TransparencyEffectsEnabled(),
		.highContrast = HighContrastEnabled(),
		.nativeFrame = Core::App().settings().nativeWindowFrame(),
	};
}

Window::Material::Decision Decide() {
	const auto environment = ReadEnvironment();
	const auto result = Window::Material::Decide(environment);
	LOG(("Material: build %1, transparency %2, high contrast %3, "
		"native frame %4, refusal: %5"
		).arg(environment.windowsBuild
		).arg(Logs::b(environment.transparencyEffects)
		).arg(Logs::b(environment.highContrast)
		).arg(Logs::b(environment.nativeFrame)
		).arg(Window::Material::RefusalName(result.refusal)));
	return result;
}

bool ApplyBackdrop(HWND handle, bool dark) {
	const auto margins = MARGINS{ -1, -1, -1, -1 };
	const auto extended = DwmExtendFrameIntoClientArea(handle, &margins);
	const auto darkValue = BOOL(dark ? TRUE : FALSE);
	const auto darkened = DwmSetWindowAttribute(
		handle,
		kDWMWA_USE_IMMERSIVE_DARK_MODE,
		&darkValue,
		sizeof(darkValue));
	const auto type = kDWMSBT_TRANSIENTWINDOW;
	const auto backdrop = DwmSetWindowAttribute(
		handle,
		kDWMWA_SYSTEMBACKDROP_TYPE,
		&type,
		sizeof(type));
	LOG(("Material: extend frame 0x%1, dark mode 0x%2, backdrop 0x%3"
		).arg(Hex(extended)
		).arg(Hex(darkened)
		).arg(Hex(backdrop)));
	return SUCCEEDED(extended) && SUCCEEDED(backdrop);
}

} // namespace Platform::Material
