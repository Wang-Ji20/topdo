pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

// The project column, with sample data until roadmap milestone 5 decides where it comes from.
ColumnLayout {
    id: project

    required property Style style
    property bool active: false
    // Open, the column lists every milestone instead of the row of squares.
    property bool open: false

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
        visible: !project.open
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

    Repeater {
        model: project.open ? project.milestones : []

        RowLayout {
            id: milestone

            required property int index
            required property string modelData
            readonly property bool now: index === project.current

            Layout.fillWidth: true
            spacing: project.style.unit * 0.8

            Rectangle {
                implicitWidth: project.style.unit * 0.8
                implicitHeight: implicitWidth
                color: milestone.index < project.current ? project.style.mid : "transparent"
                border.width: milestone.now ? 2 : (milestone.index < project.current ? 0 : 1)
                border.color: milestone.now ? project.style.accent : project.style.dim
            }

            Text {
                Layout.fillWidth: true
                text: milestone.modelData
                elide: Text.ElideRight
                color: milestone.now ? project.style.text : project.style.mid
                font.family: project.style.geometric
                font.pixelSize: project.style.unit * 0.9
            }

            Text {
                text: milestone.index < project.current ? "DONE" : (milestone.now ? "NOW" : "")
                color: milestone.now ? project.style.accent : project.style.dim
                font.family: project.style.mono
                font.pixelSize: project.style.unit * 0.55
                font.letterSpacing: project.style.unit * 0.08
            }
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: project.style.unit * 0.3

        Text {
            text: project.open ? "NEXT STEP" : `MILESTONE ${project.current + 1} OF ${project.milestones.length}`
            color: project.style.dim
            font.family: project.style.mono
            font.pixelSize: project.style.unit * 0.55
            font.letterSpacing: project.style.unit * 0.1
        }

        Text {
            Layout.fillWidth: true
            visible: !project.open
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

    Text {
        visible: project.open
        text: "last commit 68596dc feat: the open todo column"
        elide: Text.ElideRight
        Layout.fillWidth: true
        color: project.style.dim
        font.family: project.style.mono
        font.pixelSize: project.style.unit * 0.6
    }

    Item {
        Layout.fillHeight: true
    }
}
