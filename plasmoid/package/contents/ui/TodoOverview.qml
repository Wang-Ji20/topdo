pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import io.github.wangji20.topdo

// The todo column at rest: the number of open todos large, with a ring for the share done; then
// the next todo; then a few more, small. Nothing here is editable. See docs/design/widget.md.
ColumnLayout {
    id: overview

    required property Style style
    required property TodoModel todos
    property bool active: false

    // Up to three open todos other than the next one, for the quiet list at the bottom.
    readonly property var quiet: {
        overview.todos.totalCount; // re-evaluate when the todos change
        overview.todos.nextTitle;
        const titles = [];
        for (let row = 0; row < overview.todos.rowCount() && titles.length < 3; ++row) {
            const title = overview.todos.data(overview.todos.index(row, 0), TodoModel.TitleRole);
            if (title !== overview.todos.nextTitle) {
                titles.push(title);
            }
        }
        return titles;
    }
    readonly property var nextDue: style.dueLabel(todos.nextDue)

    spacing: style.unit * 0.95

    ColumnLabel {
        Layout.fillWidth: true
        style: overview.style
        number: 1
        name: "Todo"
        active: overview.active
    }

    RowLayout {
        spacing: overview.style.unit

        Ring {
            implicitWidth: overview.style.unit * 3.2
            implicitHeight: implicitWidth
            fraction: overview.todos.totalCount > 0 ? overview.todos.doneCount / overview.todos.totalCount : 0
            track: overview.style.withAlpha(overview.style.text, 0.2)
            colour: overview.style.accent
        }

        Text {
            text: overview.todos.openCount
            color: overview.style.text
            font.family: overview.style.geometric
            font.weight: Font.Light
            font.pixelSize: overview.style.unit * 4.2
        }
    }

    Text {
        Layout.topMargin: -overview.style.unit * 0.6
        textFormat: Text.StyledText
        text: {
            const late = overview.todos.lateCount > 0 ? ` · <font color="${overview.style.late}">${overview.todos.lateCount} late</font>` : "";
            return `open · ${overview.todos.doneCount} done${late}`;
        }
        color: overview.style.mid
        font.family: overview.style.mono
        font.pixelSize: overview.style.unit * 0.62
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: 1
        color: overview.style.hair
    }

    ColumnLayout {
        Layout.fillWidth: true
        visible: overview.todos.nextTitle !== ""
        spacing: overview.style.unit * 0.3

        Text {
            text: "NEXT"
            color: overview.style.dim
            font.family: overview.style.mono
            font.pixelSize: overview.style.unit * 0.55
            font.letterSpacing: overview.style.unit * 0.1
        }

        Text {
            Layout.fillWidth: true
            text: overview.todos.nextTitle
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
            color: overview.style.text
            font.family: overview.style.geometric
            font.pixelSize: overview.style.unit * 1.1
        }

        Text {
            visible: text !== ""
            text: overview.nextDue.text
            color: overview.nextDue.colour
            font.family: overview.style.mono
            font.pixelSize: overview.style.unit * 0.62
        }
    }

    Item {
        Layout.fillHeight: true
    }

    // The quiet list: small and close together, under the next todo.
    ColumnLayout {
        Layout.fillWidth: true
        spacing: overview.style.unit * 0.35

        Repeater {
            model: overview.quiet

            RowLayout {
                id: quietRow

                required property string modelData

                Layout.fillWidth: true
                spacing: overview.style.unit * 0.55

                Rectangle {
                    implicitWidth: Math.round(overview.style.unit * 0.4)
                    implicitHeight: implicitWidth
                    radius: width / 2
                    color: "transparent"
                    border.color: overview.style.dim
                }

                Text {
                    Layout.fillWidth: true
                    text: quietRow.modelData
                    elide: Text.ElideRight
                    color: overview.style.mid
                    font.family: overview.style.mono
                    font.pixelSize: overview.style.unit * 0.68
                }
            }
        }
    }
}
