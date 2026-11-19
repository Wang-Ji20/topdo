#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QRectF>
#include <QWindow>

namespace topdo {

// Window effects that QML can't ask KWin for on its own. A Plasma dialog without the theme's
// background loses KWin's blur, so the panel asks for it here.
class WindowEffects : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

  public:
    // Blurs what is behind `rect` of the window, with round corners of `radius` pixels.
    Q_INVOKABLE void blurBehind(QWindow *window, const QRectF &rect, qreal radius);
};

} // namespace topdo
