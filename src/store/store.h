#pragma once

#include "document.h"

#include <QString>

#include <expected>
#include <functional>

namespace topdo {

// ~/.config/topdo/topdo.conf: the config file shared by the widget, the CLI and the MCP server.
QString defaultConfigFile();

// The todo folder: the `dir` key of the config file, or ~/Documents/todo if there is none. A
// leading "~/" means the home folder.
QString configuredDir(const QString &configFile);

// The todo file in a todo folder.
QString todoFilePath(const QString &dir);

// Reads a todo file. A missing file reads as newTodoFile(), with no todos; the first save
// creates it.
std::expected<Document, QString> load(const QString &path);

// Writes a todo file, creating its folder if needed. The write is atomic: after a crash the file
// is either the old one or the new one, never half of each.
std::expected<void, QString> save(const QString &path, const Document &document);

// Reads the file, applies `change`, and writes the result if it differs. Every edit starts from
// what is on disk at that moment, so an edit made elsewhere in between isn't lost.
std::expected<Document, QString> update(const QString &path,
                                        const std::function<Document(const Document &)> &change);

} // namespace topdo
