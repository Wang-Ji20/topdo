#pragma once

// The line rules of docs/design/format.md, shared by todos.cpp and edits.cpp. Internal to core.

#include <QString>

#include <optional>

namespace topdo {

// A line of the form "- [ ] text", with the positions that edits need.
struct TodoLine {
    int indent = 0;           // in columns; a tab counts as four
    qsizetype markColumn = 0; // index of the character between the brackets
    qsizetype textColumn = 0; // index where the text starts
    bool done = false;
    QString text;
};

std::optional<TodoLine> matchTodoLine(const QString &line);

// The title of a "# Heading" line, or nothing if the line isn't one.
std::optional<QString> headingTitle(const QString &line);

bool isFence(const QString &line);

// Whether a line opens a comment ("<!--") that it doesn't also close ("-->").
bool opensComment(const QString &line);

bool closesComment(const QString &line);

bool isBlank(const QString &line);

// Indentation in columns; a tab counts as four spaces.
int indentation(const QString &line);

} // namespace topdo
