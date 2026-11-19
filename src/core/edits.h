#pragma once

#include "document.h"

#include <QString>

#include <optional>

namespace topdo {

// Edits of a todo file; see docs/design/format.md, rules 6 to 8. Each takes a document and returns
// the edited copy. `todo` is an index into findTodos(document).todos, and `section` an index into
// findTodos(document).sections. Text is a single line, without "\n".

// Ticks or unticks a todo. Only the character between the brackets changes.
Document setDone(const Document &document, qsizetype todo, bool done);

// Replaces the text after the checkbox.
Document setText(const Document &document, qsizetype todo, const QString &text);

// Removes a todo with its subtasks and notes: the lines indented more than it below it.
Document removeTodo(const Document &document, qsizetype todo);

// Adds "- [ ] text" after the first run of todos of the section, or of the file if no section is
// given.
Document addTodo(const Document &document, const QString &text, std::optional<qsizetype> section);

} // namespace topdo
