#include "randomfile.h"
#include "todos.h"

#include <QTest>

using topdo::Document;
using topdo::Section;
using topdo::Todo;
using topdo::TodoList;

namespace {

TodoList todosIn(const QStringList &lines) {
    const Document document{.lines = lines};
    return topdo::findTodos(document);
}

// Seed for the property tests: TOPDO_SEED replays a failure, otherwise each run picks a new one.
quint32 propertySeed() {
    bool ok = false;
    const quint32 fromEnvironment = qEnvironmentVariable("TOPDO_SEED").toUInt(&ok);
    return ok ? fromEnvironment : QRandomGenerator::global()->generate();
}

int indentation(const QString &line) {
    int columns = 0;
    for (const QChar ch : line) {
        if (ch != u' ' && ch != u'\t') {
            break;
        }
        columns += ch == u'\t' ? 4 : 1;
    }
    return columns;
}

} // namespace

class TestTodos : public QObject {
    Q_OBJECT

  private Q_SLOTS:
    // The example in docs/design/format.md.
    void readsTheFormatExample() {
        const TodoList list = todosIn({
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
        });

        const QList<Section> sections = {
            {.title = "", .line = -1},
            {.title = "Work", .line = 2},
            {.title = "Home", .line = 10},
        };
        QCOMPARE(list.sections, sections);

        const QList<Todo> todos = {
            {
                .line = 0,
                .parent = -1,
                .section = 0,
                .done = false,
                .text = "Buy milk",
                .title = "Buy milk",
                .due = std::nullopt,
            },
            {
                .line = 4,
                .parent = -1,
                .section = 1,
                .done = false,
                .text = "Write report due:2026-11-21",
                .title = "Write report",
                .due = QDate(2026, 11, 21),
            },
            {
                .line = 5,
                .parent = -1,
                .section = 1,
                .done = false,
                .text = "Plan trip",
                .title = "Plan trip",
                .due = std::nullopt,
            },
            {
                .line = 6,
                .parent = 2,
                .section = 1,
                .done = true,
                .text = "Book flight",
                .title = "Book flight",
                .due = std::nullopt,
            },
            {
                .line = 7,
                .parent = 2,
                .section = 1,
                .done = false,
                .text = "Book hotel",
                .title = "Book hotel",
                .due = std::nullopt,
            },
            {
                .line = 12,
                .parent = -1,
                .section = 2,
                .done = false,
                .text = "Call the bank",
                .title = "Call the bank",
                .due = std::nullopt,
            },
            {
                .line = 13,
                .parent = -1,
                .section = 2,
                .done = true,
                .text = "Renew passport",
                .title = "Renew passport",
                .due = std::nullopt,
            },
        };
        QCOMPARE(list.todos, todos);
    }

    void recognisesTodos_data() {
        QTest::addColumn<QString>("line");
        QTest::addColumn<bool>("isTodo");
        QTest::addColumn<bool>("done");

        QTest::newRow("dash") << "- [ ] a" << true << false;
        QTest::newRow("star") << "* [ ] a" << true << false;
        QTest::newRow("plus") << "+ [ ] a" << true << false;
        QTest::newRow("done x") << "- [x] a" << true << true;
        QTest::newRow("done X") << "- [X] a" << true << true;
        QTest::newRow("indented") << "    - [ ] a" << true << false;
        QTest::newRow("tabs") << "\t-\t[ ]\ta" << true << false;
        QTest::newRow("numbered") << "1. [ ] a" << false << false;
        QTest::newRow("no space after bullet") << "-[ ] a" << false << false;
        QTest::newRow("empty checkbox") << "- [] a" << false << false;
        QTest::newRow("other mark") << "- [y] a" << false << false;
        QTest::newRow("no space after checkbox") << "- [ ]a" << false << false;
        QTest::newRow("no text") << "- [ ]" << false << false;
        QTest::newRow("plain item") << "- a" << false << false;
    }

    void recognisesTodos() {
        QFETCH(QString, line);
        QFETCH(bool, isTodo);
        QFETCH(bool, done);

        const TodoList list = todosIn({line});

        QCOMPARE(list.todos.size(), isTodo ? 1 : 0);
        if (isTodo) {
            QCOMPARE(list.todos.first().done, done);
        }
    }

