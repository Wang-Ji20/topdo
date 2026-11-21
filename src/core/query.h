#pragma once

#include "todos.h"

#include <QDate>
#include <QList>
#include <QString>

#include <optional>

namespace topdo {

// What the panel and the badge show of a todo list; see docs/design/widget.md.

// The todos the panel lists, as indexes into list.todos: by section, then in file order. With a
// filter, the todos whose title contains it (ignoring case), done or not; without one, the open
// todos, and the done ones too if showDone is set.
QList<qsizetype> filterTodos(const TodoList &list, const QString &filter, bool showDone);

// The todo the badge shows: the open todo with the earliest due date on or before today (late
// ones first), otherwise the first open todo. Nothing if no todo is open.
std::optional<qsizetype> nextTodo(const TodoList &list, QDate today);

// How deep a todo is nested: 0 for a top-level todo, 1 for its subtasks, and so on.
int depthOf(const TodoList &list, qsizetype todo);

// The open todos due on `day`, in file order: what the today column lists under the events.
QList<qsizetype> dueOn(const TodoList &list, QDate day);

// The top-level todos of a section, in file order: the milestones of a project. See
// docs/design/project-today.md.
QList<qsizetype> milestonesOf(const TodoList &list, qsizetype section);

// The direct subtasks of a todo, in file order: the steps of a milestone.
QList<qsizetype> subtasksOf(const TodoList &list, qsizetype todo);

} // namespace topdo
