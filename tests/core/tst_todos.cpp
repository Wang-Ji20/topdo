#include "randomfile.h"
#include "todoline.h"
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

// Random documents, with the seed and file number for a failure message.
template <typename Check>
void forRandomFiles(Check check) {
    const quint32 seed = propertySeed();
    qInfo("seed %u (replay with TOPDO_SEED=%u)", seed, seed);
    QRandomGenerator random(seed);
    for (int i = 0; i < 2000; ++i) {
        const Document document = topdo::parse(topdo::test::randomTodoFile(random));
        check(document, QStringLiteral("seed %1, file %2").arg(seed).arg(i));
        if (QTest::currentTestFailed()) {
            return;
        }
    }
}

Todo todo(qsizetype line,
          qsizetype parent,
          qsizetype section,
          bool done,
          const QString &text,
          const QString &title,
          const QStringList &projects = {},
          const QStringList &contexts = {},
          const QString &id = {},
          std::optional<QDate> due = {}) {
    return {
        .line = line,
        .parent = parent,
        .section = section,
        .done = done,
        .text = text,
        .title = title,
        .projects = projects,
        .contexts = contexts,
        .id = id,
        .due = due,
    };
}

} // namespace

class TestTodos : public QObject {
    Q_OBJECT

  private Q_SLOTS:
    // The example in docs/design/format.md.
    void readsTheFormatExample() {
        const TodoList list = todosIn({
            "(A) Call the bank @phone due:2026-11-21", // 0
            "Buy milk",                                // 1
            "Plan trip +Holiday id:1",                 // 2
            "x 2026-11-20 Book flight +Holiday p:1",   // 3
            "Book hotel +Holiday p:1",                 // 4
            "2026-11-18 Write report +Work",           // 5
            "x 2026-11-19 Renew passport pri:B",       // 6
        });

        QCOMPARE(list.sections,
                 (QList<Section>{{.title = ""}, {.title = "Holiday"}, {.title = "Work"}}));
        const QList<Todo> expected = {
            todo(0,
                 -1,
                 0,
                 false,
                 "Call the bank @phone due:2026-11-21",
                 "Call the bank",
                 {},
                 {"phone"},
                 "",
                 QDate(2026, 11, 21)),
            todo(1, -1, 0, false, "Buy milk", "Buy milk"),
            todo(2, -1, 1, false, "Plan trip +Holiday id:1", "Plan trip", {"Holiday"}, {}, "1"),
            todo(3, 2, 1, true, "Book flight +Holiday p:1", "Book flight", {"Holiday"}),
            todo(4, 2, 1, false, "Book hotel +Holiday p:1", "Book hotel", {"Holiday"}),
            todo(5, -1, 2, false, "Write report +Work", "Write report", {"Work"}),
            todo(6, -1, 0, true, "Renew passport pri:B", "Renew passport"),
        };
        QCOMPARE(list.todos, expected);
    }

    void skipsBlankLines() {
        const TodoList list = todosIn({"", "a", "   ", "\t", "b"});

        QCOMPARE(list.todos.size(), 2);
        QCOMPARE(list.todos.at(1).line, 4);
    }

