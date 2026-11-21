#include "query.h"

#include <QTest>

using topdo::Document;
using topdo::TodoList;

namespace {

TodoList todosIn(const QStringList &lines) {
    const Document document{.lines = lines};
    return topdo::findTodos(document);
}

QDate today() { return {2026, 11, 19}; }

} // namespace

class TestQuery : public QObject {
    Q_OBJECT

  private Q_SLOTS:
    void filtersTodos_data() {
        QTest::addColumn<QString>("filter");
        QTest::addColumn<bool>("showDone");
        QTest::addColumn<QList<qsizetype>>("shown");

        QTest::newRow("open only") << QString() << false << QList<qsizetype>{0, 2};
        QTest::newRow("with done") << QString() << true << QList<qsizetype>{0, 1, 2};
        QTest::newRow("filter ignores case") << QString("BANK") << false << QList<qsizetype>{0};
        QTest::newRow("filter finds done todos") << QString("pass") << false << QList<qsizetype>{1};
        QTest::newRow("filter skips due dates") << QString("due") << false << QList<qsizetype>{};
    }

    void filtersTodos() {
        QFETCH(QString, filter);
        QFETCH(bool, showDone);
        QFETCH(QList<qsizetype>, shown);
        const TodoList list = todosIn({
            "Call the bank due:2026-11-21",
            "x Renew passport",
            "Buy milk",
        });

        QCOMPARE(topdo::filterTodos(list, filter, showDone), shown);
    }

    void picksTheNextTodo_data() {
        QTest::addColumn<QStringList>("lines");
        QTest::addColumn<int>("next"); // -1: none

        QTest::newRow("first open") << QStringList{"x a", "b", "c"} << 1;
        QTest::newRow("due today beats first") << QStringList{"a", "b due:2026-11-19"} << 1;
        QTest::newRow("late beats due today")
            << QStringList{"a due:2026-11-19", "b due:2026-11-18"} << 1;
        QTest::newRow("due later doesn't count") << QStringList{"a", "b due:2026-11-20"} << 0;
        QTest::newRow("done doesn't count") << QStringList{"a", "x b due:2026-11-01"} << 0;
        QTest::newRow("subtasks count") << QStringList{"a id:1", "b due:2026-11-18 p:1"} << 1;
        QTest::newRow("nothing open") << QStringList{"x a"} << -1;
    }

    void picksTheNextTodo() {
        QFETCH(QStringList, lines);
        QFETCH(int, next);

        const std::optional<qsizetype> picked = topdo::nextTodo(todosIn(lines), today());

        QCOMPARE(picked.value_or(-1), static_cast<qsizetype>(next));
    }

    void sortsBySection() {
        const TodoList list = todosIn({"a +Work", "b", "c +Home", "d +Work", "e"});

        QCOMPARE(topdo::filterTodos(list, {}, false), (QList<qsizetype>{1, 4, 0, 3, 2}));
    }

    void findsTodosDueOnADay() {
        const TodoList list = todosIn({
            "a due:2026-11-19",
            "x b due:2026-11-19",
            "c due:2026-11-18",
            "d",
            "e due:2026-11-19",
        });

        QCOMPARE(topdo::dueOn(list, today()), (QList<qsizetype>{0, 4}));
    }

    void findsMilestonesAndSteps() {
        const TodoList list = todosIn({
            "Paper +AidWalk id:1",          // 0: milestone
            "x Run baselines +AidWalk p:1", // 1: step
            "Write discussion p:1",         // 2: step without the project
            "Lunch",                        // 3: other section
            "x Data +AidWalk",              // 4: milestone without steps
            "Figure +AidWalk p:9",          // 5: unknown parent: top-level, a milestone
        });

        QCOMPARE(topdo::milestonesOf(list, 1), (QList<qsizetype>{0, 4, 5}));
        QCOMPARE(topdo::subtasksOf(list, 0), (QList<qsizetype>{1, 2}));
        QCOMPARE(topdo::subtasksOf(list, 4), QList<qsizetype>{});
    }

    void countsDepth() {
        const TodoList list = todosIn({"a id:1", "b id:2 p:1", "c p:2", "d"});

        QCOMPARE(topdo::depthOf(list, 0), 0);
        QCOMPARE(topdo::depthOf(list, 1), 1);
        QCOMPARE(topdo::depthOf(list, 2), 2);
        QCOMPARE(topdo::depthOf(list, 3), 0);
    }
};

QTEST_GUILESS_MAIN(TestQuery)
#include "tst_query.moc"
