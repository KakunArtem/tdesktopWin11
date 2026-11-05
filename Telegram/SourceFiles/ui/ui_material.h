/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "rpl/producer.h"
#include "base/flat_map.h"
#include "base/basic_types.h"
#include "ui/style/style_core.h"

#include <QtCore/QRect>
#include <QtGui/QColor>

class QPainter;
class QPainterPath;
class QPixmap;
class QWidget;

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
[[nodiscard]] bool SoftPanels();
void SetSoftPanels(bool enabled);
[[nodiscard]] rpl::producer<bool> SoftPanelsValue();
[[nodiscard]] uint64 Revision();
[[nodiscard]] rpl::producer<bool> ActiveValue();
void SetActive(bool active);
void SetFills(
	base::flat_map<Surface, QColor> fills,
	base::flat_map<Surface, QColor> borders);
[[nodiscard]] QColor Fill(Surface surface);
[[nodiscard]] QColor Border(Surface surface);
[[nodiscard]] rpl::producer<> FillsChanged();

class GrabScope final {
public:
	GrabScope();
	~GrabScope();
	GrabScope(const GrabScope &) = delete;
	GrabScope &operator=(const GrabScope &) = delete;

};

[[nodiscard]] bool Grabbing();
[[nodiscard]] QPixmap Grab(not_null<QWidget*> widget, QRect rect = QRect());
[[nodiscard]] QPixmap GrabOpaque(
	not_null<QWidget*> widget,
	QRect rect,
	const style::color &solid);
void PaintAnimationBackground(
	QPainter &p,
	const QRect &rect,
	Surface surface,
	const style::color &solid);
void PaintFill(QPainter &p, const QRect &rect, const QColor &fill);
void PaintFill(QPainter &p, const QPainterPath &shape, const QColor &fill);
void PaintSurface(
	QPainter &p,
	const QRect &rect,
	Surface surface,
	const style::color &solid);
void PaintSurface(
	QPainter &p,
	const QPainterPath &shape,
	Surface surface,
	const style::color &solid);
void SetOpaquePaint(not_null<QWidget*> widget, bool opaqueInSolidMode);
void RepaintOnFillsChanged(not_null<QWidget*> widget);

} // namespace Ui::Material
