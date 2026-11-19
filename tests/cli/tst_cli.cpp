#include "cli.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

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
           "- [x] Renew passport\n"
           "\n"
           "Notes that are not todos stay exactly as written.\n";
}

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
        QFile file(m_dir.filePath("todo.md"));
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

    [[nodiscard]] QString todoFile() const { return m_dir.filePath("todo.md"); }

  private:
    QTemporaryDir m_dir;
};

} // namespace

class TestCli : public QObject {
    Q_OBJECT

  private Q_SLOTS:
    void listsOpenTodosAndHeadings() {
        const Result result = Cli(exampleFile()).run({"list"});

        QCOMPARE(result.status, 0);
        QCOMPARE(result.out,
                 QString("1  - [ ] Buy milk\n"
                         "## Work\n"
                         "2  - [ ] Write report due:2026-11-21\n"
                         "3  - [ ] Plan trip\n"
                         "5    - [ ] Book hotel\n"
                         "## Home\n"
                         "6  * [ ] Call the bank\n"));
        QCOMPARE(result.err, QString());
    }

    void listsDoneTodosWithAll() {
        const Result result = Cli(exampleFile()).run({"list", "--all"});

        QVERIFY(result.out.contains("4    - [x] Book flight\n"));
        QVERIFY(result.out.contains("7  - [x] Renew passport\n"));
    }

    void alignsNumbers() {
        QByteArray file;
        for (int i = 1; i <= 10; ++i) {
            file += "- [ ] t" + QByteArray::number(i) + "\n";
        }

        const Result result = Cli(file).run({"list"});

        QVERIFY(result.out.startsWith(" 1  - [ ] t1\n"));
        QVERIFY(result.out.endsWith("10  - [ ] t10\n"));
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
    }

    void rejectsUnknownCommandsAndOptions() {
        QFETCH(QStringList, arguments);
        QFETCH(QString, message);

        const Result result = Cli().run(arguments);

        QCOMPARE(result.status, 2);
        QVERIFY2(result.err.startsWith(message), qPrintable(result.err));
        QVERIFY(result.err.contains("Usage:"));
    }
};

QTEST_GUILESS_MAIN(TestCli)
#include "tst_cli.moc"
