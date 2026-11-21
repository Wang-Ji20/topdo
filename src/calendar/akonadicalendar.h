#pragma once

#include "day.h"

#include <QDate>
#include <QObject>

namespace Akonadi {
class ETMCalendar;
}

namespace topdo {

// The user's calendars as KDE keeps them in Akonadi (KOrganizer's, Google's once synced), read
// only. Akonadi loads them in the background; changed() fires when they arrive or change.
class AkonadiCalendar : public QObject {
    Q_OBJECT

  public:
    explicit AkonadiCalendar(QObject *parent = nullptr);

    // The events of a day, recurring ones included, in sortedDay() order.
    [[nodiscard]] QList<Event> eventsOn(QDate day) const;

  Q_SIGNALS:
    void changed();

  private:
    Akonadi::ETMCalendar *m_calendar;
};

} // namespace topdo
