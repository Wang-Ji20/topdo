#include "document.h"

#include <QStringDecoder>

namespace topdo {

Document parse(const QByteArray &bytes) {
    QStringDecoder decoder(QStringDecoder::Utf8); // also drops a byte order mark
    const QString text = decoder.decode(bytes);
    QStringList lines = text.split(u'\n');
    if (lines.last().isEmpty()) {
        lines.removeLast(); // the final newline ends the last line; it doesn't start a new one
    }
    for (QString &line : lines) {
        if (line.endsWith(u'\r')) {
            line.chop(1);
        }
    }
    return {.lines = lines};
}

QByteArray render(const Document &document) {
    if (document.lines.isEmpty()) {
        return {};
    }
    const QString text = document.lines.join(u'\n') + u'\n';
    return text.toUtf8();
}

} // namespace topdo
