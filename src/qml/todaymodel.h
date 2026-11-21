#pragma once

#include "akonadicalendar.h"

#include <QDateTime>
#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QVariantList>

namespace topdo {

// Today's events for the today column and the badge, from the KDE calendar, with a clock that
// moves `now` on. See docs/design/project-today.md.
class TodayModel : public QObject {
    Q_OBJECT
    QML_ELEMENT

    // One map per event, in sortedDay() order: title, start, end (QDateTime) and allDay.
    Q_PROPERTY(QVariantList events READ events NOTIFY changed)
    Q_PROPERTY(QDateTime now READ now NOTIFY changed)
    // The index in events of nextEvent(), or -1.
    Q_PROPERTY(int next READ next NOTIFY changed)
    // countdown() for the next event, or empty.
    Q_PROPERTY(QString countdown READ countdown NOTIFY changed)

  public:
    explicit TodayModel(QObject *parent = nullptr);

    [[nodiscard]] QVariantList events() const;
    [[nodiscard]] QDateTime now() const { return m_now; }
    [[nodiscard]] int next() const { return m_next; }
    [[nodiscard]] QString countdown() const;

  Q_SIGNALS:
    void changed();

  private:
    void findNext();
    void reload();
    void tick();

    AkonadiCalendar m_calendar;
    QTimer m_clock;
    QDateTime m_now;
    QList<Event> m_events;
    int m_next = -1;
};

} // namespace topdo