    void skipsCodeFences() {
        const TodoList list =
            todosIn({"```", "- [ ] code", "```", "- [ ] a", "~~~md", "# not a heading", "~~~"});

        QCOMPARE(list.todos.size(), 1);
        QCOMPARE(list.todos.first().line, 3);
        QCOMPARE(list.sections.size(), 1);
    }

    void needsSpaceAfterHashes() {
        const TodoList list = todosIn({"#hashtag", "####### seven", "### Three ###"});

        QCOMPARE(list.sections.size(), 2);
        QCOMPARE(list.sections.last().title, QString("Three ###"));
    }

    void nestsByIndentation() {
        const TodoList list = todosIn({
            "- [ ] a",       // 0: top level
            "  - [ ] b",     // 1: under a
            "      - [ ] c", // 2: under b
            "    - [ ] d",   // 3: under b (less than c, more than b)
            "\t- [ ] e",     // 4: a tab is four columns: under b
            "- [ ] f",       // 5: top level again
            "  - [ ] g",     // 6: under f
            "  A note",      // a note ends nesting at its indentation
            "  - [ ] i",     // 7: still under f
            "Some note",     // an unindented note ends all nesting
            "  - [ ] j",     // 8: top level
            "# Heading",     // a heading ends nesting
            "  - [ ] h",     // 9: top level of the new section
        });

        QList<qsizetype> parents;
        for (const Todo &todo : list.todos) {
            parents.append(todo.parent);
        }
        QCOMPARE(parents, (QList<qsizetype>{-1, 0, 1, 1, 1, -1, 5, 5, -1, -1}));
    }

    void splitsDueDates_data() {
        QTest::addColumn<QString>("text");
        QTest::addColumn<QString>("title");
        QTest::addColumn<QDate>("due");

        QTest::newRow("none") << "Call" << "Call" << QDate();
        QTest::newRow("at the end") << "Call due:2026-11-21" << "Call" << QDate(2026, 11, 21);
        QTest::newRow("in the middle")
            << "Call due:2026-11-21 Bob" << "Call Bob" << QDate(2026, 11, 21);
        QTest::newRow("invalid date stays text")
            << "Call due:2026-13-40" << "Call due:2026-13-40" << QDate();
        QTest::newRow("first valid counts")
            << "a due:2026-13-40 due:2026-01-02 due:2026-03-04"
            << "a due:2026-13-40 due:2026-03-04" << QDate(2026, 1, 2);
        QTest::newRow("part of a word") << "overdue:2026-11-21" << "overdue:2026-11-21" << QDate();
    }

    void splitsDueDates() {
        QFETCH(QString, text);
        QFETCH(QString, title);
        QFETCH(QDate, due);

        const Todo todo = todosIn({"- [ ] " + text}).todos.first();

        QCOMPARE(todo.title, title);
        QCOMPARE(todo.due.value_or(QDate()), due);
    }

    // A parent comes before its subtask, is less indented, and is in the same section.
    void parentsPrecedeSubtasks() {
        const quint32 seed = propertySeed();
        qInfo("seed %u (replay with TOPDO_SEED=%u)", seed, seed);
        QRandomGenerator random(seed);
        for (int i = 0; i < 2000; ++i) {
            const QByteArray bytes = topdo::test::randomTodoFile(random);
            const Document document = topdo::parse(bytes);
            const TodoList list = topdo::findTodos(document);
            for (const Todo &todo : list.todos) {
                if (todo.parent < 0) {
                    continue;
                }
                const Todo &parent = list.todos.at(todo.parent);
                const int parentIndent = indentation(document.lines.at(parent.line));
                const int indent = indentation(document.lines.at(todo.line));
                const bool ok = parent.line < todo.line && parentIndent < indent &&
                                parent.section == todo.section;
                QVERIFY2(ok,
                         qPrintable(QStringLiteral("seed %1, file %2, line %3")
                                        .arg(seed)
                                        .arg(i)
                                        .arg(todo.line)));
            }
        }
    }
};

QTEST_GUILESS_MAIN(TestTodos)
#include "tst_todos.moc"
