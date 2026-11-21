#pragma once

#include "document.h"

#include <QString>
#include <QStringList>

#include <optional>

namespace topdo {

// Edits of a todo file; see docs/design/format.md, rules 7 to 9. Each takes a document and returns
// the edited copy. `todo` is an index into findTodos(document).todos, and `section` an index into
// findTodos(document).sections. Text is a single line, without "\n".

// Ticks or unticks a todo. Only the character between the brackets changes.
Document setDone(const Document &document, qsizetype todo, bool done);

// Replaces the text after the checkbox.
Document setText(const Document &document, qsizetype todo, const QString &text);

// Removes a todo with its subtasks and notes: the lines indented more than it below it.
Document removeTodo(const Document &document, qsizetype todo);

// The lines removeTodo() removes, and where they were, so that they can be put back.
struct Block {
    qsizetype line = 0;
    QStringList lines;

    friend bool operator==(const Block &, const Block &) = default;
};

Block todoBlock(const Document &document, qsizetype todo);

// Puts removed lines back at their old line, or at the end if the file is now shorter.
Document restoreBlock(const Document &document, const Block &block);

// Adds "- [ ] text" after the first run of todos of the section, or of the file if no section is
// given.
Document addTodo(const Document &document, const QString &text, std::optional<qsizetype> section);

// Adds "- [ ] text" as the last subtask of a todo: after its block, indented like its last
// subtask, or two spaces more than the todo if it has none.
Document addSubtask(const Document &document, qsizetype todo, const QString &text);

} // namespace topdo
