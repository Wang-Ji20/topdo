#include "day.h"

#include <QTest>

using topdo::Event;

namespace {

QDateTime at(int hour, int minute = 0) { return {QDate(2026, 11, 21), QTime(hour, minute)}; }

Event timed(const QString &title, int fromHour, int fromMinute, int toHour, int toMinute) {
    return {
        .title = title,
        .start = at(fromHour, fromMinute),
        .end = at(toHour, toMinute),
        .allDay = false,
    };
}

Event allDay(const QString &title) {
    return {.title = title, .start = at(0), .end = at(0).addDays(1), .allDay = true};
}

} // namespace

class TestDay : public QObject {
    Q_OBJECT

  private Q_SLOTS:
    void knowsWhichDayAnEventHappensOn_data() {
        QTest::addColumn<QDateTime>("start");
        QTest::addColumn<QDateTime>("end");
        QTest::addColumn<bool>("allDay");
        QTest::addColumn<bool>("today");

        QTest::newRow("during the day") << at(9) << at(10) << false << true;
        QTest::newRow("ends at midnight before") << at(20).addDays(-1) << at(0) << false << false;
        QTest::newRow("starts at midnight after")
            << at(0).addDays(1) << at(1).addDays(1) << false << false;
        QTest::newRow("over midnight") << at(23).addDays(-1) << at(1) << false << true;
        QTest::newRow("no length") << at(9) << at(9) << false << true;
        QTest::newRow("all day today") << at(0) << at(0) << true << true;
        QTest::newRow("all day yesterday")
            << at(0).addDays(-1) << at(0).addDays(-1) << true << false;
        QTest::newRow("all day, several") << at(0).addDays(-1) << at(0).addDays(1) << true << true;
    }

    void knowsWhichDayAnEventHappensOn() {
        QFETCH(QDateTime, start);
        QFETCH(QDateTime, end);
        QFETCH(bool, allDay);
        QFETCH(bool, today);
        const Event event{.title = "x", .start = start, .end = end, .allDay = allDay};

        QCOMPARE(topdo::happensOn(event, QDate(2026, 11, 21)), today);
    }

    void sortsAllDayFirstThenByStart() {
        const QList<Event> sorted = topdo::sortedDay({
            timed("Review", 15, 0, 15, 30),
            allDay("Holiday"),
            timed("Standup", 9, 30, 10, 0),
            timed("Short", 9, 30, 9, 45),
        });

        QStringList titles;
        for (const Event &event : sorted) {
            titles.append(event.title);
        }
        QCOMPARE(titles, (QStringList{"Holiday", "Short", "Standup", "Review"}));
    }

    void picksTheNextEvent_data() {
        QTest::addColumn<QDateTime>("now");
        QTest::addColumn<int>("next"); // -1: none

        QTest::newRow("before all") << at(8) << 1;
        QTest::newRow("during one") << at(9, 45) << 1;
        QTest::newRow("at its end, the next") << at(10) << 2;
        QTest::newRow("after all") << at(16) << -1;
    }

    void picksTheNextEvent() {
        QFETCH(QDateTime, now);
        QFETCH(int, next);
        const QList<Event> events = {
            allDay("Holiday"),
            timed("Standup", 9, 30, 10, 0),
            timed("Review", 15, 0, 15, 30),
        };

        QCOMPARE(topdo::nextEvent(events, now).value_or(-1), static_cast<qsizetype>(next));
    }

    void countsDown_data() {
        QTest::addColumn<QDateTime>("now");
        QTest::addColumn<QString>("text");

        QTest::newRow("minutes") << at(14, 32) << "in 28 min";
        QTest::newRow("seconds round up") << at(14, 59).addSecs(30) << "in 1 min";
        QTest::newRow("hours") << at(13) << "in 2 h";
        QTest::newRow("hours and minutes") << at(12, 55) << "in 2 h 5 min";
        QTest::newRow("started") << at(15, 10) << "now, until 15:30";
    }

    void countsDown() {
        QFETCH(QDateTime, now);
        QFETCH(QString, text);

        QCOMPARE(topdo::countdown(timed("Review", 15, 0, 15, 30), now), text);
    }
};

QTEST_GUILESS_MAIN(TestDay)
#include "tst_day.moc"
