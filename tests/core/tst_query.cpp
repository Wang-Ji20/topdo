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
            "- [ ] Call the bank due:2026-11-21",
            "- [x] Renew passport",
            "- [ ] Buy milk",
        });

        QCOMPARE(topdo::filterTodos(list, filter, showDone), shown);
    }

    void picksTheNextTodo_data() {
        QTest::addColumn<QStringList>("lines");
        QTest::addColumn<int>("next"); // -1: none

        QTest::newRow("first open") << QStringList{"- [x] a", "- [ ] b", "- [ ] c"} << 1;
        QTest::newRow("due today beats first")
            << QStringList{"- [ ] a", "- [ ] b due:2026-11-19"} << 1;
        QTest::newRow("late beats due today")
            << QStringList{"- [ ] a due:2026-11-19", "- [ ] b due:2026-11-18"} << 1;
        QTest::newRow("due later doesn't count")
            << QStringList{"- [ ] a", "- [ ] b due:2026-11-20"} << 0;
        QTest::newRow("done doesn't count")
            << QStringList{"- [ ] a", "- [x] b due:2026-11-01"} << 0;
        QTest::newRow("subtasks count") << QStringList{"- [ ] a", "  - [ ] b due:2026-11-18"} << 1;
        QTest::newRow("nothing open") << QStringList{"- [x] a"} << -1;
    }

    void picksTheNextTodo() {
        QFETCH(QStringList, lines);
        QFETCH(int, next);

        const std::optional<qsizetype> picked = topdo::nextTodo(todosIn(lines), today());

        QCOMPARE(picked.value_or(-1), static_cast<qsizetype>(next));
    }

    void countsDepth() {
        const TodoList list = todosIn({"- [ ] a", "  - [ ] b", "    - [ ] c", "- [ ] d"});

        QCOMPARE(topdo::depthOf(list, 0), 0);
        QCOMPARE(topdo::depthOf(list, 1), 1);
        QCOMPARE(topdo::depthOf(list, 2), 2);
        QCOMPARE(topdo::depthOf(list, 3), 0);
    }
};

QTEST_GUILESS_MAIN(TestQuery)
#include "tst_query.moc"
