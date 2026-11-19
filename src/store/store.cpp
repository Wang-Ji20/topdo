#include "store.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>

namespace topdo {

namespace {

QString expandHome(const QString &path) {
    if (path == u"~" || path.startsWith(u"~/")) {
        return QDir::homePath() + path.sliced(1);
    }
    return path;
}

} // namespace

QString defaultConfigFile() {
    const QString configHome =
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    return configHome + u"/topdo/topdo.conf";
}

QString configuredDir(const QString &configFile) {
    const QSettings settings(configFile, QSettings::IniFormat);
    const QString dir = settings.value("dir", "~/Documents/todo").toString();
    return expandHome(dir);
}

QString todoFilePath(const QString &dir) { return dir + u"/todo.md"; }

std::expected<Document, QString> load(const QString &path) {
    QFile file(path);
    if (!file.exists()) {
        return Document{};
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return std::unexpected(QStringLiteral("cannot read %1: %2").arg(path, file.errorString()));
    }
    return parse(file.readAll());
}

std::expected<void, QString> save(const QString &path, const Document &document) {
    const QString dir = QFileInfo(path).absolutePath();
    if (!QDir().mkpath(dir)) {
        return std::unexpected(QStringLiteral("cannot create folder %1").arg(dir));
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return std::unexpected(QStringLiteral("cannot write %1: %2").arg(path, file.errorString()));
    }
    file.write(render(document));
    if (!file.commit()) {
        return std::unexpected(QStringLiteral("cannot write %1: %2").arg(path, file.errorString()));
    }
    return {};
}

std::expected<Document, QString> update(const QString &path,
                                        const std::function<Document(const Document &)> &change) {
    const std::expected<Document, QString> loaded = load(path);
    if (!loaded) {
        return std::unexpected(loaded.error());
    }
    Document changed = change(*loaded);
    if (changed == *loaded) {
        return changed;
    }
    const std::expected<void, QString> saved = save(path, changed);
    if (!saved) {
        return std::unexpected(saved.error());
    }
    return changed;
}

} // namespace topdo
