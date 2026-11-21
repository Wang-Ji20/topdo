import "../../plasmoid/package/contents/ui"
import QtQuick
import QtTest
import io.github.wangji20.topdo
import topdo.test

// The open todo column, driven with keys through the whole panel, on a real todo file.
// See docs/design/widget.md, "Panel: an expanded card".
Item {
    id: root

    // The example in docs/design/format.md. Open todos: 1 Buy milk, 2 Write report, 3 Plan trip,
    // 5 Book hotel, 6 Call the bank; done: 4 Book flight, 7 Renew passport.
    readonly property string example: "- [ ] Buy milk\n\n## Work\n\n- [ ] Write report due:2026-11-21\n- [ ] Plan trip\n  - [x] Book flight\n  - [ ] Book hotel\n  Ask about late check-in.\n\n## Home\n\n* [ ] Call the bank\n- [x] Renew passport\n"
    property string path

    width: 1400
    height: 900

    TodoModel {
        id: todoModel

        path: "/nonexistent/topdo-test/todo.md"
    }

    Panel {
        id: panel

        width: implicitWidth
        height: implicitHeight
        todos: todoModel
        fullHeight: 800
    }

    TestCase {
        function init() {
            root.path = Fixture.write(root.example);
            todoModel.path = root.path;
            todoModel.showDone = false;
            panel.open(-1);
            panel.forceActiveFocus();
            keyClick(Qt.Key_1); // open the todo column
        }

        function type(text: string) {
            for (const character of text) {
                keyClick(character);
            }
        }

        function file() : string {
            return Fixture.read(root.path);
        }

        function rows() : int {
            return todoModel.rowCount();
        }

        function test_typingFilters() {
            type("BOOK");
            compare(todoModel.filter, "BOOK");
            compare(rows(), 2, "Book flight (done) and Book hotel");
            keyClick(Qt.Key_Escape);
            compare(todoModel.filter, "", "Esc clears the input first");
            compare(panel.openColumn, 0);
        }

        function test_enterTicksTheHighlightedTodo() {
            keyClick(Qt.Key_Down);
            keyClick(Qt.Key_Return);
            verify(file().includes("- [x] Write report"), file());
        }

        function test_enterOnTheAddRowAdds() {
            type("Pay rent");
            compare(rows(), 0);
            keyClick(Qt.Key_Return);
            verify(file().startsWith("- [ ] Buy milk\n- [ ] Pay rent\n"), file());
            compare(todoModel.filter, "", "the input is cleared");
        }

        function test_ctrlEnterAddsDespiteMatches() {
            type("Buy");
            compare(rows(), 1);
            keyClick(Qt.Key_Return, Qt.ControlModifier);
            verify(file().startsWith("- [ ] Buy milk\n- [ ] Buy\n"), file());
            verify(!file().includes("- [x] Buy milk"), "nothing was ticked");
        }

        function test_ctrlEEditsAndEnterSaves() {
            keyClick(Qt.Key_E, Qt.ControlModifier);
            compare(panel.todoEditing, true);
            for (let i = 0; i < 4; ++i) {
                keyClick(Qt.Key_Backspace);
            }
            type("eggs");
            keyClick(Qt.Key_Return);
            compare(panel.todoEditing, false);
            verify(file().startsWith("- [ ] Buy eggs\n"), file());
        }

        function test_escapeCancelsAnEdit() {
            keyClick(Qt.Key_E, Qt.ControlModifier);
            type("!!");
            keyClick(Qt.Key_Tab);
            compare(panel.openColumn, 0, "no switching columns in the middle of an edit");
            keyClick(Qt.Key_Escape);
            compare(panel.todoEditing, false);
            compare(file(), root.example);
            keyClick(Qt.Key_Escape);
            compare(panel.openColumn, -1, "then Esc goes back to the overview");
        }

        function test_shiftEnterAddsASubtask() {
            keyClick(Qt.Key_Down);
            keyClick(Qt.Key_Down); // Plan trip
            keyClick(Qt.Key_Return, Qt.ShiftModifier);
            compare(panel.todoAddingSubtask, true);
            type("Pack");
            compare(todoModel.filter, "", "a new subtask doesn't filter");
            keyClick(Qt.Key_Tab);
            compare(panel.openColumn, 0, "no switching columns in the middle of a new subtask");
            keyClick(Qt.Key_Return);
            compare(panel.todoAddingSubtask, false);
            verify(file().includes("  Ask about late check-in.\n  - [ ] Pack\n"), file());
        }

        function test_escapeCancelsASubtask() {
            keyClick(Qt.Key_Return, Qt.ShiftModifier);
            type("Pack");
            keyClick(Qt.Key_Escape);
            compare(panel.todoAddingSubtask, false);
            compare(file(), root.example);
        }

        function test_shiftDeleteRemovesWithSubtasks() {
            keyClick(Qt.Key_Down);
            keyClick(Qt.Key_Down); // Plan trip
            keyClick(Qt.Key_Delete, Qt.ShiftModifier);
            verify(!file().includes("Plan trip"), file());
            verify(!file().includes("Book hotel"), file());
            verify(file().includes("- [ ] Write report"), file());
        }

        function test_shiftBackspaceRemovesOnlyWithAnEmptyInput() {
            keyClick(Qt.Key_Backspace, Qt.ShiftModifier); // Buy milk
            verify(!file().includes("Buy milk"), file());
            type("Wr");
            keyClick(Qt.Key_Backspace, Qt.ShiftModifier);
            compare(todoModel.filter, "W", "with text, it deletes a character");
            verify(file().includes("Write report"), file());
        }

        function test_ctrlHShowsDoneTodos() {
            compare(rows(), 5);
            keyClick(Qt.Key_H, Qt.ControlModifier);
            compare(rows(), 7);
        }

        function test_arrowsSwitchColumnsOnlyWithAnEmptyInput() {
            type("ab");
            keyClick(Qt.Key_Left);
            keyClick(Qt.Key_Left);
            keyClick(Qt.Key_Left);
            compare(panel.openColumn, 0, "with text, the arrows move the cursor");
            keyClick(Qt.Key_Escape);
            keyClick(Qt.Key_Right);
            compare(panel.openColumn, 1);
        }

        function test_leavingClearsTheFilter() {
            type("milk");
            keyClick(Qt.Key_Tab);
            compare(panel.openColumn, 1);
            compare(todoModel.filter, "", "the overview gets the whole list back");
        }

        name: "TodoOpen"
        when: windowShown
    }

}
