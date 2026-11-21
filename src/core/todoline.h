#pragma once

// The line rules of docs/design/format.md, shared by todos.cpp and edits.cpp. Internal to core.

#include <QString>
#include <QStringList>

#include <functional>
#include <optional>

namespace topdo {

// A todo line split into its parts, rules 2 and 3. joinTodoLine(splitTodoLine(line)) == line for
// every line that isn't blank.
struct TodoLine {
    bool done = false;
    std::optional<QChar> priority;    // open todos only: the letter of "(A) "
    std::optional<QString> completed; // done todos only: the date after "x "
    std::optional<QString> created;   // the creation date, as written
    QString text;
};

bool isBlank(const QString &line);

// The parts of a line that isn't blank.
TodoLine splitTodoLine(const QString &line);

QString joinTodoLine(const TodoLine &todo);

// The value of the first "key:value" word of the text, if it has one.
std::optional<QString> tagValue(const QString &text, const QString &key);

// The text without the words for which `drop` returns true. Words are separated by spaces and tabs;
// the remaining ones are joined with single spaces.
QString withoutWords(const QString &text, const std::function<bool(const QString &)> &drop);

// The words of the text: runs of characters without spaces or tabs.
QStringList wordsOf(const QString &text);

} // namespace topdo
