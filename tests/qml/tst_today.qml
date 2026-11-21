import "../../plasmoid/package/contents/ui"
import QtQuick
import QtTest
import io.github.wangji20.topdo
import topdo.test

// The today column with a day of plain values instead of the KDE calendar.
// See docs/design/project-today.md.
Item {
    id: root

    function at(hour: int, minute: int): date {
        return new Date(2026, 10, 21, hour, minute);
    }

    width: 600
    height: 800

    Style {
        id: style
    }

    TodoModel {
        id: todoModel

        path: Fixture.write(`Call the bank due:${Qt.formatDate(new Date(), "yyyy-MM-dd")}\nBuy milk\n`)
    }

    QtObject {
        id: fakeDay

        property var events: [
            {
                "title": "Holiday",
                "start": root.at(0, 0),
                "end": root.at(0, 0),
                "allDay": true
            },
            {
                "title": "Standup",
                "start": root.at(9, 30),
                "end": root.at(10, 0),
                "allDay": false
            },
            {
                "title": "Review",
                "start": root.at(15, 0),
                "end": root.at(15, 30),
                "allDay": false
            }
        ]
        property date now: root.at(14, 32)
        property int next: 2
        property string countdown: "in 28 min"
    }

    TodayColumn {
        id: column

        anchors.fill: parent
        style: style
        todos: todoModel
        day: fakeDay
    }

    TestCase {
        function test_leadsWithTheNextEvent() {
            compare(column.next.title, "Review");
            compare(column.clock(column.next.start), "15:00");
            compare(column.allDay.length, 1);
            compare(column.timed.length, 2);
        }

        function test_placesHoursInTheDay() {
            compare(column.hourOf(root.at(14, 30)), 14.5);
            compare(column.hourOf(new Date(2026, 10, 20, 22, 0)), 0, "yesterday starts the day");
        }

        function test_listsTodosDueToday() {
            compare(todoModel.dueToday, ["Call the bank"]);
        }

        function test_nothingPlanned() {
            const saved = fakeDay.events;
            fakeDay.events = [];
            fakeDay.next = -1;
            compare(column.next, null);
            fakeDay.events = saved;
            fakeDay.next = 2;
        }

        name: "Today"
        when: windowShown
    }
}
