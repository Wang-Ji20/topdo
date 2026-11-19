pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

// The project column, with sample data until roadmap milestone 5 decides where it comes from.
ColumnLayout {
    id: project

    required property Style style
    property bool active: false

    readonly property var milestones: ["Core and store", "Command line", "Plasma widget", "MCP server", "Project and today", "Grow with use"]
    readonly property int current: 2

    spacing: style.unit * 0.95

    ColumnLabel {
        Layout.fillWidth: true
        style: project.style
        number: 2
        name: "Project"
        active: project.active
        stub: true
    }

    Text {
        text: "topdo"
        color: project.style.text
        font.family: project.style.geometric
        font.weight: Font.Light
        font.pixelSize: project.style.unit * 2.9
    }

    Row {
        spacing: project.style.unit * 0.45

        Repeater {
            model: project.milestones.length

            Rectangle {
                required property int index

                width: project.style.unit * 1.2
                height: width
                color: index < project.current ? project.style.mid : "transparent"
                border.width: index === project.current ? 2 : (index < project.current ? 0 : 1)
                border.color: index === project.current ? project.style.accent : project.style.dim
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 1
        color: project.style.hair
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: project.style.unit * 0.3

        Text {
            text: `MILESTONE ${project.current + 1} OF ${project.milestones.length}`
            color: project.style.dim
            font.family: project.style.mono
            font.pixelSize: project.style.unit * 0.55
            font.letterSpacing: project.style.unit * 0.1
        }

        Text {
            Layout.fillWidth: true
            text: project.milestones[project.current]
            elide: Text.ElideRight
            color: project.style.text
            font.family: project.style.geometric
            font.pixelSize: project.style.unit * 1.1
        }

        Text {
            Layout.fillWidth: true
            text: "The overview: three columns"
            elide: Text.ElideRight
            color: project.style.mid
            font.family: project.style.mono
            font.pixelSize: project.style.unit * 0.62
        }
    }

    Item {
        Layout.fillHeight: true
    }
}
