/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "window/window_soft_panels_layout.h"
#include "ui/rp_widget.h"

#include <QtGui/QPainterPath>

namespace Window::SoftPanels {

extern const char kOptionLayout[];

void Initialize();
[[nodiscard]] bool LayoutEnabled();
void SetLayoutEnabled(bool enabled);
void RefreshSoftControls();
[[nodiscard]] QPixmap GrabContent(not_null<QWidget*> widget, QRect rect);
[[nodiscard]] int TopBarHeight();
[[nodiscard]] int TopBarShift();
[[nodiscard]] int TopBarShadowHeight();
void PaintCard(QPainter &p, QRect card);

// Widgets with this property split a card layout like a divider does.
inline constexpr auto kCardGapProperty = "softCardGap";

struct CardsArgs {
	int inset = 0;
	int shrink = 0;
};

// Draws rounded cards behind runs of rows in content, under its widgets.
void SetupCards(
	not_null<QWidget*> host,
	not_null<Ui::RpWidget*> content,
	CardsArgs args = {});

class PanelFrame final : public Ui::RpWidget {
public:
	PanelFrame(not_null<QWidget*> parent, Corners corners);
	void setPanelGeometry(QRect panel);

protected:
	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;

private:
	const Corners _corners;
	QPainterPath _outside;
	QPainterPath _outline;
	QRect _inner;

};

} // namespace Window::SoftPanels
