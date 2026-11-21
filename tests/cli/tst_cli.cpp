#include "cli.h"

#include <QDate>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

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

QString today() { return QDate::currentDate().toString(Qt::ISODate); }

struct Result {
    int status = 0;
    QString out;
    QString err;
};

// Runs topdo with `--dir` pointing at a temporary todo folder.
class Cli {
  public:
    explicit Cli(const QByteArray &todoFile = {}) {
        if (todoFile.isEmpty()) {
            return;
        }
        QFile file(m_dir.filePath("todo.txt"));
        if (!file.open(QIODevice::WriteOnly)) {
            qFatal("cannot write the test todo file");
        }
        file.write(todoFile);
    }

    [[nodiscard]] Result run(const QStringList &arguments) const {
        Result result;
        QTextStream out(&result.out);
        QTextStream err(&result.err);
        const QStringList all = QStringList{"topdo", "--dir", m_dir.path()} + arguments;
        result.status = topdo::runCli(all, out, err);
        return result;
    }

    [[nodiscard]] QByteArray contents() const {
        QFile file(todoFile());
        if (!file.open(QIODevice::ReadOnly)) {
            return {};
        }
        return file.readAll();
    }

    [[nodiscard]] QString todoFile() const { return m_dir.filePath("todo.txt"); }

  private:
    QTemporaryDir m_dir;
};

} // namespace

class TestCli : public QObject {
    Q_OBJECT

  private Q_SLOTS:
    void listsOpenTodosAsWritten() {
        const Result result = Cli(exampleFile()).run({"list"});

        QCOMPARE(result.status, 0);
        QCOMPARE(result.out,
                 QString("1  (A) Call the bank @phone due:2026-11-21\n"
                         "2  Buy milk\n"
                         "3  Plan trip +Holiday id:1\n"
                         "5  Book hotel +Holiday p:1\n"
                         "6  2026-11-18 Write report +Work\n"));
        QCOMPARE(result.err, QString());
    }

    void listsDoneTodosWithAll() {
        const Result result = Cli(exampleFile()).run({"list", "--all"});

        QVERIFY(result.out.contains("4  x 2026-11-20 Book flight +Holiday p:1\n"));
        QVERIFY(result.out.contains("7  x 2026-11-19 Renew passport pri:B\n"));
    }

    void alignsNumbers() {
        QByteArray file;
        for (int i = 1; i <= 10; ++i) {
            file += "t" + QByteArray::number(i) + "\n";
        }

        const Result result = Cli(file).run({"list"});

        QVERIFY(result.out.startsWith(" 1  t1\n"));
        QVERIFY(result.out.endsWith("10  t10\n"));
    }

    void listsNothingWithoutAFile() {
        const Result result = Cli().run({"list"});

        QCOMPARE(result.status, 0);
        QCOMPARE(result.out, QString());
    }

    void printsThePath() {
        const Cli cli;
        const Result result = cli.run({"path"});

        QCOMPARE(result.out, cli.todoFile() + "\n");
    }

    void rejectsUnknownCommandsAndOptions_data() {
        QTest::addColumn<QStringList>("arguments");
        QTest::addColumn<QString>("message");

        QTest::newRow("no command") << QStringList{} << "topdo: no command given";
        QTest::newRow("unknown command")
            << QStringList{"frobnicate"} << "topdo: unknown command \"frobnicate\"";
        QTest::newRow("unknown option")
            << QStringList{"list", "--bogus"} << "topdo: Unknown option 'bogus'.";
        QTest::newRow("section and under")
            << QStringList{"add", "a", "--project", "Home", "--under", "1"}
            << "topdo: add takes --project or --under, not both";
    }

    void rejectsUnknownCommandsAndOptions() {
        QFETCH(QStringList, arguments);
        QFETCH(QString, message);

        const Result result = Cli().run(arguments);

        QCOMPARE(result.status, 2);
        QVERIFY2(result.err.startsWith(message), qPrintable(result.err));
        QVERIFY(result.err.contains("Usage:"));
    }

    void addsTodos() {
        const Cli cli(exampleFile());

        const Result first = cli.run({"add", "Pay", "rent", "due:2026-12-01"});
        const Result second = cli.run({"add", "Water", "plants", "--project", "Home"});

        QCOMPARE(first.status, 0);
        QCOMPARE(first.out, QString()); // silent on success
        QCOMPARE(second.status, 0);
        QCOMPARE(cli.contents(), exampleFile() + "Pay rent due:2026-12-01\nWater plants +Home\n");
    }

