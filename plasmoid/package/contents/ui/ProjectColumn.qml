pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import io.github.wangji20.topdo

// The project column: one +project of the todo file. At rest, its name, a square per milestone
// and the current milestone with its next step. Open, every milestone with the current one's
// steps; ↑ ↓ move the highlight, Enter ticks, Page Up and Page Down pick another project.
// See docs/design/project-today.md.
FocusScope {
    id: column

    objectName: "projectColumn"

    required property Style style
    required property TodoModel todos
    required property ProjectModel project
    property bool active: false
    property bool open: false
    property int highlight: 0

    // The open view's rows: each milestone, and under the current one its steps.
    readonly property var rows: {
        const rows = [];
        column.project.milestones.forEach((milestone, index) => {
            rows.push({
                number: milestone.number,
                title: milestone.title,
                done: milestone.done,
                step: false,
                now: index === column.project.current
            });
            if (index === column.project.current) {
                milestone.steps.forEach(step => rows.push({
                        number: step.number,
                        title: step.title,
                        done: step.done,
                        step: true,
                        now: false
                    }));
            }
        });
        return rows;
    }
    readonly property var currentMilestone: project.current >= 0 ? project.milestones[project.current] : null
    readonly property var nextStep: currentMilestone ? currentMilestone.steps.find(step => !step.done) ?? null : null

    // Asks for the project `by` places on from the one shown, wrapping around.
    signal projectPicked(string name)

    function pickProject(by: int) {
        const names = column.project.projects;
        if (names.length === 0) {
            return;
        }
        const at = names.indexOf(column.project.name);
        column.projectPicked(names[(at + by + names.length) % names.length]);
        column.highlight = 0;
    }

    function move(by: int) {
        column.highlight = Math.max(0, Math.min(column.highlight + by, column.rows.length - 1));
    }

    function tick() {
        const row = column.rows[column.highlight];
        if (row) {
            column.todos.setDone(row.number, !row.done);
        }
    }

    // The column stays loaded when it opens, so it takes the keys itself.
    onOpenChanged: {
        if (open) {
            column.highlight = 0;
            column.forceActiveFocus();
        }
    }
    Keys.onPressed: event => {
        const ctrl = (event.modifiers & Qt.ControlModifier) !== 0;
        if (!column.open) {
            return;
        }
        if (event.key === Qt.Key_Down || (ctrl && event.key === Qt.Key_J)) {
            column.move(1);
        } else if (event.key === Qt.Key_Up || (ctrl && event.key === Qt.Key_K)) {
            column.move(-1);
        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            column.tick();
        } else if (event.key === Qt.Key_PageDown) {
            column.pickProject(1);
        } else if (event.key === Qt.Key_PageUp) {
            column.pickProject(-1);
        } else {
            return; // on to the panel
        }
        event.accepted = true;
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: column.style.unit * 0.95

        ColumnLabel {
            visible: !column.open // the panel's tabs show it
            Layout.fillWidth: true
            style: column.style
            number: 2
            name: "Project"
            active: column.active
        }

        Text {
            Layout.fillWidth: true
            text: column.project.name || "no project"
            elide: Text.ElideRight
            color: column.project.name ? column.style.text : column.style.dim
            font.family: column.style.geometric
            font.weight: Font.Light
            font.pixelSize: column.style.unit * 2.9
        }

        Text {
            Layout.fillWidth: true
            visible: column.project.name === ""
            text: "Add +Name to a todo to start a project."
            wrapMode: Text.Wrap
            color: column.style.dim
            font.family: column.style.mono
            font.pixelSize: column.style.unit * 0.62
        }

        // One square per milestone: filled when done, outlined in the accent colour when current.
        Flow {
            Layout.fillWidth: true
            visible: !column.open && column.project.milestones.length > 0
            spacing: column.style.unit * 0.45

            Repeater {
                model: column.project.milestones

                Rectangle {
                    required property int index
                    required property var modelData
                    readonly property bool now: index === column.project.current

                    width: column.style.unit * 1.2
                    height: width
                    color: modelData.done ? column.style.mid : "transparent"
                    border.width: now ? 2 : (modelData.done ? 0 : 1)
                    border.color: now ? column.style.accent : column.style.dim
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: column.style.hair
        }

        ColumnLayout {
            Layout.fillWidth: true
            visible: !column.open && column.currentMilestone !== null
            spacing: column.style.unit * 0.3

            Text {
                text: `MILESTONE ${column.project.current + 1} OF ${column.project.milestones.length}`
                color: column.style.dim
                font.family: column.style.mono
                font.pixelSize: column.style.unit * 0.55
                font.letterSpacing: column.style.unit * 0.1
            }

            Text {
                Layout.fillWidth: true
                text: column.currentMilestone ? column.currentMilestone.title : ""
                elide: Text.ElideRight
                color: column.style.text
                font.family: column.style.geometric
                font.pixelSize: column.style.unit * 1.1
            }

            Text {
                Layout.fillWidth: true
                visible: column.nextStep !== null
                text: column.nextStep ? column.nextStep.title : ""
                elide: Text.ElideRight
                color: column.style.mid
                font.family: column.style.mono
                font.pixelSize: column.style.unit * 0.62
            }
        }

        Text {
            visible: !column.open && column.project.name !== "" && column.currentMilestone === null
            text: "ALL DONE"
            color: column.style.dim
            font.family: column.style.mono
            font.pixelSize: column.style.unit * 0.55
            font.letterSpacing: column.style.unit * 0.1
        }

        ListView {
            id: list

            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: column.open
            clip: true
            model: column.open ? column.rows : []
            boundsBehavior: Flickable.StopAtBounds
            currentIndex: column.highlight

            delegate: Item {
                id: row

                required property int index
                required property var modelData
                readonly property bool highlighted: index === column.highlight

                width: ListView.view.width
                height: cells.implicitHeight + column.style.unit * 0.6

                Rectangle {
                    anchors.fill: parent
                    radius: column.style.unit * 0.4
                    color: row.highlighted ? column.style.withAlpha(column.style.accent, 0.16) : "transparent"
                }

                RowLayout {
                    id: cells

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: column.style.unit * (row.modelData.step ? 1.6 : 0.45)
                    anchors.rightMargin: column.style.unit * 0.45
                    spacing: column.style.unit * 0.8

                    // A square for a milestone, a circle for a step.
                    Rectangle {
                        implicitWidth: column.style.unit * (row.modelData.step ? 0.55 : 0.8)
                        implicitHeight: implicitWidth
                        radius: row.modelData.step ? width / 2 : 0
                        color: row.modelData.done ? column.style.mid : "transparent"
                        border.width: row.modelData.now ? 2 : (row.modelData.done ? 0 : 1)
                        border.color: row.modelData.now ? column.style.accent : column.style.dim
                    }

                    Text {
                        Layout.fillWidth: true
                        text: row.modelData.title
                        elide: Text.ElideRight
                        color: row.modelData.now ? column.style.text : column.style.mid
                        font.strikeout: row.modelData.step && row.modelData.done
                        font.family: column.style.geometric
                        font.pixelSize: column.style.unit * (row.modelData.step ? 0.8 : 0.9)
                    }

                    Text {
                        visible: !row.modelData.step
                        text: row.modelData.done ? "DONE" : (row.modelData.now ? "NOW" : "")
                        color: row.modelData.now ? column.style.accent : column.style.dim
                        font.family: column.style.mono
                        font.pixelSize: column.style.unit * 0.55
                        font.letterSpacing: column.style.unit * 0.08
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        column.highlight = row.index;
                        column.tick();
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
            visible: !column.open
        }
    }
}
