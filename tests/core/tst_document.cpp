#include "document.h"
#include "randomfile.h"

#include <QDir>
#include <QFile>
#include <QTest>

using topdo::Document;
using topdo::Line;

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
        QTest::addColumn<bool>("byteOrderMark");
        QTest::addColumn<QByteArrayList>("texts");
        QTest::addColumn<QByteArrayList>("endings");

        QTest::newRow("empty") << QByteArray() << false << QByteArrayList{} << QByteArrayList{};
        QTest::newRow("one line") << QByteArray("a\n") << false << QByteArrayList{"a"}
                                  << QByteArrayList{"\n"};
        QTest::newRow("no final newline")
            << QByteArray("a\nb") << false << QByteArrayList{"a", "b"} << QByteArrayList{"\n", ""};
        QTest::newRow("mixed endings")
            << QByteArray("a\r\nb\n\r\n") << false << QByteArrayList{"a", "b", ""}
            << QByteArrayList{"\r\n", "\n", "\r\n"};
        QTest::newRow("lone CR stays in the text")
            << QByteArray("a\rb\n") << false << QByteArrayList{"a\rb"} << QByteArrayList{"\n"};
        QTest::newRow("byte order mark") << QByteArray("\xEF\xBB\xBF"
                                                       "a\n")
                                         << true << QByteArrayList{"a"} << QByteArrayList{"\n"};
    }

    void splitsLines() {
        QFETCH(QByteArray, input);
        QFETCH(bool, byteOrderMark);
        QFETCH(QByteArrayList, texts);
        QFETCH(QByteArrayList, endings);

        const Document document = topdo::parse(input);

        QCOMPARE(document.hasByteOrderMark, byteOrderMark);
        QCOMPARE(document.lines.size(), texts.size());
        for (qsizetype i = 0; i < texts.size(); ++i) {
            QCOMPARE(document.lines.at(i).text, texts.at(i));
            QCOMPARE(document.lines.at(i).ending, endings.at(i));
        }
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
