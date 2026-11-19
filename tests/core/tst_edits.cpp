#include "edits.h"
#include "randomfile.h"
#include "todos.h"

#include <QTest>

using topdo::Document;
using topdo::Todo;
using topdo::TodoList;

namespace {

// The example in docs/design/format.md.
QStringList example() {
    return {
        "- [ ] Buy milk",                    // 0
        "",                                  // 1
        "## Work",                           // 2
        "",                                  // 3
        "- [ ] Write report due:2026-11-21", // 4
        "- [ ] Plan trip",                   // 5
        "  - [x] Book flight",               // 6
        "  - [ ] Book hotel",                // 7
        "  Ask about late check-in.",        // 8
        "",                                  // 9
        "## Home",                           // 10
        "",                                  // 11
        "* [ ] Call the bank",               // 12
        "- [x] Renew passport",              // 13
        "",                                  // 14
        "Notes that are not todos stay exactly as written.",
    };
}

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
    for (qsizetype i = index + 1; i < list.todos.size(); ++i) {
        qsizetype ancestor = list.todos.at(i).parent;
        while (ancestor > index) {
            ancestor = list.todos.at(ancestor).parent;
        }
        if (ancestor == index) {
            ++count;
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
    void ticksAndUnticks() {
        const Document document{.lines = {"- [ ] a", "* [X] b", "  + [x]  c"}};

        QCOMPARE(topdo::setDone(document, 0, true).lines.at(0), QString("- [x] a"));
        QCOMPARE(topdo::setDone(document, 1, false).lines.at(1), QString("* [ ] b"));
        QCOMPARE(topdo::setDone(document, 1, true).lines.at(1), QString("* [X] b"));
        QCOMPARE(topdo::setDone(document, 2, false).lines.at(2), QString("  + [ ]  c"));
    }

    void replacesOnlyTheText() {
        const Document document{.lines = {"  *  [x]  old due:2026-11-21"}};

        const Document edited = topdo::setText(document, 0, "new");

        QCOMPARE(edited.lines, QStringList{"  *  [x]  new"});
    }

    void removesSubtasksAndNotes() {
        const Document edited = topdo::removeTodo({.lines = example()}, 2); // "Plan trip"

        QStringList expected = example();
        expected.remove(5, 4); // lines 5 to 8; the blank line 9 stays
        QCOMPARE(edited.lines, expected);
    }

    void addsTodos_data() {
        QTest::addColumn<QStringList>("lines");
        QTest::addColumn<int>("section"); // -1: no section given
        QTest::addColumn<int>("expectedLine");

        QTest::newRow("no section: after the first run") << example() << -1 << 1;
        QTest::newRow("Work: after the notes of the run") << example() << 1 << 9;
        QTest::newRow("Home: after the last todo") << example() << 2 << 14;
        QTest::newRow("unnamed section") << example() << 0 << 1;
        QTest::newRow("empty section: after the heading")
            << QStringList{"# A", "text", "# B"} << 0 << 0;
        QTest::newRow("empty named section") << QStringList{"# A", "text", "# B"} << 2 << 3;
        QTest::newRow("no todos: at the end") << QStringList{"# A", "text"} << -1 << 2;
        QTest::newRow("empty file") << QStringList{} << -1 << 0;
        QTest::newRow("a blank line ends the run")
            << QStringList{"- [ ] a", "", "- [ ] b"} << -1 << 1;
    }

    void addsTodos() {
        QFETCH(QStringList, lines);
        QFETCH(int, section);
        QFETCH(int, expectedLine);

        const std::optional<qsizetype> where =
            section < 0 ? std::nullopt : std::optional<qsizetype>(section);
        const Document edited = topdo::addTodo({.lines = lines}, "new", where);

        QStringList expected = lines;
        expected.insert(expectedLine, "- [ ] new");
        QCOMPARE(edited.lines, expected);
    }

    // Ticking changes at most one character, and only that todo's done state.
    void setDoneTouchesOneCharacter() {
        forRandomTodos([](const Document &document,
                          const TodoList &list,
                          qsizetype todo,
                          QRandomGenerator &random,
                          const QString &where) {
            const bool done = random.bounded(2) == 1;
            const Document edited = topdo::setDone(document, todo, done);
            TodoList expected = list;
            expected.todos[todo].done = done;
            const QList<qsizetype> changed = changedLines(document, edited);
            const bool onlyItsLine =
                changed.isEmpty() || changed == QList{list.todos.at(todo).line};
            QVERIFY2(onlyItsLine, qPrintable(where));
            QVERIFY2(topdo::findTodos(edited) == expected, qPrintable(where));
        });
    }

    void setTextTouchesOneLine() {
        forRandomTodos([](const Document &document,
                          const TodoList &list,
                          qsizetype todo,
                          QRandomGenerator &,
                          const QString &where) {
            const Document edited = topdo::setText(document, todo, "new text");
            const QList<qsizetype> changed = changedLines(document, edited);
            const bool onlyItsLine =
                changed.isEmpty() || changed == QList{list.todos.at(todo).line};
            QVERIFY2(onlyItsLine, qPrintable(where));
            QVERIFY2(topdo::findTodos(edited).todos.at(todo).text == "new text", qPrintable(where));
        });
    }

    // Removing deletes one block of lines starting at the todo, with exactly its subtasks.
    void removeDeletesTheSubtree() {
        forRandomTodos([](const Document &document,
                          const TodoList &list,
                          qsizetype todo,
                          QRandomGenerator &,
                          const QString &where) {
            const Document edited = topdo::removeTodo(document, todo);
            const qsizetype first = list.todos.at(todo).line;
            const qsizetype removed = document.lines.size() - edited.lines.size();
            QStringList expected = document.lines;
            expected.remove(first, removed);
            QVERIFY2(removed >= 1 && edited.lines == expected, qPrintable(where));
            const qsizetype todosRemoved =
                list.todos.size() - topdo::findTodos(edited).todos.size();
            QVERIFY2(todosRemoved == 1 + descendantCount(list, todo), qPrintable(where));
        });
    }

    // Adding inserts one line, which reads back as an open top-level todo with the text.
    void addInsertsOneTodo() {
        forRandomTodos([](const Document &document,
                          const TodoList &list,
                          qsizetype,
                          QRandomGenerator &random,
                          const QString &where) {
            const auto section = random.bounded(static_cast<int>(list.sections.size()));
            const Document edited = topdo::addTodo(document, "new text", section);
            const TodoList after = topdo::findTodos(edited);
            QVERIFY2(after.todos.size() == list.todos.size() + 1, qPrintable(where));
            const auto added = std::ranges::find(after.todos, QString("new text"), &Todo::text);
            QVERIFY2(added != after.todos.end(), qPrintable(where));
            QVERIFY2(added->parent == -1 && !added->done && added->section == section,
                     qPrintable(where));
            QStringList withoutIt = edited.lines;
            withoutIt.remove(added->line);
            QVERIFY2(withoutIt == document.lines, qPrintable(where));
        });
    }
};

QTEST_GUILESS_MAIN(TestEdits)
#include "tst_edits.moc"
