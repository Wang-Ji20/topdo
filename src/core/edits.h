#pragma once

#include "document.h"

#include <QDate>
#include <QList>
#include <QString>

#include <optional>

namespace topdo {

// Edits of a todo file; see docs/design/format.md, rules 11 to 14. Each takes a document and
// returns the edited copy. `todo` is an index into findTodos(document).todos. Text is a single
// line, without "\n".

// Ticks or unticks a todo. Ticking writes "x <today> " in front and moves a priority to pri:.
Document setDone(const Document &document, qsizetype todo, bool done, QDate today);

// Replaces the text after the priority and the dates.
Document setText(const Document &document, qsizetype todo, const QString &text);

// Removes a todo with its subtasks, their subtasks, and so on.
Document removeTodo(const Document &document, qsizetype todo);

// The lines removeTodo() removes, and where they were, so that they can be put back.
struct RemovedLine {
    qsizetype line = 0;
    QString text;

    friend bool operator==(const RemovedLine &, const RemovedLine &) = default;
};
using Block = QList<RemovedLine>; // in file order

Block todoBlock(const Document &document, qsizetype todo);

// Puts removed lines back at their old lines, or at the end if the file is now shorter.
Document restoreBlock(const Document &document, const Block &block);

// Adds a todo after the last line that isn't blank, with " +project" appended unless the text has
// that project already.
Document
addTodo(const Document &document, const QString &text, const std::optional<QString> &project);

// Adds "text p:ID" after the last subtask of a todo, giving the todo an id: tag if it has none.
Document addSubtask(const Document &document, qsizetype todo, const QString &text);

} // namespace topdo
