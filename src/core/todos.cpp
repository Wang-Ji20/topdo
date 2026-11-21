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
bool isTitleTag(const QString &word) {
    for (const QLatin1StringView key : {"id:"_L1, "p:"_L1, "pri:"_L1}) {
        if (word.startsWith(key) && word.size() > key.size()) {
            return true;
        }
    }
    const bool due = word.startsWith(u"due:");
    return due && dueDate(word).has_value();
}

// A +project or @context word: the marker and at least one character.
bool isMarked(const QString &word, QChar marker) { return word.size() > 1 && word.at(0) == marker; }

bool isHiddenWord(const QString &word) {
    return isTitleTag(word) || isMarked(word, u'+') || isMarked(word, u'@');
}

// The words with the marker, without it.
QStringList markedWords(const QString &text, QChar marker) {
    QStringList found;
    for (const QString &word : wordsOf(text)) {
        if (isMarked(word, marker)) {
            found.append(word.sliced(1));
        }
    }
    return found;
}

qsizetype sectionOf(QList<Section> &sections, const QStringList &projects) {
    if (projects.isEmpty()) {
        return 0;
    }
    for (qsizetype i = 1; i < sections.size(); ++i) {
        if (sections.at(i).title == projects.first()) {
            return i;
        }
    }
    sections.append({.title = projects.first()});
    return sections.size() - 1;
}

qsizetype topLevelOf(const QList<Todo> &todos, qsizetype todo) {
    while (todos.at(todo).parent >= 0) {
        todo = todos.at(todo).parent;
    }
    return todo;
}

// Rule 9: a top-level todo's group is its first project; a subtask's is its top-level todo's.
void groupTodos(TodoList &list) {
    for (Todo &todo : list.todos) {
        if (todo.parent < 0) {
            todo.section = sectionOf(list.sections, todo.projects);
        }
    }
    for (qsizetype i = 0; i < list.todos.size(); ++i) {
        const qsizetype top = topLevelOf(list.todos, i);
        list.todos[i].section = list.todos.at(top).section;
    }
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
            .section = 0,
            .done = parts.done,
            .text = parts.text,
            .title = withoutWords(parts.text, isHiddenWord),
            .projects = markedWords(parts.text, u'+'),
            .contexts = markedWords(parts.text, u'@'),
            .id = tagValue(parts.text, QStringLiteral("id")).value_or(QString()),
            .due = dueDate(parts.text),
        });
        parentIds.append(tagValue(parts.text, QStringLiteral("p")).value_or(QString()));
    }
    linkSubtasks(list.todos, parentIds);
    groupTodos(list);
    return list;
}

} // namespace topdo
