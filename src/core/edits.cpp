#include "edits.h"
#include "todoline.h"
#include "todos.h"

#include <algorithm>

namespace topdo {

namespace {

qsizetype lineOf(const Document &document, qsizetype todo) {
    const TodoList list = findTodos(document);
    return list.todos.at(todo).line;
}

struct LocatedTodo {
    qsizetype line = 0;
    TodoLine match;
};

LocatedTodo locate(const Document &document, qsizetype todo) {
    const qsizetype line = lineOf(document, todo);
    const std::optional<TodoLine> match = matchTodoLine(document.lines.at(line));
    Q_ASSERT(match); // findTodos() found a todo on this line
    return {.line = line, .match = *match};
}

// The last line of a todo's block: the todo and the lines indented more than it below it.
// Blank lines inside the block belong to it; blank lines after it don't.
qsizetype blockEnd(const QStringList &lines, qsizetype first) {
    const int indent = indentation(lines.at(first));
    qsizetype last = first;
    for (qsizetype i = first + 1; i < lines.size(); ++i) {
        const QString &line = lines.at(i);
        if (isBlank(line)) {
            continue;
        }
        if (indentation(line) <= indent) {
            break;
        }
        last = i;
    }
    return last;
}

// The last line of the run of todos that starts at `first`: todo blocks that follow each other
// directly, with no blank or other line between them.
qsizetype runEnd(const QStringList &lines, qsizetype first) {
    qsizetype last = blockEnd(lines, first);
    while (last + 1 < lines.size() && matchTodoLine(lines.at(last + 1))) {
        last = blockEnd(lines, last + 1);
    }
    return last;
}

qsizetype insertionLine(const Document &document, std::optional<qsizetype> section) {
    const TodoList list = findTodos(document);
    const auto inScope = [&](const Todo &todo) { return !section || todo.section == *section; };
    const auto first = std::ranges::find_if(list.todos, inScope);
    if (first != list.todos.end()) {
        return runEnd(document.lines, first->line) + 1;
    }
    if (section) {
        return list.sections.at(*section).line + 1; // the unnamed section's line is -1
    }
    return document.lines.size();
}

// The spaces and tabs at the start of a line, as written.
QString leadingWhitespace(const QString &line) {
    qsizetype end = 0;
    while (end < line.size() && (line.at(end) == u' ' || line.at(end) == u'\t')) {
        ++end;
    }
    return line.left(end);
}

// The indentation for a new subtask of `todo`: its last subtask's, or two spaces more than its own.
QString subtaskIndent(const QStringList &lines, const TodoList &list, qsizetype todo) {
    const auto isChild = [todo](const Todo &other) { return other.parent == todo; };
    const auto lastChild = std::ranges::find_last_if(list.todos, isChild).begin();
    if (lastChild == list.todos.end()) {
        return leadingWhitespace(lines.at(list.todos.at(todo).line)) + QStringLiteral("  ");
    }
    return leadingWhitespace(lines.at(lastChild->line));
}

} // namespace

Document setDone(const Document &document, qsizetype todo, bool done) {
    const auto [line, match] = locate(document, todo);
    if (match.done == done) {
        return document; // an [X] stays as written
    }
    Document edited = document;
    edited.lines[line][match.markColumn] = done ? u'x' : u' ';
    return edited;
}

Document setText(const Document &document, qsizetype todo, const QString &text) {
    const auto [line, match] = locate(document, todo);
    Document edited = document;
    edited.lines[line] = document.lines.at(line).left(match.textColumn) + text;
    return edited;
}

Block todoBlock(const Document &document, qsizetype todo) {
    const qsizetype first = lineOf(document, todo);
    const qsizetype last = blockEnd(document.lines, first);
    return {.line = first, .lines = document.lines.mid(first, last - first + 1)};
}

Document removeTodo(const Document &document, qsizetype todo) {
    const Block block = todoBlock(document, todo);
    Document edited = document;
    edited.lines.remove(block.line, block.lines.size());
    return edited;
}

Document restoreBlock(const Document &document, const Block &block) {
    const qsizetype line = std::min(block.line, document.lines.size());
    Document edited = document;
    for (qsizetype i = 0; i < block.lines.size(); ++i) {
        edited.lines.insert(line + i, block.lines.at(i));
    }
    return edited;
}

Document addTodo(const Document &document, const QString &text, std::optional<qsizetype> section) {
    const qsizetype line = insertionLine(document, section);
    Document edited = document;
    edited.lines.insert(line, QStringLiteral("- [ ] ") + text);
    return edited;
}

Document addSubtask(const Document &document, qsizetype todo, const QString &text) {
    const TodoList list = findTodos(document);
    const qsizetype line = blockEnd(document.lines, list.todos.at(todo).line) + 1;
    const QString indent = subtaskIndent(document.lines, list, todo);
    Document edited = document;
    edited.lines.insert(line, indent + QStringLiteral("- [ ] ") + text);
    return edited;
}

} // namespace topdo
