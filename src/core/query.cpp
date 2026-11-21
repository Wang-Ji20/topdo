#include "query.h"

#include <algorithm>

namespace topdo {

QList<qsizetype> filterTodos(const TodoList &list, const QString &filter, bool showDone) {
    QList<qsizetype> shown;
    for (qsizetype i = 0; i < list.todos.size(); ++i) {
        const Todo &todo = list.todos.at(i);
        const bool visible = filter.isEmpty() ? (showDone || !todo.done)
                                              : todo.title.contains(filter, Qt::CaseInsensitive);
        if (visible) {
            shown.append(i);
        }
    }
    std::ranges::stable_sort(shown, {}, [&list](qsizetype i) { return list.todos.at(i).section; });
    return shown;
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

} // namespace topdo
