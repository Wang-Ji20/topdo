#include "akonadicalendar.h"

#include <Akonadi/ETMCalendar>
#include <KCalendarCore/Event>
#include <KCalendarCore/OccurrenceIterator>

namespace topdo {

AkonadiCalendar::AkonadiCalendar(QObject *parent)
    : QObject(parent),
      m_calendar(new Akonadi::ETMCalendar({KCalendarCore::Event::eventMimeType()}, this)) {
    // Otherwise the calendar holds only the collections ticked in its own selection, which is
    // empty: no events at all. topdo shows every calendar.
    m_calendar->setCollectionFilteringEnabled(false);
    connect(m_calendar, &Akonadi::ETMCalendar::calendarChanged, this, &AkonadiCalendar::changed);
}

QList<Event> AkonadiCalendar::eventsOn(QDate day) const {
    const QDateTime start(day, QTime(0, 0));
    const QDateTime end = start.addDays(1).addMSecs(-1);
    QList<Event> events;
    KCalendarCore::OccurrenceIterator occurrences(*m_calendar, start, end);
    while (occurrences.hasNext()) {
        occurrences.next();
        const KCalendarCore::Incidence::Ptr incidence = occurrences.incidence();
        if (incidence->type() != KCalendarCore::IncidenceBase::TypeEvent) {
            continue;
        }
        const QDateTime from = occurrences.occurrenceStartDate().toLocalTime();
        const Event event{
            .title = incidence->summary(),
            .start = from,
            .end = incidence->endDateForStart(from).toLocalTime(),
            .allDay = incidence->allDay(),
        };
        if (happensOn(event, day)) { // the iterator also yields events that end at midnight
            events.append(event);
        }
    }
    return sortedDay(events);
}

} // namespace topdo