    void addsSubtasks() {
        const Cli cli(exampleFile());

        QCOMPARE(cli.run({"add", "--under", "3", "Pack", "bags"}).status, 0);
        QCOMPARE(cli.run({"add", "Find", "branch", "--under", "1"}).status, 0);

        QVERIFY(cli.contents().startsWith("(A) Call the bank @phone due:2026-11-21 id:2\n"
                                          "Find branch p:2\n"));
        QVERIFY(cli.contents().contains("Book hotel +Holiday p:1\nPack bags p:1\n"));
    }

    void addCreatesTheFile() {
        const Cli cli;

        QCOMPARE(cli.run({"add", "first"}).status, 0);
        QCOMPARE(cli.contents(), QByteArray("first\n"));
    }

    void addKeepsOneTodoOnOneLine() {
        const Cli cli;

        QCOMPARE(cli.run({"add", "two\nlines"}).status, 0);
        QCOMPARE(cli.contents(), QByteArray("two lines\n"));
    }

    void ticksAndUnticks() {
        const Cli cli(exampleFile());

        QCOMPARE(cli.run({"done", "1", "2"}).status, 0);
        QCOMPARE(cli.run({"undone", "7"}).status, 0);

        const QString contents = QString::fromUtf8(cli.contents());
        QVERIFY(contents.startsWith("x " + today() +
                                    " Call the bank @phone due:2026-11-21 pri:A\n"
                                    "x " +
                                    today() + " Buy milk\n"));
        QVERIFY(contents.endsWith("\n(B) Renew passport\n"));
    }

    void editsText() {
        const Cli cli(exampleFile());

        QCOMPARE(cli.run({"edit", "6", "Write", "the", "report"}).status, 0);

        QVERIFY(cli.contents().contains("\n2026-11-18 Write the report\n"));
    }

    // Numbers refer to the list before the command: removing 3 (with its subtasks 4 and 5)
    // and 7 removes those todos, whatever order they are given in.
    void removesTodos() {
        const Cli cli(exampleFile());

        QCOMPARE(cli.run({"rm", "7", "3", "4"}).status, 0);

        const Result list = cli.run({"list", "--all"});
        QCOMPARE(list.out,
                 QString("1  (A) Call the bank @phone due:2026-11-21\n"
                         "2  Buy milk\n"
                         "3  2026-11-18 Write report +Work\n"));
    }

    void changesNothingOnABadArgument_data() {
        QTest::addColumn<QStringList>("arguments");
        QTest::addColumn<QString>("message");

        QTest::newRow("number too high") << QStringList{"done", "2", "9"} << "topdo: no todo 9\n";
        QTest::newRow("zero") << QStringList{"rm", "0"} << "topdo: no todo 0\n";
        QTest::newRow("not a number") << QStringList{"undone", "x"} << "topdo: no todo x\n";
        QTest::newRow("no todo to add under")
            << QStringList{"add", "a", "--under", "9"} << "topdo: no todo 9\n";
    }

    void changesNothingOnABadArgument() {
        QFETCH(QStringList, arguments);
        QFETCH(QString, message);
        const Cli cli(exampleFile());

        const Result result = cli.run(arguments);

        QCOMPARE(result.status, 1);
        QCOMPARE(result.err, message);
        QCOMPARE(cli.contents(), exampleFile());
    }

    void needsArguments_data() {
        QTest::addColumn<QStringList>("arguments");

        QTest::newRow("add") << QStringList{"add"};
        QTest::newRow("done") << QStringList{"done"};
        QTest::newRow("rm") << QStringList{"rm"};
        QTest::newRow("edit without text") << QStringList{"edit", "1"};
        QTest::newRow("edit") << QStringList{"edit"};
    }

    void needsArguments() {
        QFETCH(QStringList, arguments);
        const Cli cli(exampleFile());

        const Result result = cli.run(arguments);

        QCOMPARE(result.status, 2);
        QVERIFY(result.err.contains("Usage:"));
        QCOMPARE(cli.contents(), exampleFile());
    }
};

QTEST_GUILESS_MAIN(TestCli)
#include "tst_cli.moc"
