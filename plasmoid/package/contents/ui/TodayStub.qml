pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

// The today column, with a sample day until roadmap milestone 5 decides where it comes from.
// The day runs from 08:00 to 20:00 as a line, events as blocks, now as a ring.
ColumnLayout {
    id: today

    required property Style style
    property bool active: false
    // Open, the column shows the whole day as a timeline instead of the strip.
    property bool open: false

    readonly property var events: [
        {
            start: 8.5,
            end: 9,
            what: "Email"
        },
        {
            start: 9.5,
            end: 10,
            what: "Standup"
        },
        {
            start: 10,
            end: 12,
            what: "Deep work"
        },
        {
            start: 13,
            end: 14,
            what: "Lunch"
        },
        {
            start: 14,
            end: 15,
            what: "Deep work"
        },
        {
            start: 15,
            end: 15.5,
            what: "Review"
        },
        {
            start: 17,
            end: 18,
            what: "Gym"
        },
    ]
    readonly property real now: 14.53

    function clock(hour: real): string {
        const minutes = Math.round(hour * 60);
        return `${String(Math.floor(minutes / 60)).padStart(2, "0")}:${String(minutes % 60).padStart(2, "0")}`;
    }

    spacing: style.unit * 0.95

    ColumnLabel {
        Layout.fillWidth: true
        style: today.style
        number: 3
        name: "Today"
        active: today.active
        stub: true
    }

    Text {
        text: "15:00"
        color: today.style.text
        font.family: today.style.geometric
        font.weight: Font.Light
        font.pixelSize: today.style.unit * 4.2
    }

    ColumnLayout {
        Layout.topMargin: -today.style.unit * 0.6
        spacing: today.style.unit * 0.2

        Text {
            text: "Review"
            color: today.style.text
            font.family: today.style.geometric
            font.pixelSize: today.style.unit * 1.1
        }

        Text {
            textFormat: Text.StyledText
            text: `<font color="${today.style.accent}">in 28 min</font> · with Mia`
            color: today.style.mid
            font.family: today.style.mono
            font.pixelSize: today.style.unit * 0.62
        }
    }

    Item {
        Layout.fillHeight: true
    }

    // The timeline, open: one row per event, a line at now.
    ColumnLayout {
        Layout.fillWidth: true
        visible: today.open
        spacing: 0

        Repeater {
            model: today.open ? today.events : []

            ColumnLayout {
                id: event

                required property int index
                required property var modelData
                readonly property bool past: modelData.end <= today.now
                readonly property bool current: modelData.start <= today.now && today.now < modelData.end
                readonly property bool nowBefore: today.now < modelData.start && (index === 0 || today.events[index - 1].start <= today.now)

                Layout.fillWidth: true
                spacing: 0

                RowLayout {
                    visible: event.nowBefore
                    Layout.fillWidth: true
                    spacing: today.style.unit * 0.8

                    Text {
                        Layout.preferredWidth: today.style.unit * 2.6
                        text: today.clock(today.now)
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
                        text: event.modelData.what
                        color: today.style.text
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
        }
    }

    // The day strip, at rest.
    Item {
        id: strip

        visible: !today.open
        function xOf(hour: real): real {
            return (hour - 8) / 12 * strip.width;
        }

        Layout.fillWidth: true
        implicitHeight: today.style.unit * 1.5

        Rectangle {
            y: today.style.unit * 0.6
            width: parent.width
            height: 1
            color: today.style.hair
        }

        Repeater {
            model: today.events

            Rectangle {
                required property var modelData

                x: strip.xOf(modelData.start)
                y: today.style.unit * 0.42
                width: Math.max(2, strip.xOf(modelData.end) - x)
                height: today.style.unit * 0.4
                color: modelData.start > today.now && modelData.start < 15.5 ? today.style.mid : today.style.dim
            }
        }

        Rectangle {
            x: strip.xOf(today.now) - width / 2
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
