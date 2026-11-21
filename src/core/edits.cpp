#include "edits.h"
#include "todoline.h"
#include "todos.h"

#include <QSet>

#include <algorithm>
#include <ranges>

namespace topdo {

namespace {

qsizetype lineOf(const Document &document, qsizetype todo) {
    const TodoList list = findTodos(document);
    return list.todos.at(todo).line;
}

// A "pri:A" word with a valid priority letter.
bool isPriorityTag(const QString &word) {
    return word.size() == 5 && word.startsWith(u"pri:") && word.at(4) >= u'A' && word.at(4) <= u'Z';
}

TodoLine ticked(TodoLine todo, QDate today) {
    todo.done = true;
    todo.completed = today.toString(Qt::ISODate);
    if (todo.priority) {
        todo.text += QStringLiteral(" pri:") + *todo.priority;
        todo.priority.reset();
    }
    return todo;
}

TodoLine unticked(TodoLine todo) {
    todo.done = false;
    todo.completed.reset();
    const QStringList words = wordsOf(todo.text);
    const auto tag = std::ranges::find_if(words, isPriorityTag);
    if (tag != words.end()) {
        todo.priority = tag->at(4);
        todo.text = withoutWords(todo.text, [&tag](const QString &word) { return word == *tag; });
    }
    return todo;
}

// A todo and its subtasks, their subtasks, and so on, as indexes into list.todos in file order.
QList<qsizetype> subtree(const TodoList &list, qsizetype todo) {
    QList<qsizetype> members{todo};
    for (qsizetype i = 0; i < list.todos.size(); ++i) {
        for (qsizetype up = list.todos.at(i).parent; up >= 0; up = list.todos.at(up).parent) {
            if (up == todo) {
                members.append(i);
                break;
            }
        }
    }
    std::ranges::sort(members, {}, [&list](qsizetype i) { return list.todos.at(i).line; });
    return members;
}

qsizetype afterLastText(const QStringList &lines) {
    for (qsizetype i = lines.size(); i > 0; --i) {
        if (!isBlank(lines.at(i - 1))) {
            return i;
        }
    }
    return 0;
}

// The smallest whole number from 1 that no todo uses as its id.
QString freeId(const TodoList &list) {
    QSet<QString> used;
    for (const Todo &todo : list.todos) {
        used.insert(todo.id);
    }
    int id = 1;
    while (used.contains(QString::number(id))) {
        ++id;
    }
    return QString::number(id);
}

} // namespace

Document setDone(const Document &document, qsizetype todo, bool done, QDate today) {
    const qsizetype line = lineOf(document, todo);
    const TodoLine parts = splitTodoLine(document.lines.at(line));
    if (parts.done == done) {
        return document;
    }
    const QString changed = joinTodoLine(done ? ticked(parts, today) : unticked(parts));
    // Unticking "x " with no text gives a blank line, and "x x y" gives "x y", done again: an open
    // todo can't be written that way, so it stays done.
    if (isBlank(changed) || splitTodoLine(changed).done != done) {
        return document;
    }
    Document edited = document;
    edited.lines[line] = changed;
    return edited;
}

Document setText(const Document &document, qsizetype todo, const QString &text) {
    const qsizetype line = lineOf(document, todo);
    TodoLine parts = splitTodoLine(document.lines.at(line));
    parts.text = text;
    Document edited = document;
    edited.lines[line] = joinTodoLine(parts);
    return edited;
}

Block todoBlock(const Document &document, qsizetype todo) {
    const TodoList list = findTodos(document);
    Block block;
    for (const qsizetype member : subtree(list, todo)) {
        const qsizetype line = list.todos.at(member).line;
        block.append({.line = line, .text = document.lines.at(line)});
    }
    return block;
}

Document removeTodo(const Document &document, qsizetype todo) {
    const Block block = todoBlock(document, todo);
    Document edited = document;
    for (const RemovedLine &removed : block | std::views::reverse) {
        edited.lines.remove(removed.line);
    }
    return edited;
}

Document restoreBlock(const Document &document, const Block &block) {
    Document edited = document;
    for (const RemovedLine &removed : block) {
        const qsizetype line = std::min(removed.line, edited.lines.size());
        edited.lines.insert(line, removed.text);
    }
    return edited;
}

Document
addTodo(const Document &document, const QString &text, const std::optional<QString> &project) {
    QString line = text;
    if (project) {
        const QString word = u'+' + *project;
        if (!wordsOf(text).contains(word)) {
            line += u' ' + word;
        }
    }
    Document edited = document;
    edited.lines.insert(afterLastText(document.lines), line);
    return edited;
}

Document addSubtask(const Document &document, qsizetype todo, const QString &text) {
    const TodoList list = findTodos(document);
    const Todo &parent = list.todos.at(todo);
    const QString id = parent.id.isEmpty() ? freeId(list) : parent.id;
    const qsizetype last = list.todos.at(subtree(list, todo).constLast()).line;
    Document edited = document;
    if (parent.id.isEmpty()) {
        edited.lines[parent.line] += QStringLiteral(" id:") + id;
    }
    edited.lines.insert(last + 1, text + QStringLiteral(" p:") + id);
    return edited;
}

} // namespace topdo
