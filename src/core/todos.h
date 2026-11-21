#pragma once

#include "document.h"

#include <QDate>
#include <QList>
#include <QString>

#include <optional>

namespace topdo {

// The part of a todo file under one heading. Section 0 is the unnamed one before the first
// heading; it always exists, even when it holds no todos.
struct Section {
    QString title;
    qsizetype line = -1; // index of the heading in Document::lines; -1 for the unnamed section

    friend bool operator==(const Section &, const Section &) = default;
};

// A todo line; see docs/design/format.md, rules 1 to 6.
struct Todo {
    qsizetype line = 0;    // index in Document::lines
    qsizetype parent = -1; // index in TodoList::todos; -1 for a top-level todo
    qsizetype section = 0; // index in TodoList::sections
    bool done = false;
    QString text;  // everything after the checkbox, as written
    QString title; // the text without its due date, for display
    std::optional<QDate> due;

    friend bool operator==(const Todo &, const Todo &) = default;
};

struct TodoList {
    QList<Section> sections;
    QList<Todo> todos;

    friend bool operator==(const TodoList &, const TodoList &) = default;
};

// Finds the todos, subtasks and sections of a document, line by line.
TodoList findTodos(const Document &document);

} // namespace topdo
