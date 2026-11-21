#include "edits.h"
#include "store.h"
#include "todos.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using topdo::Document;

namespace {

void writeFile(const QString &path, const QByteArray &bytes) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qFatal("cannot write %s", qPrintable(path));
    }
    file.write(bytes);
}

QByteArray readFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qFatal("cannot read %s", qPrintable(path));
    }
    return file.readAll();
}

Document tickFirst(const Document &document) { return topdo::setDone(document, 0, true); }

} // namespace

class TestStore : public QObject {
    Q_OBJECT

  private Q_SLOTS:
    // A missing file reads as the preamble, which holds no todos.
    void loadsAMissingFileAsANewOne() {
        const QTemporaryDir dir;
        const auto loaded = topdo::load(dir.filePath("todo.md"));

        QVERIFY(loaded);
        QCOMPARE(*loaded, topdo::newTodoFile());
        QVERIFY(topdo::findTodos(*loaded).todos.isEmpty());
        QCOMPARE(topdo::findTodos(*loaded).sections.size(), 1);
    }

    void savesAndLoads() {
        const QTemporaryDir dir;
        const QString path = dir.filePath("todo.md");
        const Document document{.lines = {"# Todo", "- [ ] a"}};

        QVERIFY(topdo::save(path, document));

        QCOMPARE(readFile(path), QByteArray("# Todo\n- [ ] a\n"));
        QCOMPARE(topdo::load(path).value(), document);
    }

    void saveCreatesTheFolder() {
        const QTemporaryDir dir;
        const QString path = dir.filePath("todo/nested/todo.md");

        QVERIFY(topdo::save(path, {.lines = {"- [ ] a"}}));
        QVERIFY(QFile::exists(path));
    }

    void reportsWhatFailed() {
        const QTemporaryDir dir;
        const QString notAFile = dir.path(); // a folder can't be read as a todo file
        const QString file = dir.filePath("file");
        writeFile(file, "x");
        const QString underAFile = file + "/todo.md"; // its folder would be a file

        const auto loaded = topdo::load(notAFile);
        const auto saved = topdo::save(underAFile, {.lines = {"- [ ] a"}});

        QVERIFY(!loaded);
        QVERIFY(loaded.error().contains(notAFile));
        QVERIFY(!saved);
        QVERIFY(saved.error().contains(file));
    }

    void updateWritesTheChange() {
        const QTemporaryDir dir;
        const QString path = dir.filePath("todo.md");
        writeFile(path, "- [ ] a\n");

        const auto updated = topdo::update(path, tickFirst);

        QVERIFY(updated);
        QCOMPARE(readFile(path), QByteArray("- [x] a\n"));
    }

    void updateLeavesAnUnchangedFileAlone() {
        const QTemporaryDir dir;
        const QString path = dir.filePath("todo.md");

        const auto updated = topdo::update(path, [](const Document &document) { return document; });

        QVERIFY(updated);
        QVERIFY(!QFile::exists(path)); // nothing to write, so no empty file appears
    }

    // An edit made elsewhere since the last load (say, in a text editor) is kept.
    void updateStartsFromTheFileOnDisk() {
        const QTemporaryDir dir;
        const QString path = dir.filePath("todo.md");
        writeFile(path, "- [ ] a\n");
        const Document seen = topdo::load(path).value();
        writeFile(path, "- [ ] a\n- [ ] added in an editor\n");

        const auto updated = topdo::update(path, tickFirst);

        QVERIFY(updated);
        QCOMPARE(seen.lines.size(), 1);
        QCOMPARE(readFile(path), QByteArray("- [x] a\n- [ ] added in an editor\n"));
    }

    void readsTheTodoFolderFromTheConfig_data() {
        QTest::addColumn<QByteArray>("config");
        QTest::addColumn<QString>("dir");

        QTest::newRow("no config file") << QByteArray() << QDir::homePath() + "/Documents/todo";
        QTest::newRow("home") << QByteArray("dir=~/notes/todo\n")
                              << QDir::homePath() + "/notes/todo";
        QTest::newRow("absolute") << QByteArray("dir=/srv/todo\n") << QString("/srv/todo");
    }

    void readsTheTodoFolderFromTheConfig() {
        QFETCH(QByteArray, config);
        QFETCH(QString, dir);
        const QTemporaryDir temporary;
        const QString configFile = temporary.filePath("topdo.conf");
        if (!config.isEmpty()) {
            writeFile(configFile, config);
        }

        QCOMPARE(topdo::configuredDir(configFile), dir);
    }
};

QTEST_GUILESS_MAIN(TestStore)
#include "tst_store.moc"
