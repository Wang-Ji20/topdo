#include "query.h"

#include <algorithm>

namespace topdo {

QList<qsizetype> milestonesOf(const TodoList &list, qsizetype section) {
    QList<qsizetype> milestones;
    for (qsizetype i = 0; i < list.todos.size(); ++i) {
        const Todo &todo = list.todos.at(i);
        if (todo.section == section && todo.parent < 0) {
            milestones.append(i);
        }
    }
    return milestones;
}

QList<qsizetype> subtasksOf(const TodoList &list, qsizetype todo) {
    QList<qsizetype> subtasks;
    for (qsizetype i = 0; i < list.todos.size(); ++i) {
        if (list.todos.at(i).parent == todo) {
            subtasks.append(i);
        }
    }
    return subtasks;
}

namespace {

bool matches(const Todo &todo, const QString &filter) {
    if (todo.title.contains(filter, Qt::CaseInsensitive)) {
        return true;
    }
    const auto contains = [&filter](const QString &word) {
        return word.contains(filter, Qt::CaseInsensitive);
    };
    return std::ranges::any_of(todo.projects, contains) ||
           std::ranges::any_of(todo.contexts, contains);
}

bool isShown(const Todo &todo, const QString &filter, bool showDone) {
    return filter.isEmpty() ? (showDone || !todo.done) : matches(todo, filter);
}

// Appends a todo and then its subtasks, depth first, keeping those that are shown.
void appendTree(const TodoList &list,
                qsizetype todo,
                const QString &filter,
                bool showDone,
                QList<qsizetype> &shown) {
    if (isShown(list.todos.at(todo), filter, showDone)) {
        shown.append(todo);
    }
    for (const qsizetype subtask : subtasksOf(list, todo)) {
        appendTree(list, subtask, filter, showDone, shown);
    }
}

} // namespace

QList<qsizetype> filterTodos(const TodoList &list, const QString &filter, bool showDone) {
    QList<qsizetype> shown;
    for (qsizetype section = 0; section < list.sections.size(); ++section) {
        for (const qsizetype top : milestonesOf(list, section)) {
            appendTree(list, top, filter, showDone, shown);
        }
    }
    return shown;
}

QStringList shownTags(const TodoList &list, qsizetype todo) {
    const Todo &item = list.todos.at(todo);
    const QString group = list.sections.at(item.section).title;
    QStringList tags;
    for (const QString &context : item.contexts) {
        tags.append(u'@' + context);
    }
    for (const QString &project : item.projects) {
        if (project != group) {
            tags.append(u'+' + project);
        }
    }
    return tags;
}

std::optional<qsizetype> nextTodo(const TodoList &list, QDate today) {
    std::optional<qsizetype> earliestDue;
    std::optional<qsizetype> firstOpen;
    for (qsizetype i = 0; i < list.todos.size(); ++i) {
        const Todo &todo = list.todos.at(i);
        if (todo.done) {
            continue;
        }
        if (!firstOpen) {
            firstOpen = i;
        }
        const bool dueByToday = todo.due && *todo.due <= today;
        const bool earlier = !earliestDue || todo.due < list.todos.at(*earliestDue).due;
        if (dueByToday && earlier) {
            earliestDue = i;
        }
    }
    return earliestDue ? earliestDue : firstOpen;
}

int depthOf(const TodoList &list, qsizetype todo) {
    int depth = 0;
    for (qsizetype parent = list.todos.at(todo).parent; parent >= 0;
         parent = list.todos.at(parent).parent) {
        ++depth;
    }
    return depth;
}

QList<qsizetype> dueOn(const TodoList &list, QDate day) {
    QList<qsizetype> due;
    for (qsizetype i = 0; i < list.todos.size(); ++i) {
        const Todo &todo = list.todos.at(i);
        if (!todo.done && todo.due == day) {
            due.append(i);
        }
    }
    return due;
}

} // namespace topdo
