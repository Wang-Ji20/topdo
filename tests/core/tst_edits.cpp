#include "edits.h"
#include "randomfile.h"
#include "todos.h"

#include <QTest>

#include <ranges>

using topdo::Document;
using topdo::Todo;
using topdo::TodoList;

namespace {

// The example in docs/design/format.md.
QStringList example() {
    return {
        "(A) Call the bank @phone due:2026-11-21", // 0
        "Buy milk",                                // 1
        "Plan trip +Holiday id:1",                 // 2
        "x 2026-11-20 Book flight +Holiday p:1",   // 3
        "Book hotel +Holiday p:1",                 // 4
        "2026-11-18 Write report +Work",           // 5
        "x 2026-11-19 Renew passport pri:B",       // 6
    };
}

QDate today() { return {2026, 11, 21}; }

// Seed for the property tests: TOPDO_SEED replays a failure, otherwise each run picks a new one.
quint32 propertySeed() {
    bool ok = false;
    const quint32 fromEnvironment = qEnvironmentVariable("TOPDO_SEED").toUInt(&ok);
    return ok ? fromEnvironment : QRandomGenerator::global()->generate();
}

// The lines that differ between two documents of the same length.
QList<qsizetype> changedLines(const Document &before, const Document &after) {
    QList<qsizetype> changed;
    for (qsizetype i = 0; i < before.lines.size(); ++i) {
        if (before.lines.at(i) != after.lines.at(i)) {
            changed.append(i);
        }
    }
    return changed;
}

// The todos inside todo `index`'s subtree: its subtasks, their subtasks, and so on.
qsizetype descendantCount(const TodoList &list, qsizetype index) {
    qsizetype count = 0;
    for (const Todo &todo : list.todos) {
        for (qsizetype up = todo.parent; up >= 0; up = list.todos.at(up).parent) {
            if (up == index) {
                ++count;
                break;
            }
        }
    }
    return count;
}

// Random files that have at least one todo, with one todo picked at random.
template <typename Check>
void forRandomTodos(Check check) {
    const quint32 seed = propertySeed();
    qInfo("seed %u (replay with TOPDO_SEED=%u)", seed, seed);
    QRandomGenerator random(seed);
    for (int i = 0; i < 2000; ++i) {
        const QByteArray bytes = topdo::test::randomTodoFile(random);
        const Document document = topdo::parse(bytes);
        const TodoList list = topdo::findTodos(document);
        if (list.todos.isEmpty()) {
            continue;
        }
        const auto todo = random.bounded(static_cast<int>(list.todos.size()));
        const QString where =
            QStringLiteral("seed %1, file %2, todo %3").arg(seed).arg(i).arg(todo);
        check(document, list, todo, random, where);
        if (QTest::currentTestFailed()) {
            return;
        }
    }
}

} // namespace

class TestEdits : public QObject {
    Q_OBJECT

  private Q_SLOTS:
    void ticks_data() {
        QTest::addColumn<QString>("line");
        QTest::addColumn<QString>("ticked");

        QTest::newRow("plain") << "Call" << "x 2026-11-21 Call";
        QTest::newRow("keeps the creation date")
            << "2026-11-18 Call" << "x 2026-11-21 2026-11-18 Call";
        QTest::newRow("priority becomes pri:")
            << "(A) 2026-11-18 Call +Work" << "x 2026-11-21 2026-11-18 Call +Work pri:A";
        QTest::newRow("already done") << "x 2026-11-20 Call" << "x 2026-11-20 Call";
    }

    void ticks() {
        QFETCH(QString, line);
        QFETCH(QString, ticked);

        const Document edited = topdo::setDone({.lines = {line}}, 0, true, today());

        QCOMPARE(edited.lines, QStringList{ticked});
    }

    void unticks_data() {
        QTest::addColumn<QString>("line");
        QTest::addColumn<QString>("unticked");

        QTest::newRow("plain") << "x 2026-11-21 Call" << "Call";
        QTest::newRow("no date") << "x Call" << "Call";
        QTest::newRow("keeps the creation date")
            << "x 2026-11-21 2026-11-18 Call" << "2026-11-18 Call";
        QTest::newRow("pri: becomes the priority")
            << "x 2026-11-21 2026-11-18 Call pri:A +Work" << "(A) 2026-11-18 Call +Work";
        QTest::newRow("not a priority") << "x Call pri:a" << "Call pri:a";
        QTest::newRow("already open") << "(B) Call" << "(B) Call";
        QTest::newRow("no text stays done") << "x 2026-11-21 " << "x 2026-11-21 ";
        QTest::newRow("text starting with x stays done") << "x x y" << "x x y";
    }