    // The examples of the todo.txt spec.
    void splitsTheLineParts_data() {
        QTest::addColumn<QString>("line");
        QTest::addColumn<bool>("done");
        QTest::addColumn<QString>("priority"); // empty: none
        QTest::addColumn<QString>("completed");
        QTest::addColumn<QString>("created");
        QTest::addColumn<QString>("text");

        QTest::newRow("priority") << "(A) Call Mom" << false << "A" << "" << "" << "Call Mom";
        QTest::newRow("priority and date")
            << "(A) 2011-03-02 Call Mom" << false << "A" << "" << "2011-03-02" << "Call Mom";
        QTest::newRow("date only")
            << "2011-03-02 Document" << false << "" << "" << "2011-03-02" << "Document";
        QTest::newRow("priority not first")
            << "Call Mom (A)" << false << "" << "" << "" << "Call Mom (A)";
        QTest::newRow("lowercase priority")
            << "(b) Get back" << false << "" << "" << "" << "(b) Get back";
        QTest::newRow("no space after priority")
            << "(B)->Submit" << false << "" << "" << "" << "(B)->Submit";
        QTest::newRow("date at the end")
            << "(A) Call Mom 2011-03-02" << false << "A" << "" << "" << "Call Mom 2011-03-02";
        QTest::newRow("done") << "x 2011-03-03 Call Mom" << true << "" << "2011-03-03" << ""
                              << "Call Mom";
        QTest::newRow("done with both dates") << "x 2011-03-05 2011-03-03 Call Dad" << true << ""
                                              << "2011-03-05" << "2011-03-03" << "Call Dad";
        QTest::newRow("done without date")
            << "x (A) pay water bill" << true << "" << "" << "" << "(A) pay water bill";
        QTest::newRow("xylophone")
            << "xylophone lesson" << false << "" << "" << "" << "xylophone lesson";
        QTest::newRow("capital X")
            << "X 2012-01-01 Make" << false << "" << "" << "" << "X 2012-01-01 Make";
        QTest::newRow("x after priority")
            << "(A) x Find tickets" << false << "A" << "" << "" << "x Find tickets";
    }

    void splitsTheLineParts() {
        QFETCH(QString, line);
        QFETCH(bool, done);
        QFETCH(QString, priority);
        QFETCH(QString, completed);
        QFETCH(QString, created);
        QFETCH(QString, text);

        const topdo::TodoLine parts = topdo::splitTodoLine(line);

        QCOMPARE(parts.done, done);
        QCOMPARE(parts.priority ? QString(*parts.priority) : QString(), priority);
        QCOMPARE(parts.completed.value_or(QString()), completed);
        QCOMPARE(parts.created.value_or(QString()), created);
        QCOMPARE(parts.text, text);
    }

    void readsProjectsAsSections() {
        const TodoList list = todosIn({"a +Work +Home", "b +Home", "c", "d 2+2", "e +", "f +Work"});

        QCOMPARE(list.sections,
                 (QList<Section>{{.title = ""}, {.title = "Work"}, {.title = "Home"}}));
        QList<qsizetype> sections;
        for (const Todo &each : list.todos) {
            sections.append(each.section);
        }
        QCOMPARE(sections, (QList<qsizetype>{1, 2, 0, 0, 0, 1}));
        QCOMPARE(list.todos.at(0).projects, (QStringList{"Work", "Home"}));
        QCOMPARE(list.todos.at(0).title, "a");
        QCOMPARE(list.todos.at(3).title, "d 2+2");
        QCOMPARE(list.todos.at(4).title, "e +");
    }

    // A subtask is in its top-level todo's group, whatever its own projects; only top-level
    // todos make groups.
    void groupsSubtasksWithTheirTopLevelTodo() {
        const TodoList list = todosIn({"a +Work id:1", "b p:1 id:2", "c +Home p:2", "d"});

        QCOMPARE(list.sections, (QList<Section>{{.title = ""}, {.title = "Work"}}));
        QList<qsizetype> sections;
        for (const Todo &each : list.todos) {
            sections.append(each.section);
        }
        QCOMPARE(sections, (QList<qsizetype>{1, 1, 1, 0}));
    }

    void readsContexts() {
        const Todo first = todosIn({"Call @phone @home mail@example.com"}).todos.first();

        QCOMPARE(first.contexts, (QStringList{"phone", "home"}));
        QCOMPARE(first.title, "Call mail@example.com");
    }

