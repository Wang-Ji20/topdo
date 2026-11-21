pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import io.github.wangji20.topdo

// The open todo column: an input line that filters the list and adds todos, and the list, where
// the highlighted row shows its actions. See docs/design/widget.md, "Panel: an expanded card".
// Keys it doesn't use (Tab, Esc on an empty input, arrows on an empty input) go on to the panel.
FocusScope {
    id: column

    required property Style style
    required property TodoModel todos
    // The highlighted row: a todo of the list, or, one past them, the add row.
    property int highlight: 0
    // The number of the todo being edited in the input line, or -1.
    property int editing: -1
    // The number of the todo the input line writes a new subtask for, or -1.
    property int subtaskOf: -1
    // Editing or writing a subtask: the input line holds text for one todo and doesn't filter.
    readonly property bool busy: editing >= 0 || subtaskOf >= 0

    readonly property string typed: input.text.trim()
    readonly property bool hasAddRow: !busy && typed !== ""
    readonly property int rowCount: list.count + (hasAddRow ? 1 : 0)

    function roleAt(row: int, role: int): var {
        return column.todos.data(column.todos.index(row, 0), role);
    }

    function move(by: int) {
        column.highlight = Math.max(0, Math.min(column.highlight + by, column.rowCount - 1));
        list.positionViewAtIndex(Math.min(column.highlight, list.count - 1), ListView.Contain);
    }

    function add() {
        if (column.typed === "") {
            return;
        }
        column.todos.add(column.typed);
        input.clear();
        column.highlight = 0;
    }

    // Enter: ticks or unticks the highlighted todo, or adds on the add row.
    function activate() {
        if (column.highlight >= list.count) {
            column.add();
            return;
        }
        const number = column.roleAt(column.highlight, TodoModel.NumberRole);
        const done = column.roleAt(column.highlight, TodoModel.DoneRole);
        column.todos.setDone(number, !done);
        column.move(0);
    }

    function startEditing() {
        if (column.highlight >= list.count) {
            return;
        }
        const text = column.roleAt(column.highlight, TodoModel.TextRole);
        column.editing = column.roleAt(column.highlight, TodoModel.NumberRole);
        input.text = text;
    }

    function stopEditing(save: bool) {
        if (save) {
            column.todos.setText(column.editing, input.text);
        }
        column.editing = -1;
        input.clear();
    }

    function startSubtask() {
        if (column.highlight >= list.count) {
            return;
        }
        column.subtaskOf = column.roleAt(column.highlight, TodoModel.NumberRole);
        input.clear();
    }

    function stopSubtask(add: bool) {
        if (add) {
            column.todos.addSubtask(column.subtaskOf, input.text);
        }
        column.subtaskOf = -1;
        input.clear();
    }

    function remove() {
        if (column.highlight >= list.count) {
            return;
        }
        column.todos.remove(column.roleAt(column.highlight, TodoModel.NumberRole));
        column.move(0);
    }

    // The title with the filter text underlined.
    function marked(title: string): string {
        const escaped = s => s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
        const at = column.typed === "" ? -1 : title.toLowerCase().indexOf(column.typed.toLowerCase());
        if (at < 0) {
            return escaped(title);
        }
        const end = at + column.typed.length;
        return escaped(title.slice(0, at)) + "<u>" + escaped(title.slice(at, end)) + "</u>" + escaped(title.slice(end));
    }

    // The model's filter follows what is typed; the overview wants the whole list back.
    Binding {
        target: column.todos
        property: "filter"
        value: column.busy ? "" : column.typed
    }

    Component.onDestruction: column.todos.filter = ""
    onTypedChanged: column.highlight = list.count > 0 || !column.hasAddRow ? 0 : list.count

    ColumnLayout {
        anchors.fill: parent
        spacing: column.style.unit * 0.8

        ColumnLabel {
            Layout.fillWidth: true
            style: column.style
            number: 1
            name: "Todo"
            active: true
        }

        // The input line.
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: prompt.implicitHeight + column.style.unit * 1.2
            radius: column.style.unit * 0.55
            color: column.style.withAlpha("black", 0.22)
            border.color: column.style.withAlpha(column.style.text, 0.08)

            RowLayout {
                id: prompt

                anchors.fill: parent
                anchors.leftMargin: column.style.unit * 0.8
                anchors.rightMargin: column.style.unit * 0.8
                spacing: column.style.unit * 0.65

                // Its mode: a circle for todos, a square while editing one.
                Rectangle {
                    implicitWidth: column.style.unit * 0.55
                    implicitHeight: implicitWidth
                    radius: column.editing < 0 ? width / 2 : 0 // a subtask is a todo: a circle
                    color: "transparent"
                    border.width: 2
                    border.color: column.style.accent
                }

                TextInput {
                    id: input

                    Layout.fillWidth: true
                    focus: true
                    clip: true
                    color: column.style.text
                    selectionColor: column.style.accent
                    font.family: column.style.geometric
                    font.pixelSize: column.style.unit * 0.95

                    Keys.onPressed: event => {
                        const ctrl = (event.modifiers & Qt.ControlModifier) !== 0;
                        const shift = (event.modifiers & Qt.ShiftModifier) !== 0;
                        const enter = event.key === Qt.Key_Return || event.key === Qt.Key_Enter;
                        if (column.editing >= 0) {
                            if (enter) {
                                column.stopEditing(true);
                            } else if (event.key === Qt.Key_Escape) {
                                column.stopEditing(false);
                            } else if (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab) {
                                // no switching columns in the middle of an edit
                            } else {
                                return;
                            }
                        } else if (column.subtaskOf >= 0) {
                            if (enter) {
                                column.stopSubtask(true);
                            } else if (event.key === Qt.Key_Escape) {
                                column.stopSubtask(false);
                            } else if (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab) {
                                // no switching columns in the middle of a new subtask
                            } else {
                                return;
                            }
                        } else if (event.key === Qt.Key_Down || (ctrl && event.key === Qt.Key_J)) {
                            column.move(1);
                        } else if (event.key === Qt.Key_Up || (ctrl && event.key === Qt.Key_K)) {
                            column.move(-1);
                        } else if (enter && ctrl) {
                            column.add();
                        } else if (enter && shift) {
                            column.startSubtask();
                        } else if (enter) {
                            column.activate();
                        } else if (ctrl && event.key === Qt.Key_E) {
                            column.startEditing();
                        } else if (shift && event.key === Qt.Key_Delete) {
                            column.remove();
                        } else if (shift && event.key === Qt.Key_Backspace && input.text === "") {
                            // For keyboards without Delete. With text typed, it deletes a character:
                            // Shift is often still down from typing a capital.
                            column.remove();
                        } else if (ctrl && event.key === Qt.Key_Z) {
                            // Undoes a removal, not typing: the input holds a few words at most.
                            column.todos.undoRemove();
                            column.move(0);
                        } else if (ctrl && event.key === Qt.Key_H) {
                            column.todos.showDone = !column.todos.showDone;
                        } else if (event.key === Qt.Key_Escape && input.text !== "") {
                            input.clear();
                        } else if ((event.key === Qt.Key_Left || event.key === Qt.Key_Right) && input.text !== "") {
                            // With text typed, the arrows belong to the cursor, even at either end.
                            const by = event.key === Qt.Key_Left ? -1 : 1;
                            input.cursorPosition = Math.max(0, Math.min(input.cursorPosition + by, input.length));
                        } else {
                            return; // on to the text, or up to the panel
                        }
                        event.accepted = true;
                    }

                    Text {
                        anchors.fill: parent
                        visible: input.text === ""
                        text: "filter, or write a new todo"
                        color: column.style.dim
                        font: input.font
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Text {
                    visible: column.busy
                    text: column.editing >= 0 ? `EDITING ${column.editing}` : `SUBTASK OF ${column.subtaskOf}`
                    color: column.style.dim
                    font.family: column.style.mono
                    font.pixelSize: column.style.unit * 0.55
                    font.letterSpacing: column.style.unit * 0.08
                }
            }
        }

        ListView {
            id: list

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: column.todos
            currentIndex: column.highlight < count ? column.highlight : -1
            boundsBehavior: Flickable.StopAtBounds
            section.property: "section"
            section.delegate: Text {
                required property string section

                width: ListView.view.width
                visible: column.typed === "" && section !== ""
                height: visible ? implicitHeight + column.style.unit * 0.9 : 0
                verticalAlignment: Text.AlignBottom
                bottomPadding: column.style.unit * 0.25
                leftPadding: column.style.unit * 0.45
                text: section.toUpperCase()
                color: column.style.dim
                font.family: column.style.mono
                font.pixelSize: column.style.unit * 0.55
                font.letterSpacing: column.style.unit * 0.1
            }

            delegate: Item {
                id: row

                required property int index
                required property int number
                required property string title
                required property bool done
                required property int depth
                required property var due
                required property list<string> tags
                readonly property bool highlighted: index === column.highlight
                readonly property var dueLabel: column.style.dueLabel(due)

                width: ListView.view.width
                height: cells.implicitHeight + column.style.unit * 0.75

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
                    anchors.leftMargin: column.style.unit * 0.45
                    anchors.rightMargin: column.style.unit * 0.45
                    spacing: column.style.unit * 0.6

                    Text {
                        Layout.preferredWidth: column.style.unit * 1.2
                        horizontalAlignment: Text.AlignRight
                        text: row.number
                        color: column.style.dim
                        font.family: column.style.mono
                        font.pixelSize: column.style.unit * 0.55
                    }

                    // Subtasks: indented, with a short line before the circle.
                    Rectangle {
                        visible: row.depth > 0
                        Layout.leftMargin: (row.depth - 1) * column.style.unit * 1.1
                        implicitWidth: column.style.unit * 0.55
                        implicitHeight: 1
                        color: column.style.dim
                    }

                    Rectangle {
                        implicitWidth: column.style.unit * 0.55
                        implicitHeight: implicitWidth
                        radius: width / 2
                        color: row.done ? column.style.mid : "transparent"
                        border.width: 1.5
                        border.color: column.style.mid
                    }

                    Text {
                        Layout.fillWidth: true
                        textFormat: Text.StyledText
                        text: column.marked(row.title)
                        wrapMode: Text.Wrap
                        color: row.done ? column.style.dim : column.style.text
                        font.strikeout: row.done
                        font.family: column.style.geometric
                        font.pixelSize: column.style.unit * 0.85
                    }

                    // Contexts and other projects, small: they belong to the todo but aren't its
                    // title. The keys are in the panel's hint line, so nothing here changes with
                    // the highlight and the title never reflows.
                    Text {
                        visible: row.tags.length > 0
                        text: row.tags.join("  ")
                        color: column.style.dim
                        font.family: column.style.mono
                        font.pixelSize: column.style.unit * 0.55
                    }

                    Text {
                        visible: text !== ""
                        text: row.dueLabel.text
                        color: row.dueLabel.colour
                        font.family: column.style.mono
                        font.pixelSize: column.style.unit * 0.6
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: column.highlight = row.index
                    onClicked: {
                        column.highlight = row.index;
                        column.activate();
                        input.forceActiveFocus();
                    }
                }
            }

            footer: ColumnLayout {
                width: ListView.view.width
                spacing: 0

                // The add row, last, while something is typed.
                Item {
                    id: addRow

                    readonly property bool highlighted: column.hasAddRow && column.highlight === list.count

                    Layout.fillWidth: true
                    visible: column.hasAddRow
                    implicitHeight: addText.implicitHeight + column.style.unit * 0.75

                    Rectangle {
                        anchors.fill: parent
                        radius: column.style.unit * 0.4
                        color: addRow.highlighted ? column.style.withAlpha(column.style.accent, 0.16) : "transparent"
                    }

                    Text {
                        id: addText

                        anchors.verticalCenter: parent.verticalCenter
                        x: column.style.unit * 2.25 + column.style.unit * 1.2
                        width: parent.width - x
                        elide: Text.ElideRight
                        text: `add “${column.typed}”`
                        color: column.style.accent
                        font.family: column.style.geometric
                        font.pixelSize: column.style.unit * 0.85
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            column.add();
                            input.forceActiveFocus();
                        }
                    }
                }

                Text {
                    visible: column.typed === "" && column.todos.doneCount > 0
                    topPadding: column.style.unit * 0.6
                    leftPadding: column.style.unit * 0.45
                    text: `${column.todos.showDone ? "▾" : "▸"} ${column.todos.doneCount} done`
                    color: column.style.dim
                    font.family: column.style.mono
                    font.pixelSize: column.style.unit * 0.62

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            column.todos.showDone = !column.todos.showDone;
                            input.forceActiveFocus();
                        }
                    }
                }
            }
        }
    }
}
