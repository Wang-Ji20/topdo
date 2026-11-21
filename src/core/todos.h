#pragma once

#include "document.h"

#include <QDate>
#include <QList>
#include <QString>

#include <optional>

namespace topdo {

// A group of the panel: the todos whose first project is `title`. Section 0 holds the todos
// without a project; its title is empty, and it always exists, even when it holds no todos.
struct Section {
    QString title; // the project without its "+"

    friend bool operator==(const Section &, const Section &) = default;
};

// A todo line; see docs/design/format.md, rules 1 to 10.
struct Todo {
    qsizetype line = 0;    // index in Document::lines
    qsizetype parent = -1; // index in TodoList::todos; -1 for a top-level todo
    qsizetype section = 0; // index in TodoList::sections
    bool done = false;
    QString text;  // everything after the priority and the dates, as written
    QString title; // the text without the due:, id:, p: and pri: tags, for display
    QString id;    // the value of its id: tag, or empty
    std::optional<QDate> due;

    friend bool operator==(const Todo &, const Todo &) = default;
};

struct TodoList {
    QList<Section> sections;
    QList<Todo> todos;

    friend bool operator==(const TodoList &, const TodoList &) = default;
};

// Finds the todos of a document, their subtasks and their projects.
TodoList findTodos(const Document &document);

} // namespace topdo
