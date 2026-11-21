#pragma once

#include <QByteArray>
#include <QStringList>

namespace topdo {

// A todo file as text, one entry per line, without line endings; see docs/design/format.md.
struct Document {
    QStringList lines;

    friend bool operator==(const Document &, const Document &) = default;
};

// What a new todo file starts as: a comment that topdo skips, which teaches the format by example,
// and a blank line after it. See docs/design/format.md, "Where the file lives".
Document newTodoFile();

// Reads a UTF-8 todo file. Accepts "\n" and "\r\n" line endings.
Document parse(const QByteArray &bytes);

// Writes the document as UTF-8 with "\n" line endings and a final newline.
// render(parse(bytes)) == bytes for every file already in that form.
QByteArray render(const Document &document);

} // namespace topdo
