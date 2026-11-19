// Runs the Qt Quick Tests in tests/qml/ with a fixture: todo files in a temporary folder, which
// the QML tests write and read back through the singleton Fixture (module topdo.test).

#include <QFile>
#include <QQmlEngine>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QtQuickTest>

namespace {

class Fixture : public QObject {
    Q_OBJECT

  public:
    // Writes a new todo file and returns its path; each call gets a file of its own.
    Q_INVOKABLE QString write(const QString &contents) {
        ++m_count;
        const QString path = m_dir.filePath(QStringLiteral("todo-%1.md").arg(m_count));
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            qFatal("cannot write %s", qPrintable(path));
        }
        file.write(contents.toUtf8());
        file.commit();
        return path;
    }

    Q_INVOKABLE [[nodiscard]] QString read(const QString &path) const {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return {};
        }
        return QString::fromUtf8(file.readAll());
    }

  private:
    QTemporaryDir m_dir;
    int m_count = 0;
};

class Setup : public QObject {
    Q_OBJECT

  public Q_SLOTS:
    void qmlEngineAvailable(QQmlEngine *engine) {
        engine->addImportPath(QStringLiteral(TOPDO_QML_DIR));
        qmlRegisterSingletonInstance("topdo.test", 1, 0, "Fixture", &m_fixture);
    }

  private:
    Fixture m_fixture;
};

} // namespace

QUICK_TEST_MAIN_WITH_SETUP(topdo, Setup)

#include "tst_quick.moc"
