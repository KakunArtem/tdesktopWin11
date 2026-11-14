/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "window/window_material.h"

#include "base/flat_set.h"
#include "base/options.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "rpl/variable.h"
#include "styles/palette.h"
#include "ui/chat/chat_theme_readability.h"
#include "ui/style/style_core_palette.h"
#include "ui/color_contrast.h"
#include "ui/platform/ui_popup_backdrop.h"
#include "ui/ui_material.h"
#include "window/window_material_fills.h"
#include "window/window_soft_panels.h"

#include <algorithm>
#include <cmath>
#include <optional>

#ifdef Q_OS_WIN
#include "platform/win/window_material_win.h"
#endif // Q_OS_WIN

namespace Window::Material {
namespace {

constexpr auto kPopupBackdropFillOpacity = 0.80;

constexpr auto kPopupMaterialPref = std::string_view("popup_material");
constexpr auto kTransparencyPref = std::string_view(
	"window_material_transparency");

// WHY: the experimental option is no longer listed; it stays registered so
// a value stored by an older build is read once into the setting.
base::options::toggle PopupMaterialOption({
	.id = kOptionPopupMaterialPrototype,
	.name = "Popup material prototype",
	.description = "Show the Windows 11 Acrylic backdrop behind popup menus"
		" while window transparency is active. For testing the redesign.",
	.defaultValue = true,
	.scope = base::options::windows,
	.restartRequired = true,
});

auto StartupCapability = std::optional<bool>();
auto ActiveState = rpl::variable<bool>(false);
auto ChatBackgroundState = rpl::variable<bool>(false);
auto StateLifetime = rpl::lifetime();
auto Adjustments = base::flat_map<const style::color*, Adjustment>();
auto PaletteSnapshot = QByteArray();
auto Adjusting = false;

using Colors = base::flat_map<const style::color*, QColor>;
using Fills = base::flat_map<Surface, QColor>;

constexpr auto Surfaces = std::array{
	Surface::Window, Surface::Navigation, Surface::Conversation,
	Surface::Bar, Surface::Composer, Surface::Popup,
};

struct AdjustmentRule {
	const style::color *entry = nullptr;
	std::vector<Surface> roles;
	float64 target = 4.5;
	const style::color *under = nullptr;
};

[[nodiscard]] const std::vector<AdjustmentRule> &AdjustmentRules() {
	static const auto result = std::vector<AdjustmentRule>{
		{ &st::windowSubTextFg,
			{ Surface::Navigation, Surface::Conversation } },
		{ &st::dialogsTextFg,
			{ Surface::Navigation, Surface::Conversation } },
		{ &st::dialogsDateFg,
			{ Surface::Navigation, Surface::Conversation } },
		{ &st::dialogsTextFgService,
			{ Surface::Navigation, Surface::Conversation } },
		{ &st::windowActiveTextFg,
			{ Surface::Navigation, Surface::Conversation } },
		{ &st::historyComposeAreaFgService,
			{ Surface::Navigation, Surface::Conversation } },
		{ &st::titleButtonFg, { Surface::Window }, 3. },
		{ &st::titleButtonFgActive, { Surface::Window }, 3. },
		{ &st::titleButtonCloseFg, { Surface::Window }, 3. },
		{ &st::titleButtonCloseFgActive, { Surface::Window }, 3. },
		{ &st::titleFg, { Surface::Window } },
		{ &st::titleFgActive, { Surface::Window } },
		{ &st::checkboxFg,
			{ Surface::Navigation, Surface::Conversation, Surface::Popup },
			3. },
		{ &st::activeLineFg,
			{ Surface::Navigation, Surface::Conversation, Surface::Popup },
			3. },
		{ &st::lightButtonFg,
			{ Surface::Navigation, Surface::Conversation, Surface::Popup } },
		{ &st::menuIconFg,
			{ Surface::Navigation, Surface::Conversation, Surface::Popup },
			3. },
		{ &st::cancelIconFg,
			{ Surface::Navigation, Surface::Conversation, Surface::Popup },
			3. },
		{ &st::boxTitleCloseFg,
			{ Surface::Navigation, Surface::Conversation, Surface::Popup },
			3. },
		{ &st::menuIconFgOver,
			{ Surface::Navigation, Surface::Conversation, Surface::Popup } },
		{ &st::cancelIconFgOver,
			{ Surface::Navigation, Surface::Conversation, Surface::Popup } },
		{ &st::boxTitleCloseFgOver,
			{ Surface::Navigation, Surface::Conversation, Surface::Popup } },
		{ &st::dialogsMenuIconFg, { Surface::Navigation }, 3. },
		{ &st::sideBarIconFg, { Surface::Navigation }, 3. },
		{ &st::sideBarIconFgActive, { Surface::Navigation }, 3. },
		{ &st::dialogsMenuIconFgOver, { Surface::Navigation } },
		{ &st::historyComposeIconFg, { Surface::Composer }, 3. },
		{ &st::historyReplyCancelFg, { Surface::Composer }, 3. },
		{ &st::historyComposeIconFgOver, { Surface::Composer } },
		{ &st::historyReplyCancelFgOver, { Surface::Composer } },
		{ &st::emojiIconFg, { Surface::Popup }, 3. },
		{ &st::smallCloseIconFg, { Surface::Popup }, 3. },
		{ &st::smallCloseIconFgOver, { Surface::Popup } },
		{ &st::sideBarTextFg, { Surface::Navigation } },
		{ &st::sideBarTextFgActive, { Surface::Navigation } },
		{ &st::placeholderFg, { Surface::Navigation, Surface::Bar } },
		// The row hover tint takes this margin, leaving 4.5:1 on the pill.
		{ &st::dialogsTextFgOver, { Surface::Navigation }, 5.4 },
		{ &st::dialogsDateFgOver, { Surface::Navigation }, 5.4 },
		{ &st::dialogsTextFgServiceOver, { Surface::Navigation }, 5.4 },
		{ &st::dialogsUnreadBgMuted, {}, 4.5, &st::dialogsUnreadFg },
		{ &st::dialogsUnreadBgMutedOver, {}, 4.5, &st::dialogsUnreadFgOver },
	};
	return result;
}

[[nodiscard]] FillInputs Inputs(
		Surface surface,
		bool dark,
		const Colors &colors = {}) {
	const auto color = [&](const style::color &entry) {
		const auto i = colors.find(&entry);
		return (i != colors.end()) ? i->second : entry->c;
	};
	switch (surface) {
	case Surface::Window:
	case Surface::Conversation:
		return { color(st::windowBg), color(st::windowFg),
			color(st::windowSubTextFg), dark };
	case Surface::Navigation:
		return { color(st::dialogsBg), color(st::dialogsNameFg),
			color(st::dialogsTextFg), dark };
	case Surface::Bar:
		return { color(st::topBarBg), color(st::windowFg),
			color(st::windowSubTextFg), dark };
	case Surface::Composer:
		return { color(st::historyComposeAreaBg), color(st::windowFg),
			color(st::historyComposeAreaFgService), dark };
	case Surface::Popup:
		return { color(st::menuBg), color(st::windowFg),
			color(st::windowSubTextFg), dark };
	}
	Unexpected("Invalid material surface.");
}

bool SetPaletteEntry(const style::color &entry, const QColor &color) {
	if (entry->c == color) {
		return false;
	}
	entry.set(
		uchar(color.red()),
		uchar(color.green()),
		uchar(color.blue()),
		uchar(color.alpha()));
	return true;
}

[[nodiscard]] float64 ContrastOver(
		const QColor &text,
		const QColor &background) {
	return Ui::CountContrast(Composite(text, background), background);
}

[[nodiscard]] std::pair<Colors, Fills> CalculateFills(
		const Colors &originals,
		bool dark) {
	auto coverage = base::flat_map<Surface, int>();
	for (const auto role : Surfaces) {
		const auto inputs = Inputs(role, dark, originals);
		auto opaque = inputs.surface;
		opaque.setAlpha(255);
		const auto unreadable = ContrastOver(inputs.text, opaque) <= 1.
			|| ContrastOver(inputs.secondaryText, opaque) <= 1.;
		coverage[role] = unreadable
			? 100
			: int(std::lround(EffectiveFloor(role, dark) * 100));
	}
	const auto range = AcrylicRange(dark);
	auto blocked = base::flat_set<const style::color*>();
	auto colors = Colors();
	auto fills = Fills();
	const auto limit = 1 + int(Surfaces.size()) * 100
		+ int(AdjustmentRules().size());
	for (auto pass = 0; pass != limit; ++pass) {
		colors.clear();
		auto failed = std::vector<const AdjustmentRule*>();
		for (const auto &rule : AdjustmentRules()) {
			const auto original = originals.find(rule.entry)->second;
			if (blocked.contains(rule.entry)) {
				colors[rule.entry] = original;
				continue;
			}
			auto backgrounds = std::vector<QColor>();
			for (const auto role : rule.roles) {
				auto fill = Inputs(role, dark).surface;
				fill.setAlphaF(coverage[role] / 100.);
				backgrounds.push_back(Composite(fill, range.darkest));
				backgrounds.push_back(Composite(fill, range.brightest));
			}
			if (rule.under) {
				backgrounds.push_back((*rule.under)->c);
			}
			const auto adjusted = Ui::EnsureTextContrast(
				original,
				backgrounds,
				rule.target);
			colors[rule.entry] = adjusted;
			if (ranges::any_of(backgrounds, [&](const QColor &background) {
				return ContrastOver(adjusted, background) < rule.target;
			})) {
				failed.push_back(&rule);
			}
		}
		if (!failed.empty()) {
			auto increase = base::flat_set<Surface>();
			for (const auto rule : failed) {
				if (ranges::any_of(rule->roles, [&](Surface role) {
					return coverage[role] < 100;
				})) {
					for (const auto role : rule->roles) {
						if (coverage[role] < 100) {
							increase.emplace(role);
						}
					}
				} else {
					blocked.emplace(rule->entry);
				}
			}
			for (const auto role : increase) {
				++coverage[role];
			}
			continue;
		}
		auto changed = false;
		for (const auto role : Surfaces) {
			auto fill = ComputeFill(role, Inputs(role, dark, colors));
			const auto needed = int(std::lround(fill.alphaF() * 100));
			if (needed > coverage[role]) {
				coverage[role] = needed;
				changed = true;
			}
			fill.setAlphaF(coverage[role] / 100.);
			fills[role] = fill;
		}
		if (!changed) {
			return { std::move(colors), std::move(fills) };
		}
	}
	for (const auto role : Surfaces) {
		auto fill = Inputs(role, dark, originals).surface;
		fill.setAlpha(255);
		fills[role] = fill;
	}
	return { originals, std::move(fills) };
}

void RecomputeFills(bool active) {
	Adjusting = true;
	const auto guard = gsl::finally([] {
		Adjusting = false;
	});
	const auto dark = Ui::IsDarkPalette(*style::main_palette::get());
	auto changed = false;
	auto fills = Fills();
	if (active) {
		auto originals = Colors();
		for (const auto &rule : AdjustmentRules()) {
			const auto &entry = *rule.entry;
			const auto i = Adjustments.find(rule.entry);
			originals[rule.entry] = (i != Adjustments.end())
				? i->second.original
				: entry->c;
		}
		auto calculated = CalculateFills(originals, dark);
		fills = std::move(calculated.second);
		for (const auto &[entry, adjusted] : calculated.first) {
			Adjustments[entry] = { originals.find(entry)->second, adjusted };
			changed = SetPaletteEntry(*entry, adjusted) || changed;
		}
	} else {
		for (const auto &[entry, adjustment] : Adjustments) {
			changed = SetPaletteEntry(*entry, adjustment.original) || changed;
		}
		Adjustments.clear();
	}
	auto borders = base::flat_map<Surface, QColor>();
	for (const auto role : Surfaces) {
		const auto inputs = Inputs(role, dark);
		if (!active) {
			auto fill = inputs.surface;
			fill.setAlpha(255);
			fills.emplace(role, fill);
		}
		borders.emplace(role, BorderColor(inputs.text));
	}
	if (changed) {
		style::NotifyPaletteChanged();
	}
	PaletteSnapshot = style::main_palette::save();
	Ui::Material::SetFills(std::move(fills), std::move(borders));
	Ui::Platform::SetPopupBackdrop(
		active && PopupMaterialEnabled(),
		kPopupBackdropFillOpacity);
}

} // namespace

const char kOptionPopupMaterialPrototype[] = "popup-material-prototype";

bool SettingAvailable() {
#ifdef Q_OS_WIN
	return Platform::Material::Supported();
#else // Q_OS_WIN
	return false;
#endif // Q_OS_WIN
}

bool SettingEnabled() {
	return Core::App().settings().readPref<bool>("window_material", true);
}

void SetSettingEnabled(bool enabled) {
	Core::App().settings().writePref<bool>("window_material", enabled);
	Core::App().saveSettingsDelayed();
}

bool ChatBackgroundEnabled() {
	return Core::App().settings().readPref<bool>(
		"window_material_chat_background",
		true);
}

bool PopupMaterialEnabled() {
	auto &settings = Core::App().settings();
	const auto stored = settings.readPref<bool>(kPopupMaterialPref, false);
	if (stored != settings.readPref<bool>(kPopupMaterialPref, true)) {
		const auto migrated = PopupMaterialOption.value();
		LOG(("Material: migrated popup setting %1."
			).arg(Logs::b(migrated)));
		settings.writePref<bool>(kPopupMaterialPref, migrated);
		Core::App().saveSettingsDelayed();
		return migrated;
	}
	return stored;
}

void SetPopupMaterialEnabled(bool enabled) {
	Core::App().settings().writePref<bool>(kPopupMaterialPref, enabled);
	Core::App().saveSettingsDelayed();
}

void SetChatBackgroundEnabled(bool enabled) {
	Core::App().settings().writePref<bool>(
		"window_material_chat_background",
		enabled);
	Core::App().saveSettingsDelayed();
	ChatBackgroundState = enabled;
}

int WindowTransparency() {
	const auto stored = Core::App().settings().readPref<QByteArray>(
		kTransparencyPref);
	auto ok = false;
	const auto value = stored.toInt(&ok);
	return ok
		? std::clamp(value, 0, 100)
		: int(std::lround(kDefaultTransparency * 100));
}

void SetWindowTransparency(int value) {
	value = std::clamp(value, 0, 100);
	Core::App().settings().writePref<QByteArray>(
		kTransparencyPref,
		QByteArray::number(value));
	Core::App().saveSettingsDelayed();
	SetTransparency(value / 100.);
	if (Active()) {
		RecomputeFills(true);
	}
}

bool ChatBackgroundActive() {
	return Active() && ChatBackgroundState.current();
}

rpl::producer<bool> ChatBackgroundActiveValue() {
	return rpl::combine(
		ActiveValue(),
		ChatBackgroundState.value(),
		[](bool active, bool enabled) { return active && enabled; }
	) | rpl::distinct_until_changed();
}

bool Capable() {
	if (!StartupCapability.has_value()) {
		SoftPanels::Initialize();
		ChatBackgroundState = ChatBackgroundEnabled();
		SetTransparency(WindowTransparency() / 100.);
#ifdef Q_OS_WIN
		StartupCapability = Platform::Material::Decide().backdrop
			== Backdrop::Acrylic;
#else // Q_OS_WIN
		StartupCapability = false;
#endif // Q_OS_WIN
		style::PaletteChanged() | rpl::on_next([] {
			if (Adjusting || style::main_palette::save() == PaletteSnapshot) {
				return;
			}
			for (auto i = Adjustments.begin(); i != Adjustments.end();) {
				if ((*i->first)->c != i->second.adjusted) {
					i = Adjustments.erase(i);
				} else {
					++i;
				}
			}
			RecomputeFills(ActiveState.current());
		}, StateLifetime);
		ActiveState.value() | rpl::on_next([](bool active) {
			Ui::Material::SetActive(active);
			RecomputeFills(active);
		}, StateLifetime);
#ifdef Q_OS_WIN
		Core::App().settings().nativeWindowFrameChanges(
		) | rpl::on_next([](bool) {
			Refresh();
		}, StateLifetime);
#endif // Q_OS_WIN
		Refresh();
	}
	return *StartupCapability;
}

bool Active() {
	return Capable() && ActiveState.current();
}

rpl::producer<bool> ActiveValue() {
	if (!Capable()) {
		return rpl::single(false);
	}
	return ActiveState.value();
}

void Refresh() {
	if (!StartupCapability.has_value()) {
		return;
	}
#ifdef Q_OS_WIN
	const auto environment = Platform::Material::ReadEnvironment();
	ActiveState = *StartupCapability
		&& LiveAllows(environment)
		&& !environment.nativeFrame;
#else // Q_OS_WIN
	ActiveState = false;
#endif // Q_OS_WIN
}

void ReportBackdropFailure() {
	if (!Capable()) {
		return;
	}
	StartupCapability = false;
	LOG(("Material: backdrop failure; using solid rendering until restart."));
	Refresh();
}

void PaletteReplaced() {
	Adjustments.clear();
	PaletteSnapshot.clear();
}

QByteArray UnadjustedPalette() {
	auto result = style::main_palette::save();
	for (const auto &[entry, adjustment] : Adjustments) {
		if ((*entry)->c != adjustment.adjusted) {
			continue;
		}
		const auto index = style::main_palette::indexOfColor(*entry);
		Expects(index >= 0 && (index + 1) * 4 <= result.size());
		const auto color = adjustment.original;
		result[index * 4] = char(color.red());
		result[index * 4 + 1] = char(color.green());
		result[index * 4 + 2] = char(color.blue());
		result[index * 4 + 3] = char(color.alpha());
	}
	return result;
}

} // namespace Window::Material
