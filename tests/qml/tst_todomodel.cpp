#include "todomodel.h"

#include <QAbstractItemModelTester>
#include <QFile>
#include <QSaveFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using topdo::TodoModel;

namespace {

// The example in docs/design/format.md.
QByteArray exampleFile() {
    return "- [ ] Buy milk\n"
           "\n"
           "## Work\n"
           "\n"
           "- [ ] Write report due:2026-11-21\n"
           "- [ ] Plan trip\n"
           "  - [x] Book flight\n"
           "  - [ ] Book hotel\n"
           "  Ask about late check-in.\n"
           "\n"
           "## Home\n"
           "\n"
           "* [ ] Call the bank\n"
           "- [x] Renew passport\n";
}

// Writes the way an editor does: a new file replaces the old one.
void writeFile(const QString &path, const QByteArray &bytes) {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qFatal("cannot write %s", qPrintable(path));
    }
    file.write(bytes);
    file.commit();
}

QByteArray readFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll();
}

QVariant role(const TodoModel &model, int row, TodoModel::Role role) {
    return model.data(model.index(row), role);
}

QStringList titles(const TodoModel &model) {
    QStringList shown;
    for (int row = 0; row < model.rowCount(); ++row) {
        shown.append(role(model, row, TodoModel::TitleRole).toString());
    }
    return shown;
}

// A todo model on a todo file in a temporary folder, checked by QAbstractItemModelTester.
struct Fixture {
    explicit Fixture(const QByteArray &file) {
        path = dir.filePath("todo.md");
        if (!file.isEmpty()) {
            writeFile(path, file);
        }
        model.setPath(path);
    }

    QTemporaryDir dir;
    QString path;
    TodoModel model;
    QAbstractItemModelTester tester{&model, QAbstractItemModelTester::FailureReportingMode::QtTest};
};

} // namespace

class TestTodoModel : public QObject {
    Q_OBJECT

  private Q_SLOTS:
    void listsOpenTodos() {
        const Fixture fixture(exampleFile());
        const TodoModel &model = fixture.model;

        QCOMPARE(
            titles(model),
            (QStringList{"Buy milk", "Write report", "Plan trip", "Book hotel", "Call the bank"}));
        QCOMPARE(role(model, 3, TodoModel::NumberRole), 5);
        QCOMPARE(role(model, 3, TodoModel::DepthRole), 1);
        QCOMPARE(role(model, 3, TodoModel::SectionRole), "Work");
        QCOMPARE(role(model, 1, TodoModel::TextRole), "Write report due:2026-11-21");
        QCOMPARE(role(model, 1, TodoModel::DueRole), QDate(2026, 11, 21));
        QVERIFY(!role(model, 0, TodoModel::DueRole).isValid());
        QCOMPARE(model.openCount(), 4);
        QCOMPARE(model.doneCount(), 2);
        QCOMPARE(model.totalCount(), 7);
        QCOMPARE(model.error(), QString());
    }

    void filtersAndShowsDone() {
        Fixture fixture(exampleFile());
        TodoModel &model = fixture.model;

        model.setShowDone(true);
        QCOMPARE(model.rowCount(), 7);
        model.setFilter("BOOK");
        QCOMPARE(titles(model), (QStringList{"Book flight", "Book hotel"}));
    }

    void picksTheNextTodo() {
        const QDate today = QDate::currentDate();
        const QByteArray yesterday = today.addDays(-1).toString(Qt::ISODate).toUtf8();
        const Fixture fixture("- [ ] first\n- [ ] late due:" + yesterday + "\n");

        QCOMPARE(fixture.model.nextTitle(), "late");
        QCOMPARE(fixture.model.nextDue(), today.addDays(-1));
        QCOMPARE(fixture.model.lateCount(), 1);
    }

    void editsTheFile() {
        Fixture fixture(exampleFile());
        TodoModel &model = fixture.model;

        model.add("Pay\nrent");
        model.setDone(1, true);
        model.setText(2, "Pay the rent");
        model.remove(4); // Plan trip, with its subtasks and note

        QCOMPARE(readFile(fixture.path),
                 QByteArray("- [x] Buy milk\n"
                            "- [ ] Pay the rent\n"
                            "\n"
                            "## Work\n"
                            "\n"
                            "- [ ] Write report due:2026-11-21\n"
                            "\n"
                            "## Home\n"
                            "\n"
                            "* [ ] Call the bank\n"
                            "- [x] Renew passport\n"));
        QCOMPARE(titles(model), (QStringList{"Pay the rent", "Write report", "Call the bank"}));
    }

    void ignoresStaleNumbers() {
        Fixture fixture(exampleFile());
        const QSignalSpy changed(&fixture.model, &TodoModel::todosChanged);

        fixture.model.setDone(99, true);
        fixture.model.remove(0);
        fixture.model.setText(1, "  ");

        QCOMPARE(readFile(fixture.path), exampleFile());
        QCOMPARE(changed.count(), 0);
    }

    void createsTheFileOnTheFirstAdd() {
        Fixture fixture({});

        QCOMPARE(fixture.model.rowCount(), 0);
        fixture.model.add("first");

        QCOMPARE(readFile(fixture.path), QByteArray("- [ ] first\n"));
        QCOMPARE(fixture.model.rowCount(), 1);
    }

    // An edit made elsewhere (the CLI, a text editor) shows without any call to the model.
    void followsTheFileOnDisk() {
        Fixture fixture(exampleFile());

        writeFile(fixture.path, "- [ ] from an editor\n");
        QTRY_COMPARE(titles(fixture.model), QStringList{"from an editor"});

        writeFile(fixture.path, "- [ ] and again\n");
        QTRY_COMPARE(titles(fixture.model), QStringList{"and again"});
    }

    void reportsErrors() {
        const QTemporaryDir dir;
        TodoModel model;

        model.setPath(dir.path()); // a folder can't be read as a todo file

        QVERIFY(model.error().contains(dir.path()));
    }
};

QTEST_GUILESS_MAIN(TestTodoModel)
#include "tst_todomodel.moc"
