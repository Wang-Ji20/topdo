#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace topdo {

// A todo file as text, one entry per line, without line endings; see docs/design/format.md.
struct Document {
    QStringList lines;

    friend bool operator==(const Document &, const Document &) = default;
};

// The manual topdo writes as README.md into a new todo folder: the rules of
// docs/design/format.md, short and by example.
QString manualText();

// Reads a UTF-8 todo file. Accepts "\n" and "\r\n" line endings.
Document parse(const QByteArray &bytes);

// Writes the document as UTF-8 with "\n" line endings and a final newline.
// render(parse(bytes)) == bytes for every file already in that form.
QByteArray render(const Document &document);

} // namespace topdo
