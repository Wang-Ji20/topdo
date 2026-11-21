#include "projectmodel.h"
#include "todomodel.h"

#include <QAbstractItemModelTester>
#include <QFile>
#include <QSaveFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using topdo::ProjectModel;
using topdo::TodoModel;

namespace {

// The example in docs/design/format.md.
QByteArray exampleFile() {
    return "(A) Call the bank @phone due:2026-11-21\n"
           "Buy milk\n"
           "Plan trip +Holiday id:1\n"
           "x 2026-11-20 Book flight +Holiday p:1\n"
           "Book hotel +Holiday p:1\n"
           "2026-11-18 Write report +Work\n"
           "x 2026-11-19 Renew passport pri:B\n";
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
        path = dir.filePath("todo.txt");
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
    // By project, then in file order.
    void listsOpenTodos() {
        const Fixture fixture(exampleFile());
        const TodoModel &model = fixture.model;

        QCOMPARE(
            titles(model),
            (QStringList{"Call the bank", "Buy milk", "Plan trip", "Book hotel", "Write report"}));
        QCOMPARE(role(model, 3, TodoModel::NumberRole), 5);
        QCOMPARE(role(model, 3, TodoModel::DepthRole), 1);
        QCOMPARE(role(model, 3, TodoModel::SectionRole), "Holiday");
        QCOMPARE(role(model, 0, TodoModel::SectionRole), "");
        QCOMPARE(role(model, 0, TodoModel::TextRole), "Call the bank @phone due:2026-11-21");
        QCOMPARE(role(model, 0, TodoModel::DueRole), QDate(2026, 11, 21));
        QVERIFY(!role(model, 1, TodoModel::DueRole).isValid());
        QCOMPARE(role(model, 0, TodoModel::TagsRole), QStringList{"@phone"});
        QCOMPARE(role(model, 2, TodoModel::TagsRole), QStringList{});
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
        const Fixture fixture("first\nlate due:" + yesterday + "\n");

        QCOMPARE(fixture.model.nextTitle(), "late");
        QCOMPARE(fixture.model.nextDue(), today.addDays(-1));
        QCOMPARE(fixture.model.lateCount(), 1);
        QCOMPARE(fixture.model.dueToday(), QStringList{});
    }

    void editsTheFile() {
        Fixture fixture(exampleFile());
        TodoModel &model = fixture.model;
        const QByteArray today = QDate::currentDate().toString(Qt::ISODate).toUtf8();

        model.add("Pay\nrent");
        model.setDone(2, true);
        model.setText(8, "Pay the rent");
        model.remove(3); // Plan trip, with its subtasks
        model.addSubtask(1, "Find the branch");

        QCOMPARE(readFile(fixture.path),
                 QByteArray("(A) Call the bank @phone due:2026-11-21 id:1\n"
                            "Find the branch p:1\n"
                            "x " +
                            today +
                            " Buy milk\n"
                            "2026-11-18 Write report +Work\n"
                            "x 2026-11-19 Renew passport pri:B\n"
                            "Pay the rent\n"));
        QCOMPARE(titles(model),
                 (QStringList{"Call the bank", "Find the branch", "Pay the rent", "Write report"}));
    }

    // Undo puts removed todos back, the latest first, and a stale number leaves nothing to undo.
    void undoesRemovals() {
        Fixture fixture(exampleFile());
        TodoModel &model = fixture.model;

        model.remove(99);
        QVERIFY(!model.canUndoRemove());
        model.remove(3); // Plan trip, with its subtasks
        model.remove(1); // Call the bank
        QVERIFY(model.canUndoRemove());

        model.undoRemove();
        QVERIFY(readFile(fixture.path).startsWith("(A) Call the bank"));
        model.undoRemove();
        QCOMPARE(readFile(fixture.path), exampleFile());
        QVERIFY(!model.canUndoRemove());
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

        QCOMPARE(readFile(fixture.path), QByteArray("first\n"));
        QCOMPARE(fixture.model.rowCount(), 1);
    }

    // An edit made elsewhere (the CLI, a text editor) shows without any call to the model.
    void followsTheFileOnDisk() {
        Fixture fixture(exampleFile());

        writeFile(fixture.path, "from an editor\n");
        QTRY_COMPARE(titles(fixture.model), QStringList{"from an editor"});

        writeFile(fixture.path, "and again\n");
        QTRY_COMPARE(titles(fixture.model), QStringList{"and again"});
    }

    // A project's milestones and steps, the first project when the asked-for one is missing, and
    // the view following the file.
    void showsAProject() {
        Fixture fixture("Paper +AidWalk id:1\n"
                        "x Run baselines p:1\n"
                        "Write discussion p:1\n"
                        "x Data +AidWalk\n"
                        "Report +Work\n");
        ProjectModel project;
        project.setTodos(&fixture.model);

        QCOMPARE(project.projects(), (QStringList{"AidWalk", "Work"}));
        QCOMPARE(project.name(), "AidWalk");
        QCOMPARE(project.milestones().size(), 2);
        QCOMPARE(project.current(), 0);
        const QVariantMap paper = project.milestones().at(0).toMap();
        QCOMPARE(paper.value("title"), "Paper");
        QCOMPARE(paper.value("number"), 1);
        const QVariantList steps = paper.value("steps").toList();
        QCOMPARE(steps.size(), 2);
        QCOMPARE(steps.at(0).toMap().value("done"), true);
        QCOMPARE(steps.at(1).toMap().value("number"), 3);

        project.setProject("Work");
        QCOMPARE(project.name(), "Work");
        project.setProject("Gone");
        QCOMPARE(project.name(), "AidWalk");

        fixture.model.setDone(1, true);
        QCOMPARE(project.current(), -1);
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
