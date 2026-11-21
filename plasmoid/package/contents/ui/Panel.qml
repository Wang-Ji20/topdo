pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import io.github.wangji20.topdo

// The panel: one glass slab with a metal edge, divided by hairlines into three columns, and a
// line of key hints. It opens as an overview; an open column is wider and the slab grows to
// `fullHeight`. See docs/design/widget.md, "Panel".
FocusScope {
    id: panel

    required property TodoModel todos
    required property ProjectModel project
    // A TodayModel, or in tests any object with its properties.
    required property var day
    // The column picked in the overview, and the open column (-1: none).
    property int focusColumn: 0
    property int openColumn: -1
    // The height of the slab while a column is open: the screen below the top bar.
    property real fullHeight: theme.unit * 40
    // Whether the open todo column is editing a todo; the hint line follows it.
    property bool todoEditing: false
    // Whether the open todo column is writing a new subtask; the hint line follows it.
    property bool todoAddingSubtask: false

    signal closeRequested
    // The project column asks to show another project; main.qml remembers it.
    signal projectPicked(string name)

    // Opens a column, or with -1 goes back to the overview.
    function open(column: int) {
        if (column >= 0) {
            panel.focusColumn = column;
        }
        panel.openColumn = column;
        if (column < 0) {
            panel.forceActiveFocus(); // the overview's keys are the panel's own
        }
    }

    // Steps one column left (-1) or right (+1), wrapping around.
    function step(by: int) {
        const next = (panel.focusColumn + by + 3) % 3;
        if (panel.openColumn >= 0) {
            panel.open(next);
        } else {
            panel.focusColumn = next;
        }
    }

    // In the overview the three columns share the width; an open column takes all of it, and
    // the tabs above it show the others.
    function weight(column: int): real {
        if (panel.openColumn < 0) {
            return 1;
        }
        return column === panel.openColumn ? 1 : 0;
    }

    implicitWidth: theme.unit * 62
    implicitHeight: openColumn < 0 ? theme.unit * 25 : fullHeight
    focus: true

    Keys.onPressed: event => {
        const overview = panel.openColumn < 0;
        if (event.key === Qt.Key_Left || event.key === Qt.Key_Backtab) {
            panel.step(-1);
        } else if (event.key === Qt.Key_Right || event.key === Qt.Key_Tab) {
            panel.step(1);
        } else if (event.key >= Qt.Key_1 && event.key <= Qt.Key_3) {
            panel.open(event.key - Qt.Key_1);
        } else if (overview && (event.key === Qt.Key_Return || event.key === Qt.Key_Enter)) {
            panel.open(panel.focusColumn);
        } else if (event.key === Qt.Key_Escape) {
            if (overview) {
                panel.closeRequested();
            } else {
                panel.open(-1);
            }
        } else {
            return;
        }
        event.accepted = true;
    }

    Style {
        id: theme
    }

    // The metal edge: light at the top, darker below, under the glass.
    Rectangle {
        id: edge

        anchors.fill: parent
        radius: theme.radius

        gradient: Gradient {
            GradientStop {
                position: 0
                color: theme.withAlpha(theme.text, 0.42)
            }

            GradientStop {
                position: 0.45
                color: theme.withAlpha(theme.text, 0.07)
            }

            GradientStop {
                position: 1
                color: theme.withAlpha(theme.text, 0.2)
            }
        }
    }

    Rectangle {
        id: glass

        anchors.fill: parent
        anchors.margins: 1
        radius: theme.radius - 1
        color: theme.glass

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // The tabs, while a column is open: each column's label; a click opens it.
            Row {
                id: tabs

                visible: panel.openColumn >= 0
                Layout.fillWidth: true

                Repeater {
                    model: ["Todo", "Project", "Today"]

                    Item {
                        id: tab

                        required property int index
                        required property string modelData

                        width: tabs.width / 3
                        height: tabLabel.implicitHeight + theme.unit * 1.6

                        Rectangle {
                            anchors.fill: parent
                            visible: panel.openColumn === tab.index
                            color: theme.withAlpha(theme.text, 0.05)
                        }

                        Rectangle {
                            visible: tab.index > 0
                            width: 1
                            height: parent.height
                            color: theme.hair
                        }

                        ColumnLabel {
                            id: tabLabel

                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: theme.unit * 1.4
                            anchors.rightMargin: theme.unit * 1.4
                            style: theme
                            number: tab.index + 1
                            name: tab.modelData
                            active: panel.openColumn === tab.index
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: panel.open(tab.index)
                        }
                    }
                }
            }

            Rectangle {
                visible: panel.openColumn >= 0
                Layout.fillWidth: true
                implicitHeight: 1
                color: theme.hair
            }

            Row {
                id: columns

                Layout.fillWidth: true
                Layout.fillHeight: true

                Repeater {
                    model: 3

                    Item {
                        id: column

                        required property int index
                        readonly property bool picked: panel.openColumn === index || (panel.openColumn < 0 && panel.focusColumn === index)

                        width: columns.width * panel.weight(index) / (panel.weight(0) + panel.weight(1) + panel.weight(2))
                        height: columns.height
                        visible: panel.weight(index) > 0
                        clip: true

                        Rectangle {
                            visible: column.index > 0 && panel.openColumn < 0
                            width: 1
                            height: parent.height
                            color: theme.hair
                        }

                        Loader {
                            anchors.fill: parent
                            anchors.margins: theme.unit * 1.4
                            focus: panel.openColumn === column.index
                            sourceComponent: column.index === 0 && panel.openColumn === 0 ? todoOpen : [todoOverview, projectColumn, todayColumn][column.index]
                            onLoaded: {
                                if (panel.openColumn === column.index) {
                                    (item as Item).forceActiveFocus();
                                    fadeIn.restart();
                                }
                            }

                            // Sizes change at once: animating them resizes the window every
                            // frame. Only the opened column's content fades in, on the render
                            // thread.
                            OpacityAnimator {
                                id: fadeIn

                                target: column
                                from: 0.4
                                to: 1
                                duration: 120
                                easing.type: Easing.OutQuad
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            enabled: panel.openColumn !== column.index
                            onClicked: panel.open(column.index)
                        }

                        Component {
                            id: todoOverview

                            TodoOverview {
                                style: theme
                                todos: panel.todos
                                active: column.picked
                            }
                        }

                        Component {
                            id: todoOpen

                            TodoOpen {
                                focus: true
                                style: theme
                                todos: panel.todos
                                onEditingChanged: panel.todoEditing = editing >= 0
                                onSubtaskOfChanged: panel.todoAddingSubtask = subtaskOf >= 0
                                Component.onDestruction: {
                                    panel.todoEditing = false;
                                    panel.todoAddingSubtask = false;
                                }
                            }
                        }

                        Component {
                            id: projectColumn

                            ProjectColumn {
                                focus: true
                                style: theme
                                todos: panel.todos
                                project: panel.project
                                active: column.picked
                                open: panel.openColumn === column.index
                                onProjectPicked: name => panel.projectPicked(name)
                            }
                        }

                        Component {
                            id: todayColumn

                            TodayColumn {
                                style: theme
                                todos: panel.todos
                                day: panel.day
                                active: column.picked
                                open: panel.openColumn === column.index
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 1
                color: theme.hair
            }

            Text {
                Layout.fillWidth: true
                Layout.margins: theme.unit * 0.6
                Layout.leftMargin: theme.unit * 1.4
                text: {
                    if (panel.openColumn < 0) {
                        return "← →  pick     1 2 3  open     ↵  open     esc  close";
                    }
                    if (panel.openColumn === 1) {
                        return "↑ ↓  pick     ↵  tick     ⇞ ⇟  other project     ← → ⇥  other columns     esc  back";
                    }
                    if (panel.openColumn !== 0) {
                        return "← → ⇥  other columns     esc  back";
                    }
                    if (panel.todoEditing) {
                        return "↵  save     esc  cancel";
                    }
                    if (panel.todoAddingSubtask) {
                        return "↵  add subtask     esc  cancel";
                    }
                    return "↵  tick / add     ⌃↵  add     ⇧↵  subtask     ⌃E  edit     ⇧⌦  remove     ⌃Z  undo remove     ⌃H  done     ⇥  next column     esc  back";
                }
                color: theme.dim
                font.family: theme.mono
                font.pixelSize: theme.unit * 0.6
            }
        }
    }
}
