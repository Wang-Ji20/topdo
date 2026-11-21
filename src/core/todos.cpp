#include "todos.h"
#include "todoline.h"

#include <QHash>

using namespace Qt::StringLiterals;

namespace topdo {

namespace {

// The first valid due:YYYY-MM-DD tag; rule 5.
std::optional<QDate> dueDate(const QString &text) {
    for (const QString &word : wordsOf(text)) {
        if (!word.startsWith(u"due:")) {
            continue;
        }
        const QDate date = QDate::fromString(word.sliced(4), Qt::ISODate);
        if (date.isValid() && word.size() == 14) {
            return date;
        }
    }
    return std::nullopt;
}

// Whether a word is one of the tags that the title leaves out; rule 8.
bool isHiddenTag(const QString &word) {
    for (const QLatin1StringView key : {"id:"_L1, "p:"_L1, "pri:"_L1}) {
        if (word.startsWith(key) && word.size() > key.size()) {
            return true;
        }
    }
    const bool due = word.startsWith(u"due:");
    return due && dueDate(word).has_value();
}

std::optional<QString> firstProject(const QString &text) {
    for (const QString &word : wordsOf(text)) {
        if (word.startsWith(u'+') && word.size() > 1) {
            return word.sliced(1);
        }
    }
    return std::nullopt;
}

qsizetype sectionOf(QList<Section> &sections, const std::optional<QString> &project) {
    if (!project) {
        return 0;
    }
    for (qsizetype i = 1; i < sections.size(); ++i) {
        if (sections.at(i).title == *project) {
            return i;
        }
    }
    sections.append({.title = *project});
    return sections.size() - 1;
}

// Whether following parents from `todo` comes back to it; rule 6.
bool onCircle(const QList<qsizetype> &parents, qsizetype todo) {
    qsizetype at = parents.at(todo);
    for (qsizetype steps = 0; at >= 0 && steps < parents.size(); ++steps) {
        if (at == todo) {
            return true;
        }
        at = parents.at(at);
    }
    return false;
}

// Links each todo to the first todo whose id its p: tag names.
void linkSubtasks(QList<Todo> &todos, const QStringList &parentIds) {
    QHash<QString, qsizetype> byId;
    for (qsizetype i = 0; i < todos.size(); ++i) {
        if (!todos.at(i).id.isEmpty() && !byId.contains(todos.at(i).id)) {
            byId.insert(todos.at(i).id, i);
        }
    }
    QList<qsizetype> parents(todos.size(), -1);
    for (qsizetype i = 0; i < todos.size(); ++i) {
        parents[i] = parentIds.at(i).isEmpty() ? -1 : byId.value(parentIds.at(i), -1);
    }
    for (qsizetype i = 0; i < todos.size(); ++i) {
        todos[i].parent = onCircle(parents, i) ? -1 : parents.at(i);
    }
}

} // namespace

TodoList findTodos(const Document &document) {
    TodoList list{.sections = {Section{}}, .todos = {}};
    QStringList parentIds;
    for (qsizetype i = 0; i < document.lines.size(); ++i) {
        const QString &line = document.lines.at(i);
        if (isBlank(line)) {
            continue;
        }
        const TodoLine parts = splitTodoLine(line);
        list.todos.append({
            .line = i,
            .parent = -1,
            .section = sectionOf(list.sections, firstProject(parts.text)),
            .done = parts.done,
            .text = parts.text,
            .title = withoutWords(parts.text, isHiddenTag),
            .id = tagValue(parts.text, QStringLiteral("id")).value_or(QString()),
            .due = dueDate(parts.text),
        });
        parentIds.append(tagValue(parts.text, QStringLiteral("p")).value_or(QString()));
    }
    linkSubtasks(list.todos, parentIds);
    return list;
}

} // namespace topdo