    void unticks() {
        QFETCH(QString, line);
        QFETCH(QString, unticked);

        const Document edited = topdo::setDone({.lines = {line}}, 0, false, today());

        QCOMPARE(edited.lines, QStringList{unticked});
    }

    void replacesOnlyTheText() {
        const Document document{.lines = {"x 2026-11-21 2026-11-18 old due:2026-11-21", "(A) a"}};

        QCOMPARE(topdo::setText(document, 0, "new").lines.at(0),
                 QString("x 2026-11-21 2026-11-18 new"));
        QCOMPARE(topdo::setText(document, 1, "b").lines.at(1), QString("(A) b"));
    }

    void removesSubtasks() {
        QStringList lines = example();
        lines.insert(5, "Pack p:1 id:2");
        lines.append("Passport photo p:2");

        const Document edited = topdo::removeTodo({.lines = lines}, 2); // "Plan trip"

        QCOMPARE(edited.lines,
                 (QStringList{example().at(0), example().at(1), example().at(5), example().at(6)}));
    }

    void addsTodos_data() {
        QTest::addColumn<QStringList>("lines");
        QTest::addColumn<QString>("project"); // empty: none
        QTest::addColumn<QStringList>("expected");

        QTest::newRow("at the end") << QStringList{"a", "b"} << "" << QStringList{"a", "b", "new"};
        QTest::newRow("before trailing blank lines")
            << QStringList{"a", "", " "} << "" << QStringList{"a", "new", "", " "};
        QTest::newRow("empty file") << QStringList{} << "" << QStringList{"new"};
        QTest::newRow("blank file") << QStringList{""} << "" << QStringList{"new", ""};
        QTest::newRow("to a project")
            << QStringList{"a"} << "Home" << QStringList{"a", "new +Home"};
    }

    void addsTodos() {
        QFETCH(QStringList, lines);
        QFETCH(QString, project);
        QFETCH(QStringList, expected);
        const std::optional<QString> to = project.isEmpty() ? std::nullopt : std::optional(project);

        const Document edited = topdo::addTodo({.lines = lines}, "new", to);

        QCOMPARE(edited.lines, expected);
    }

    void addsToAProjectOnce() {
        const Document empty;
        const QString project = "Home";

        const Document edited = topdo::addTodo(empty, "Paint +Home", project);

        QCOMPARE(edited.lines, QStringList{"Paint +Home"});
    }

    void addsSubtasks_data() {
        QTest::addColumn<QStringList>("lines");
        QTest::addColumn<int>("todo");
        QTest::addColumn<QStringList>("expected");

        QStringList afterLast = example();
        afterLast.insert(5, "new p:1");
        QTest::newRow("after the last subtask") << example() << 2 << afterLast;
        QTest::newRow("gives the parent an id") << QStringList{"a id:1", "b", "c"} << 1
                                                << QStringList{"a id:1", "b id:2", "new p:2", "c"};
        QTest::newRow("after nested subtasks")
            << QStringList{"a id:1", "b id:2 p:1", "c p:2", "d"} << 0
            << QStringList{"a id:1", "b id:2 p:1", "c p:2", "new p:1", "d"};
    }

    void addsSubtasks() {
        QFETCH(QStringList, lines);
        QFETCH(int, todo);
        QFETCH(QStringList, expected);

        const Document edited = topdo::addSubtask({.lines = lines}, todo, "new");

        QCOMPARE(edited.lines, expected);
    }

    void restoresAtTheEndOfAShorterFile() {
        const topdo::Block block{{.line = 5, .text = "a"}, {.line = 7, .text = "b"}};

        const Document restored = topdo::restoreBlock({.lines = {"x"}}, block);

        QCOMPARE(restored.lines, (QStringList{"x", "a", "b"}));
    }

