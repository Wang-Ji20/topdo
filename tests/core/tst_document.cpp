#include "document.h"
#include "randomfile.h"

#include <QDir>
#include <QFile>
#include <QTest>

namespace {

// Seed for the property tests: TOPDO_SEED replays a failure, otherwise each run picks a new one.
quint32 propertySeed() {
    bool ok = false;
    const quint32 fromEnvironment = qEnvironmentVariable("TOPDO_SEED").toUInt(&ok);
    return ok ? fromEnvironment : QRandomGenerator::global()->generate();
}

QByteArray readFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qFatal("cannot read %s", qPrintable(path));
    }
    return file.readAll();
}

} // namespace

class TestDocument : public QObject {
    Q_OBJECT

  private Q_SLOTS:
    void splitsLines_data() {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<QStringList>("lines");

        QTest::newRow("empty") << QByteArray() << QStringList{};
        QTest::newRow("one line") << QByteArray("a\n") << QStringList{"a"};
        QTest::newRow("blank lines") << QByteArray("a\n\n\nb\n") << QStringList{"a", "", "", "b"};
        QTest::newRow("no final newline") << QByteArray("a\nb") << QStringList{"a", "b"};
        QTest::newRow("CRLF") << QByteArray("a\r\nb\r\n") << QStringList{"a", "b"};
        QTest::newRow("UTF-8") << QByteArray("- [ ] 中文\n") << QStringList{"- [ ] 中文"};
        QTest::newRow("byte order mark") << QByteArray("\xEF\xBB\xBF"
                                                       "a\n")
                                         << QStringList{"a"};
    }

    void splitsLines() {
        QFETCH(QByteArray, input);
        QFETCH(QStringList, lines);

        QCOMPARE(topdo::parse(input).lines, lines);
    }

    void writesLfAndFinalNewline() {
        const topdo::Document document = topdo::parse("a\r\nb");
        QCOMPARE(topdo::render(document), QByteArray("a\nb\n"));
    }

    void roundTripsGoldenFiles() {
        const QDir dir(QStringLiteral(TOPDO_TEST_DATA "/roundtrip"));
        const QFileInfoList files = dir.entryInfoList(QDir::Files);
        QVERIFY(!files.isEmpty());
        for (const QFileInfo &info : files) {
            const QByteArray bytes = readFile(info.filePath());
            const QByteArray rendered = topdo::render(topdo::parse(bytes));
            QVERIFY2(rendered == bytes, qPrintable(info.fileName()));
        }
    }

    void roundTripsRandomFiles() {
        const quint32 seed = propertySeed();
        qInfo("seed %u (replay with TOPDO_SEED=%u)", seed, seed);
        QRandomGenerator random(seed);
        for (int i = 0; i < 2000; ++i) {
            const QByteArray bytes = topdo::test::randomTodoFile(random);
            const QByteArray rendered = topdo::render(topdo::parse(bytes));
            QVERIFY2(rendered == bytes,
                     qPrintable(QStringLiteral("seed %1, file %2").arg(seed).arg(i)));
        }
    }
};

QTEST_GUILESS_MAIN(TestDocument)
#include "tst_document.moc"
