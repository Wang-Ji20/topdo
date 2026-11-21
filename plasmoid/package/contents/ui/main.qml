pragma ComponentBehavior: Bound

// A badge in the top bar that opens our own frameless panel, with KWin's blur behind it.
// See docs/design/widget.md, "How it is built".
import QtQuick
import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import io.github.wangji20.topdo

PlasmoidItem {
    id: root

    // The badge is shown inline in the bar; the panel is our own dialog, not Plasma's popup.
    preferredRepresentation: fullRepresentation

    fullRepresentation: Badge {
        todos: todoModel
        event: {
            const next = todayModel.next >= 0 ? todayModel.events[todayModel.next] : null;
            return next ? `${Qt.formatTime(next.start, "HH:mm")} ${next.title}` : "";
        }
        onClicked: panel.visible = !panel.visible
    }

    TodoModel {
        id: todoModel
    }

    // Today's events from the KDE calendar, for the today column and the badge.
    TodayModel {
        id: todayModel
    }

    // The project column's project, remembered in the widget's settings.
    ProjectModel {
        id: projectModel

        todos: todoModel
        // The settings are a map with the keys of contents/config/main.xml, unknown to qmllint.
        project: Plasmoid.configuration.project // qmllint disable missing-property
    }

    // The widget's global shortcut (its settings, "Keyboard Shortcuts") opens and closes the panel.
    // Plasma would expand its own popup; topdo's panel is its own dialog.
    Connections {
        function onActivated() {
            panel.visible = !panel.visible;
        }

        target: Plasmoid
    }

    PlasmaCore.Dialog {
        id: panel

        function blur() {
            if (visible) {
                WindowEffects.blurBehind(panel, Qt.rect(0, 0, content.width, content.height), Kirigami.Units.gridUnit);
            }
        }

        visualParent: (root.fullRepresentationItem as Badge)?.pillItem ?? null
        location: PlasmaCore.Types.Floating
        type: PlasmaCore.Dialog.PopupMenu
        backgroundHints: PlasmaCore.Dialog.NoBackground
        hideOnWindowDeactivate: true
        onWidthChanged: Qt.callLater(blur) // one blur request per resize, not one per side
        onHeightChanged: Qt.callLater(blur)
        // Every time it opens: the overview, with the keyboard focus on the panel.
        onVisibleChanged: {
            if (visible) {
                content.open(-1);
                content.focusColumn = 0;
                panel.requestActivate();
                content.forceActiveFocus();
            }
            blur();
        }

        mainItem: Panel {
            id: content

            width: implicitWidth
            height: implicitHeight
            todos: todoModel
            project: projectModel
            day: todayModel
            fullHeight: Plasmoid.containment.availableScreenRect.height - Kirigami.Units.gridUnit * 2
            onCloseRequested: panel.visible = false
            onProjectPicked: name => {
                Plasmoid.configuration.project = name; // qmllint disable missing-property
            }
        }
    }
}
