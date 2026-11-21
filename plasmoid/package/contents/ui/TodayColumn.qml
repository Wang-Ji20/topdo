pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import io.github.wangji20.topdo

// The today column: the day's events from the KDE calendar. At rest, the next event large with a
// countdown, and the day as a strip from 08:00 to 20:00 with a ring at now. Open, one row per
// event with a line at now, then the todos due today. See docs/design/project-today.md.
ColumnLayout {
    id: today

    required property Style style
    required property TodoModel todos
    // A TodayModel, or in tests any object with its properties: events, now, next, countdown.
    required property var day
    property bool active: false
    property bool open: false

    readonly property var timed: day.events.filter(event => !event.allDay)
    readonly property var allDay: day.events.filter(event => event.allDay)
    readonly property var next: day.next >= 0 ? day.events[day.next] : null

    // Hours since today's midnight, clamped to the day: an event from yesterday starts at 0.
    function hourOf(when: date): real {
        const midnight = new Date(today.day.now);
        midnight.setHours(0, 0, 0, 0);
        const hours = (when.getTime() - midnight.getTime()) / 3600000;
        return Math.max(0, Math.min(24, hours));
    }

    function clock(when: date): string {
        return Qt.formatTime(when, "HH:mm");
    }

    spacing: style.unit * 0.95

    ColumnLabel {
        visible: !today.open // the panel's tabs show it
        Layout.fillWidth: true
        style: today.style
        number: 3
        name: "Today"
        active: today.active
    }

    Text {
        text: today.next ? today.clock(today.next.start) : "—"
        color: today.next ? today.style.text : today.style.dim
        font.family: today.style.geometric
        font.weight: Font.Light
        font.pixelSize: today.style.unit * 4.2
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.topMargin: -today.style.unit * 0.6
        spacing: today.style.unit * 0.2

        Text {
            Layout.fillWidth: true
            text: today.next ? today.next.title : (today.timed.length > 0 ? "nothing more today" : "nothing planned")
            elide: Text.ElideRight
            color: today.next ? today.style.text : today.style.mid
            font.family: today.style.geometric
            font.pixelSize: today.style.unit * 1.1
        }

        Text {
            visible: today.day.countdown !== ""
            text: today.day.countdown
            color: today.style.accent
            font.family: today.style.mono
            font.pixelSize: today.style.unit * 0.62
        }

        Text {
            Layout.fillWidth: true
            visible: today.allDay.length > 0
            text: "all day · " + today.allDay.map(event => event.title).join(" · ")
            elide: Text.ElideRight
            color: today.style.mid
            font.family: today.style.mono
            font.pixelSize: today.style.unit * 0.62
        }
    }

    Item {
        Layout.fillHeight: true
        visible: !today.open
    }

    // The timeline, open: one row per timed event, a line at now.
    ListView {
        id: timeline

        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: today.open
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        model: today.open ? today.timed : []

        delegate: ColumnLayout {
            id: event

            required property int index
            required property var modelData
            readonly property bool past: modelData.end <= today.day.now
            readonly property bool current: modelData.start <= today.day.now && today.day.now < modelData.end
            readonly property bool nowBefore: today.day.now < modelData.start && (index === 0 || today.timed[index - 1].start <= today.day.now)

            width: ListView.view.width
            spacing: 0

            RowLayout {
                visible: event.nowBefore
                Layout.fillWidth: true
                spacing: today.style.unit * 0.8

                Text {
                    Layout.preferredWidth: today.style.unit * 2.6
                    text: today.clock(today.day.now)
                    color: today.style.accent
                    font.family: today.style.mono
                    font.pixelSize: today.style.unit * 0.6
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 1
                    color: today.style.accent
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: today.style.unit * 0.35
                Layout.bottomMargin: today.style.unit * 0.35
                opacity: event.past ? 0.6 : 1
                spacing: today.style.unit * 0.8

                Text {
                    Layout.preferredWidth: today.style.unit * 2.6
                    text: today.clock(event.modelData.start)
                    color: event.current ? today.style.accent : today.style.dim
                    font.family: today.style.mono
                    font.pixelSize: today.style.unit * 0.6
                }

                Text {
                    Layout.fillWidth: true
                    text: event.modelData.title
                    elide: Text.ElideRight
                    color: event.current ? today.style.accent : today.style.text
                    font.family: today.style.geometric
                    font.pixelSize: today.style.unit * 0.9
                }

                Text {
                    text: `${today.clock(event.modelData.start)}–${today.clock(event.modelData.end)}`
                    color: today.style.dim
                    font.family: today.style.mono
                    font.pixelSize: today.style.unit * 0.55
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 1
                color: today.style.hair
            }
        }

        footer: ColumnLayout {
            width: ListView.view.width
            spacing: today.style.unit * 0.3

            Text {
                visible: today.todos.dueToday.length > 0
                topPadding: today.style.unit * 0.8
                text: "DUE TODAY"
                color: today.style.dim
                font.family: today.style.mono
                font.pixelSize: today.style.unit * 0.55
                font.letterSpacing: today.style.unit * 0.1
            }

            Repeater {
                model: today.todos.dueToday

                Text {
                    required property string modelData

                    Layout.fillWidth: true
                    text: "○  " + modelData
                    elide: Text.ElideRight
                    color: today.style.mid
                    font.family: today.style.geometric
                    font.pixelSize: today.style.unit * 0.8
                }
            }
        }
    }

    // The day strip, at rest.
    Item {
        id: strip

        function xOf(hour: real): real {
            return Math.max(0, Math.min(1, (hour - 8) / 12)) * strip.width;
        }

        visible: !today.open
        Layout.fillWidth: true
        implicitHeight: today.style.unit * 1.5

        Rectangle {
            y: today.style.unit * 0.6
            width: parent.width
            height: 1
            color: today.style.hair
        }

        Repeater {
            model: today.timed

            Rectangle {
                required property var modelData

                x: strip.xOf(today.hourOf(modelData.start))
                y: today.style.unit * 0.42
                width: Math.max(2, strip.xOf(today.hourOf(modelData.end)) - x)
                height: today.style.unit * 0.4
                color: modelData.end > today.day.now ? today.style.mid : today.style.dim
            }
        }

        Rectangle {
            x: strip.xOf(today.hourOf(today.day.now)) - width / 2
            y: today.style.unit * 0.25
            width: today.style.unit * 0.75
            height: width
            radius: width / 2
            color: today.style.glass
            border.width: 2
            border.color: today.style.accent
        }

        Repeater {
            model: [8, 14, 20]

            Text {
                required property int modelData

                x: Math.min(strip.xOf(modelData), strip.width - width)
                y: today.style.unit * 0.95
                text: String(modelData).padStart(2, "0")
                color: today.style.dim
                font.family: today.style.mono
                font.pixelSize: today.style.unit * 0.48
            }
        }
    }
}