    // Ticking or unticking changes only that todo's line, and only its done state.
    void setDoneTouchesOneLine() {
        forRandomTodos([](const Document &document,
                          const TodoList &list,
                          qsizetype todo,
                          QRandomGenerator &random,
                          const QString &where) {
            const bool done = random.bounded(2) == 1;
            const Document edited = topdo::setDone(document, todo, done, today());
            const QList<qsizetype> changed = changedLines(document, edited);
            const bool onlyItsLine =
                changed.isEmpty() || changed == QList{list.todos.at(todo).line};
            QVERIFY2(onlyItsLine, qPrintable(where));
            const TodoList after = topdo::findTodos(edited);
            QVERIFY2(after.todos.size() == list.todos.size(), qPrintable(where));
            const bool unchanged = edited == document;
            QVERIFY2(after.todos.at(todo).done == done || unchanged, qPrintable(where));
        });
    }

    // Removing deletes the todo's line and its subtasks' lines; every other line stays.
    void removeTakesTheSubtree() {
        forRandomTodos([](const Document &document,
                          const TodoList &list,
                          qsizetype todo,
                          QRandomGenerator &,
                          const QString &where) {
            const Document edited = topdo::removeTodo(document, todo);
            const qsizetype removed = document.lines.size() - edited.lines.size();
            QVERIFY2(removed == 1 + descendantCount(list, todo), qPrintable(where));
            QStringList kept = document.lines;
            const topdo::Block block = topdo::todoBlock(document, todo);
            for (const topdo::RemovedLine &line : block | std::views::reverse) {
                QVERIFY2(kept.at(line.line) == line.text, qPrintable(where));
                kept.remove(line.line);
            }
            QVERIFY2(edited.lines == kept, qPrintable(where));
        });
    }

    // Removing a todo and restoring its block gives back the file as it was.
    void restoreUndoesRemove() {
        forRandomTodos([](const Document &document,
                          const TodoList &,
                          qsizetype todo,
                          QRandomGenerator &,
                          const QString &where) {
            const topdo::Block block = topdo::todoBlock(document, todo);
            const Document removed = topdo::removeTodo(document, todo);
            QVERIFY2(topdo::restoreBlock(removed, block) == document, qPrintable(where));
        });
    }

    // Adding inserts one line, which reads back as an open top-level todo of the project.
    void addInsertsOneTodo() {
        forRandomTodos([](const Document &document,
                          const TodoList &list,
                          qsizetype,
                          QRandomGenerator &,
                          const QString &where) {
            const Document edited = topdo::addTodo(document, "new text", QString("New"));
            const TodoList after = topdo::findTodos(edited);
            QVERIFY2(after.todos.size() == list.todos.size() + 1, qPrintable(where));
            const auto added =
                std::ranges::find(after.todos, QString("new text +New"), &Todo::text);
            QVERIFY2(added != after.todos.end(), qPrintable(where));
            QVERIFY2(added->parent == -1 && !added->done, qPrintable(where));
            QVERIFY2(after.sections.at(added->section).title == "New", qPrintable(where));
            QStringList withoutIt = edited.lines;
            withoutIt.remove(added->line);
            QVERIFY2(withoutIt == document.lines, qPrintable(where));
        });
    }

    // Adding a subtask inserts one line, which reads back as an open subtask of the todo (or of
    // the first todo with the same id), and changes at most the todo's own line besides.
    void addSubtaskInsertsOneSubtask() {
        forRandomTodos([](const Document &document,
                          const TodoList &list,
                          qsizetype todo,
                          QRandomGenerator &,
                          const QString &where) {
            const Document edited = topdo::addSubtask(document, todo, "new text");
            const TodoList after = topdo::findTodos(edited);
            QVERIFY2(after.todos.size() == list.todos.size() + 1, qPrintable(where));
            const auto added = std::ranges::find(after.todos, QString("new text"), &Todo::title);
            QVERIFY2(added != after.todos.end() && !added->done, qPrintable(where));
            QVERIFY2(added->parent >= 0, qPrintable(where));
            const Todo &parent = after.todos.at(added->parent);
            const qsizetype parentLine = list.todos.at(todo).line;
            QVERIFY2(parent.line == parentLine || parent.id == list.todos.at(todo).id,
                     qPrintable(where));
            QStringList withoutIt = edited.lines;
            withoutIt.remove(added->line);
            withoutIt[parentLine] = document.lines.at(parentLine);
            QVERIFY2(withoutIt == document.lines, qPrintable(where));
        });
    }
};

QTEST_GUILESS_MAIN(TestEdits)
#include "tst_edits.moc"
