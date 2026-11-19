pragma ComponentBehavior: Bound

// A badge in the top bar that opens our own frameless panel, with KWin's blur behind it.
// See docs/design/widget.md, "How it is built". The panel's content is still a placeholder.
import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import io.github.wangji20.topdo

PlasmoidItem {
    id: root

    readonly property real slabRadius: Kirigami.Units.cornerRadius * 3

    // The badge is shown inline in the bar; the panel is our own dialog, not Plasma's popup.
    preferredRepresentation: fullRepresentation

    fullRepresentation: Badge {
        todos: todoModel
        onClicked: panel.visible = !panel.visible
    }

    TodoModel {
        id: todoModel
    }

    PlasmaCore.Dialog {
        id: panel

        visualParent: root.fullRepresentationItem
        location: PlasmaCore.Types.Floating
        type: PlasmaCore.Dialog.PopupMenu
        backgroundHints: PlasmaCore.Dialog.NoBackground
        hideOnWindowDeactivate: true
        onVisibleChanged: blur()
        onWidthChanged: blur()
        onHeightChanged: blur()

        function blur() {
            if (visible) {
                WindowEffects.blurBehind(panel, Qt.rect(0, 0, slab.width, slab.height), root.slabRadius);
            }
        }

        mainItem: Rectangle {
            id: slab

            width: Kirigami.Units.gridUnit * 56
            height: Kirigami.Units.gridUnit * 22
            radius: root.slabRadius
            color: Qt.rgba(42 / 255, 44 / 255, 51 / 255, 0.88)
            border.color: Qt.rgba(1, 1, 1, 0.3)

            Text {
                anchors.centerIn: parent
                text: "glass slab"
                color: Kirigami.Theme.textColor
                font.pixelSize: Kirigami.Units.gridUnit * 3
            }
        }
    }
}
