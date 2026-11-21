#pragma once

#include <QDateTime>
#include <QList>
#include <QString>

#include <optional>

namespace topdo {

// An event of the day, as the today column shows it. See docs/design/project-today.md.
struct Event {
    QString title;
    QDateTime start;
    QDateTime end;
    bool allDay = false;

    friend bool operator==(const Event &, const Event &) = default;
};

// Whether the event takes up part of the day: an all-day event on one of its dates, a timed event
// that starts before the day ends and ends after it starts. An event of no length counts on the
// day it starts.
bool happensOn(const Event &event, QDate day);

// The events in the order the column lists them: all-day events first, then by start, then by
// end; equal ones keep their order.
QList<Event> sortedDay(QList<Event> events);

// The event the column and the badge lead with: the first timed event that hasn't ended at `now`,
// in sortedDay() order. Nothing if every timed event is over.
std::optional<qsizetype> nextEvent(const QList<Event> &events, const QDateTime &now);

// How long until the event: "in 28 min", "in 2 h 5 min", or, once it has started, "now, until
// 15:30".
QString countdown(const Event &event, const QDateTime &now);

} // namespace topdo
