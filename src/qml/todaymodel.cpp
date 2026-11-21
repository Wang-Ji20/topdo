#include "todaymodel.h"

#include <chrono>

namespace topdo {

namespace {

using namespace std::chrono_literals;

// Often enough for a countdown in minutes.
constexpr auto ClockInterval = 30s;

QVariantMap eventMap(const Event &event) {
    return {
        {QStringLiteral("title"), event.title},
        {QStringLiteral("start"), event.start},
        {QStringLiteral("end"), event.end},
        {QStringLiteral("allDay"), event.allDay},
    };
}

} // namespace

TodayModel::TodayModel(QObject *parent) : QObject(parent), m_now(QDateTime::currentDateTime()) {
    connect(&m_calendar, &AkonadiCalendar::changed, this, &TodayModel::reload);
    connect(&m_clock, &QTimer::timeout, this, &TodayModel::tick);
    m_clock.start(ClockInterval);
    reload();
}

QVariantList TodayModel::events() const {
    QVariantList events;
    for (const Event &event : m_events) {
        events.append(eventMap(event));
    }
    return events;
}

QString TodayModel::countdown() const {
    return m_next < 0 ? QString() : topdo::countdown(m_events.at(m_next), m_now);
}

void TodayModel::findNext() {
    const std::optional<qsizetype> next = nextEvent(m_events, m_now);
    m_next = next ? static_cast<int>(*next) : -1;
    Q_EMIT changed();
}

void TodayModel::reload() {
    m_events = m_calendar.eventsOn(m_now.date());
    findNext();
}

// Moves the clock on; a new day loads its events.
void TodayModel::tick() {
    const QDate before = m_now.date();
    m_now = QDateTime::currentDateTime();
    if (m_now.date() != before) {
        reload();
        return;
    }
    findNext();
}

} // namespace topdo
