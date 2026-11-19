#include "todos.h"
#include "todoline.h"

#include <QRegularExpression>

namespace topdo {

namespace {

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
        const std::optional<TodoLine> match = matchTodoLine(line);
        if (!match) {
            continue;
        }
        const DueDate split = splitDueDate(match->text);
        list.todos.append({
            .line = i,
            .parent = popToParent(ancestors, match->indent),
            .section = list.sections.size() - 1,
            .done = match->done,
            .text = match->text,
            .title = split.title,
            .due = split.due,
        });
        ancestors.append({.indent = match->indent, .todo = list.todos.size() - 1});
    }
    return list;
}

} // namespace topdo
