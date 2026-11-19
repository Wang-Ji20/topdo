#include "todos.h"

#include <QRegularExpression>

namespace topdo {

namespace {

constexpr int TabWidth = 4;

bool isFence(const QString &line) {
    const QString trimmed = line.trimmed();
    return trimmed.startsWith(u"```") || trimmed.startsWith(u"~~~");
}

std::optional<QString> headingTitle(const QString &line) {
    static const QRegularExpression heading(QStringLiteral("^#{1,6} (.*)$"));
    const QRegularExpressionMatch match = heading.match(line);
    if (!match.hasMatch()) {
        return std::nullopt;
    }
    return match.captured(1).trimmed();
}

// Indentation in columns; a tab counts as four spaces.
int indentation(const QString &line) {
    int columns = 0;
    for (const QChar ch : line) {
        if (ch == u' ') {
            columns += 1;
        } else if (ch == u'\t') {
            columns += TabWidth;
        } else {
            break;
        }
    }
    return columns;
}

struct DueDate {
    QString title;
    std::optional<QDate> due;
};

// Splits off the first valid due:YYYY-MM-DD word.
DueDate splitDueDate(const QString &text) {
    static const QRegularExpression dueWord(
        QStringLiteral(R"((?:^|\s)due:(\d{4}-\d{2}-\d{2})(?=\s|$))"));
    QRegularExpressionMatchIterator matches = dueWord.globalMatch(text);
    while (matches.hasNext()) {
        const QRegularExpressionMatch match = matches.next();
        const QDate date = QDate::fromString(match.captured(1), Qt::ISODate);
        if (!date.isValid()) {
            continue;
        }
        QString title = text;
        title.remove(match.capturedStart(), match.capturedLength());
        return {.title = title.simplified(), .due = date};
    }
    return {.title = text.simplified(), .due = std::nullopt};
}

// A todo with its line, done and text filled in; parent and section are left to the caller.
std::optional<Todo> todoLine(const QString &line, qsizetype lineIndex) {
    static const QRegularExpression todo(
        QStringLiteral(R"(^[ \t]*[-*+][ \t]+\[([ xX])\][ \t]+(.*)$)"));
    const QRegularExpressionMatch match = todo.match(line);
    if (!match.hasMatch()) {
        return std::nullopt;
    }
    const QString text = match.captured(2);
    const DueDate split = splitDueDate(text);
    return Todo{
        .line = lineIndex,
        .done = match.captured(1) != u" ",
        .text = text,
        .title = split.title,
        .due = split.due,
    };
}

// A todo that later, more indented todos can be subtasks of.
struct Ancestor {
    int indent = 0;
    qsizetype todo = 0;
};

// The parent of a todo at this indentation: the nearest ancestor with less indentation.
// Ancestors that are indented as much or more can't hold later todos, so they are dropped.
qsizetype popToParent(QList<Ancestor> &ancestors, int indent) {
    while (!ancestors.isEmpty() && ancestors.last().indent >= indent) {
        ancestors.removeLast();
    }
    return ancestors.isEmpty() ? -1 : ancestors.last().todo;
}

} // namespace

TodoList findTodos(const Document &document) {
    TodoList list{.sections = {Section{}}, .todos = {}};
    QList<Ancestor> ancestors;
    bool inFence = false;
    for (qsizetype i = 0; i < document.lines.size(); ++i) {
        const QString &line = document.lines.at(i);
        if (isFence(line)) {
            inFence = !inFence;
            continue;
        }
        if (inFence) {
            continue;
        }
        if (const std::optional<QString> title = headingTitle(line)) {
            list.sections.append({.title = *title, .line = i});
            ancestors.clear();
            continue;
        }
        std::optional<Todo> todo = todoLine(line, i);
        if (!todo) {
            continue;
        }
        const int indent = indentation(line);
        todo->parent = popToParent(ancestors, indent);
        todo->section = list.sections.size() - 1;
        list.todos.append(*todo);
        ancestors.append({.indent = indent, .todo = list.todos.size() - 1});
    }
    return list;
}

} // namespace topdo
