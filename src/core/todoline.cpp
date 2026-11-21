#include "todoline.h"

#include <QRegularExpression>

namespace topdo {

namespace {

// A date word followed by a space, at `from`. Its value isn't checked: the spec places dates by
// position, and an odd one stays as written.
std::optional<QString> dateAt(const QString &line, qsizetype from) {
    static const QRegularExpression date(QStringLiteral(R"(\d{4}-\d{2}-\d{2} )"));
    const QRegularExpressionMatch match = date.match(
        line, from, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption);
    if (!match.hasMatch()) {
        return std::nullopt;
    }
    return match.captured(0).chopped(1);
}

std::optional<QChar> priorityAt(const QString &line) {
    static const QRegularExpression priority(QStringLiteral(R"(^\(([A-Z])\) )"));
    const QRegularExpressionMatch match = priority.match(line);
    if (!match.hasMatch()) {
        return std::nullopt;
    }
    return match.captured(1).at(0);
}

// Reads an optional date at `at` and moves `at` past it.
std::optional<QString> takeDate(const QString &line, qsizetype &at) {
    std::optional<QString> date = dateAt(line, at);
    if (date) {
        at += date->size() + 1;
    }
    return date;
}

} // namespace

bool isBlank(const QString &line) { return line.trimmed().isEmpty(); }

TodoLine splitTodoLine(const QString &line) {
    TodoLine todo;
    qsizetype at = 0;
    if (line.startsWith(u"x ")) {
        todo.done = true;
        at = 2;
        todo.completed = takeDate(line, at);
    } else {
        todo.priority = priorityAt(line);
        at = todo.priority ? 4 : 0;
    }
    todo.created = takeDate(line, at);
    todo.text = line.sliced(at);
    return todo;
}

QString joinTodoLine(const TodoLine &todo) {
    QString line;
    if (todo.done) {
        line += u"x ";
    }
    if (todo.priority) {
        line += u'(' + *todo.priority + u") ";
    }
    for (const std::optional<QString> &date : {todo.completed, todo.created}) {
        if (date) {
            line += *date + u' ';
        }
    }
    return line + todo.text;
}

QStringList wordsOf(const QString &text) {
    static const QRegularExpression space(QStringLiteral("[ \t]+"));
    return text.split(space, Qt::SkipEmptyParts);
}

std::optional<QString> tagValue(const QString &text, const QString &key) {
    const QString prefix = key + u':';
    for (const QString &word : wordsOf(text)) {
        if (word.startsWith(prefix) && word.size() > prefix.size()) {
            return word.sliced(prefix.size());
        }
    }
    return std::nullopt;
}

QString withoutWords(const QString &text, const std::function<bool(const QString &)> &drop) {
    QStringList kept = wordsOf(text);
    kept.removeIf(drop);
    return kept.join(u' ');
}

} // namespace topdo
