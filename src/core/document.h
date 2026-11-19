#pragma once

#include <QByteArray>
#include <QList>

namespace topdo {

// One line of a todo file. The line ending is kept apart from the text, so that rendering gives
// back exactly the bytes that were read, whatever mix of endings the file has.
struct Line {
    QByteArray text;   // without the line ending
    QByteArray ending; // "\n", "\r\n", or empty for a last line that has none

    friend bool operator==(const Line &, const Line &) = default;
};

// A todo file as read from disk; see docs/design/format.md. It holds bytes, not decoded text, so
// that a file that isn't valid UTF-8 still survives a round trip unchanged.
struct Document {
    bool hasByteOrderMark = false;
    QList<Line> lines;

    friend bool operator==(const Document &, const Document &) = default;
};

// Splits a todo file into lines. render(parse(bytes)) == bytes for every input.
Document parse(const QByteArray &bytes);

// Joins the lines back into the bytes of a todo file.
QByteArray render(const Document &document);

} // namespace topdo