    void linksSubtasks_data() {
        QTest::addColumn<QStringList>("lines");
        QTest::addColumn<QList<qsizetype>>("parents");

        QTest::newRow("parent after its subtask")
            << QStringList{"b p:1", "a id:1"} << QList<qsizetype>{1, -1};
        QTest::newRow("nested") << QStringList{"a id:1", "b id:2 p:1", "c p:2"}
                                << QList<qsizetype>{-1, 0, 1};
        QTest::newRow("unknown id") << QStringList{"a id:1", "b p:9"} << QList<qsizetype>{-1, -1};
        QTest::newRow("first id counts")
            << QStringList{"a id:1", "b id:1", "c p:1"} << QList<qsizetype>{-1, -1, 0};
        QTest::newRow("first p counts")
            << QStringList{"a id:1", "b id:2", "c p:2 p:1"} << QList<qsizetype>{-1, -1, 1};
        QTest::newRow("own id") << QStringList{"a id:1 p:1"} << QList<qsizetype>{-1};
        QTest::newRow("circle") << QStringList{"a id:1 p:2", "b id:2 p:1", "c p:1"}
                                << QList<qsizetype>{-1, -1, 0};
        QTest::newRow("empty tags") << QStringList{"a id:", "b p:"} << QList<qsizetype>{-1, -1};
    }

    void linksSubtasks() {
        QFETCH(QStringList, lines);
        QFETCH(QList<qsizetype>, parents);

        const TodoList list = todosIn(lines);

        QList<qsizetype> found;
        for (const Todo &each : list.todos) {
            found.append(each.parent);
        }
        QCOMPARE(found, parents);
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
        QTest::newRow("first valid counts") << "a due:2026-13-40 due:2026-01-02 due:2026-03-04"
                                            << "a due:2026-13-40" << QDate(2026, 1, 2);
        QTest::newRow("part of a word") << "overdue:2026-11-21" << "overdue:2026-11-21" << QDate();
        QTest::newRow("other tags stay")
            << "see https://x.y a:b" << "see https://x.y a:b" << QDate();
    }

    void splitsDueDates() {
        QFETCH(QString, text);
        QFETCH(QString, title);
        QFETCH(QDate, due);

        const Todo first = todosIn({text}).todos.first();

        QCOMPARE(first.title, title);
        QCOMPARE(first.due.value_or(QDate()), due);
    }

    // Splitting a line and joining its parts gives back the line.
    void joinsWhatItSplits() {
        forRandomFiles([](const Document &document, const QString &where) {
            for (const QString &line : document.lines) {
                const QString joined = topdo::joinTodoLine(topdo::splitTodoLine(line));
                QVERIFY2(joined == line, qPrintable(where + ": " + line));
            }
        });
    }

    // A parent has the id its subtask's p: tag names, and parents never lead in a circle.
    // A subtask is in its parent's group, and the title holds no project or context.
    void groupsAndTitlesAreSound() {
        forRandomFiles([](const Document &document, const QString &where) {
            const TodoList list = topdo::findTodos(document);
            for (const Todo &each : list.todos) {
                if (each.parent >= 0) {
                    QVERIFY2(each.section == list.todos.at(each.parent).section, qPrintable(where));
                }
                for (const QString &word : each.title.split(u' ')) {
                    const bool marked =
                        word.size() > 1 && (word.startsWith(u'+') || word.startsWith(u'@'));
                    QVERIFY2(!marked, qPrintable(where + ": " + each.title));
                }
            }
        });
    }

    void parentsAreSound() {
        forRandomFiles([](const Document &document, const QString &where) {
            const TodoList list = topdo::findTodos(document);
            for (const Todo &each : list.todos) {
                if (each.parent < 0) {
                    continue;
                }
                const QString tag = "p:" + list.todos.at(each.parent).id;
                QVERIFY2(topdo::tagValue(each.text, "p") == list.todos.at(each.parent).id,
                         qPrintable(where + ": " + tag));
                qsizetype steps = 0;
                for (qsizetype up = each.parent; up >= 0; up = list.todos.at(up).parent) {
                    QVERIFY2(++steps <= list.todos.size(), qPrintable(where));
                }
            }
        });
    }
};

QTEST_GUILESS_MAIN(TestTodos)
#include "tst_todos.moc"
