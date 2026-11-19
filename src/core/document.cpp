#include "document.h"

#include <QByteArrayView>

namespace topdo {

namespace {

constexpr QByteArrayView ByteOrderMark("\xEF\xBB\xBF");

// A chunk is one line including its "\n", if it has one. A lone "\r" is not a line ending here:
// it stays in the text, which keeps the round trip exact.
Line lineFromChunk(QByteArrayView chunk) {
    if (!chunk.endsWith('\n')) {
        return {.text = chunk.toByteArray(), .ending = {}};
    }
    const qsizetype endingSize = chunk.endsWith("\r\n") ? 2 : 1;
    const QByteArrayView text = chunk.chopped(endingSize);
    const QByteArrayView ending = chunk.last(endingSize);
    return {.text = text.toByteArray(), .ending = ending.toByteArray()};
}

QList<Line> splitLines(QByteArrayView bytes) {
    QList<Line> lines;
    qsizetype start = 0;
    while (start < bytes.size()) {
        const qsizetype newline = bytes.indexOf('\n', start);
        const qsizetype end = newline < 0 ? bytes.size() : newline + 1;
        const QByteArrayView chunk = bytes.sliced(start, end - start);
        lines.append(lineFromChunk(chunk));
        start = end;
    }
    return lines;
}

} // namespace

Document parse(const QByteArray &bytes) {
    const QByteArrayView view(bytes);
    const bool hasByteOrderMark = view.startsWith(ByteOrderMark);
    const QByteArrayView body = hasByteOrderMark ? view.sliced(ByteOrderMark.size()) : view;
    return {.hasByteOrderMark = hasByteOrderMark, .lines = splitLines(body)};
}

QByteArray render(const Document &document) {
    QByteArray bytes;
    if (document.hasByteOrderMark) {
        bytes.append(ByteOrderMark);
    }
    for (const Line &line : document.lines) {
        bytes.append(line.text);
        bytes.append(line.ending);
    }
    return bytes;
}

} // namespace topdo
