#include "document.h"

#include <QStringDecoder>

namespace topdo {

Document newTodoFile() {
    return {
        .lines =
            {
                "<!--",
                "This is your topdo list, a plain Markdown file. Edit it by hand whenever you",
                "like: the topdo widget and the `topdo` command read and change this same file.",
                "topdo skips this comment; delete it once you know the rules.",
                "",
                "- [ ] An open todo: a dash, a space, then [ ] and the text",
                "- [x] A done todo: [x] in the brackets",
                "  - [ ] A subtask: a todo indented under another one",
                "- [ ] A todo with a due date due:2026-12-31",
                "",
                "## A heading starts a section",
                "",
                "Any other line is a note: topdo keeps it exactly as written.",
                "All the rules: https://github.com/Wang-Ji20/topdo/blob/main/docs/design/format.md",
                "-->",
                "",
            },
    };
}

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
