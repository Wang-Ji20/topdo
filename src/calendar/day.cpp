#include "day.h"

#include <algorithm>

namespace topdo {

bool happensOn(const Event &event, QDate day) {
    if (event.allDay) {
        return event.start.date() <= day && day <= event.end.date();
    }
    const QDateTime dayStart(day, QTime(0, 0));
    const QDateTime dayEnd = dayStart.addDays(1);
    if (event.start == event.end) {
        return dayStart <= event.start && event.start < dayEnd;
    }
    return event.start < dayEnd && event.end > dayStart;
}

QList<Event> sortedDay(QList<Event> events) {
    const auto order = [](const Event &event) {
        return std::tuple(!event.allDay, event.start, event.end);
    };
    std::ranges::stable_sort(events, {}, order);
    return events;
}

std::optional<qsizetype> nextEvent(const QList<Event> &events, const QDateTime &now) {
    for (qsizetype i = 0; i < events.size(); ++i) {
        const Event &event = events.at(i);
        if (!event.allDay && event.end > now) {
            return i;
        }
    }
    return std::nullopt;
}

QString countdown(const Event &event, const QDateTime &now) {
    if (event.start <= now) {
        return QStringLiteral("now, until %1").arg(event.end.toString(QStringLiteral("HH:mm")));
    }
    const qint64 minutes = (now.secsTo(event.start) + 59) / 60; // "in 1 min", never "in 0 min"
    if (minutes < 60) {
        return QStringLiteral("in %1 min").arg(minutes);
    }
    const qint64 hours = minutes / 60;
    const qint64 rest = minutes % 60;
    if (rest == 0) {
        return QStringLiteral("in %1 h").arg(hours);
    }
    return QStringLiteral("in %1 h %2 min").arg(hours).arg(rest);
}

} // namespace topdo
