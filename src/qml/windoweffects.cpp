#include "windoweffects.h"

#include <KWindowEffects>
#include <QPainterPath>
#include <QRegion>

namespace topdo {

void WindowEffects::blurBehind(QWindow *window, const QRectF &rect, qreal radius) {
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);
    const QPolygon outline = path.toFillPolygon().toPolygon();
    KWindowEffects::enableBlurBehind(window, true, QRegion(outline));
}

} // namespace topdo
