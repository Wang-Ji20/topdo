pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import org.kde.kirigami as Kirigami
import io.github.wangji20.topdo

// The badge: it takes the free space of the bar and draws a pill at the bar's exact centre,
// whatever the width of the app menu on its left and the tray on its right.
// See docs/design/widget.md, "Badge".
Item {
    id: badge

    required property TodoModel todos
    // The day's next event, as "15:00 Review"; empty for none.
    property string event: ""
    // The pill, at the screen's centre: the panel opens under it, not under the whole badge,
    // which spans the free space of the bar.
    readonly property alias pillItem: pill

    signal clicked

    // Places the pill at the centre of the bar, as far as the free space allows. The top bar spans
    // the screen, so that is the screen's centre. Both numbers are in the bar's own window:
    // global coordinates don't match Plasma's screen geometry once a second screen is left of it.
    function centre() {
        const left = badge.mapToItem(null, 0, 0).x;
        const wanted = badge.Window.width / 2 - left - pill.width / 2;
        pill.x = Math.max(0, Math.min(wanted, badge.width - pill.width));
    }

    function withAlpha(colour, alpha) {
        return Qt.rgba(colour.r, colour.g, colour.b, alpha);
    }

    // The app menu also stretches (it ends with a filler item), and the bar splits free space
    // between stretching widgets. Asking for the whole screen leaves the app menu its minimum,
    // which is exactly its buttons, and gives the badge everything else.
    Layout.fillWidth: true
    Layout.preferredWidth: Screen.width
    Layout.fillHeight: true
    Layout.minimumWidth: Kirigami.Units.gridUnit * 6
    Component.onCompleted: centre()
    onWidthChanged: centre()
    Window.onWidthChanged: centre()

    // The metal edge: a gradient that is light at the top and darker below, under the fill.
    Rectangle {
        id: pill

        objectName: "pill"
        anchors.verticalCenter: parent.verticalCenter
        width: Math.min(row.implicitWidth + Kirigami.Units.gridUnit, badge.width)
        height: Math.min(badge.height - Kirigami.Units.smallSpacing, row.implicitHeight + Kirigami.Units.smallSpacing * 2)
        radius: height / 2
        onWidthChanged: badge.centre()

        gradient: Gradient {
            GradientStop {
                position: 0
                color: badge.withAlpha(Kirigami.Theme.textColor, 0.42)
            }

            GradientStop {
                position: 0.55
                color: badge.withAlpha(Kirigami.Theme.textColor, 0.07)
            }

            GradientStop {
                position: 1
                color: badge.withAlpha(Kirigami.Theme.textColor, 0.2)
            }
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: height / 2
            color: badge.withAlpha(Qt.lighter(Kirigami.Theme.backgroundColor, 1.2), 0.92)
        }

        RowLayout {
            id: row

            anchors.centerIn: parent
            width: Math.min(implicitWidth, pill.width - Kirigami.Units.gridUnit)
            spacing: Kirigami.Units.smallSpacing * 2

            // A ring in the theme's highlight colour.
            Rectangle {
                Layout.alignment: Qt.AlignVCenter
                implicitWidth: Kirigami.Units.smallSpacing * 2
                implicitHeight: implicitWidth
                radius: width / 2
                color: "transparent"
                border.width: 2
                border.color: Kirigami.Theme.highlightColor
            }

            Text {
                Layout.fillWidth: true
                Layout.maximumWidth: Kirigami.Units.gridUnit * 16
                text: badge.todos.nextTitle || "all done"
                elide: Text.ElideRight
                color: Kirigami.Theme.textColor
                font.family: "Poppins"
            }

            Text {
                visible: badge.todos.openCount > 0
                text: `${badge.todos.openCount} open`
                color: badge.withAlpha(Kirigami.Theme.textColor, 0.7)
                font.family: "JetBrains Mono"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
            }

            Text {
                visible: badge.todos.lateCount > 0
                text: `${badge.todos.lateCount} late`
                color: Kirigami.Theme.negativeTextColor
                font.family: "JetBrains Mono"
                font.pointSize: Kirigami.Theme.smallFont.pointSize
            }

            Rectangle {
                visible: badge.event !== ""
                Layout.fillHeight: true
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                implicitWidth: 1
                color: badge.withAlpha(Kirigami.Theme.textColor, 0.3)
            }

            Text {
                objectName: "event"
                visible: badge.event !== ""
                Layout.maximumWidth: Kirigami.Units.gridUnit * 10
                text: badge.event
                elide: Text.ElideRight
                color: Kirigami.Theme.textColor
                font.family: "Poppins"
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: badge.clicked()
        }
    }
}
