#include "document.h"

#include <QStringDecoder>

namespace topdo {

QString manualText() {
    return QStringLiteral(R"(# Your todo list

`todo.txt` in this folder is your todo list. It is a plain
[todo.txt](https://github.com/todotxt/todo.txt) file: edit it by hand
whenever you like. The topdo widget, the `topdo` command and any other
todo.txt app read and change the same file.

One line is one todo:

```
(A) Call the bank @phone due:2026-12-01
Buy milk
Plan trip +Holiday id:1
x 2026-11-20 Book flight +Holiday p:1
Book hotel +Holiday p:1
```

| You write | It means |
|---|---|
| `x 2026-11-20 ` at the start | done, on that date |
| `(A) ` at the start | priority A, the highest; up to `(Z) ` |
| `2026-11-18 ` at the start, after any priority | the day you wrote it |
| `+Holiday` | the project; topdo groups todos by their first one |
| `@phone` | a context: where or how you do it |
| `due:2026-12-01` | the due date |
| `id:1`, `p:1` | a todo with `p:1` is a subtask of the one with `id:1` |

Blank lines are kept but aren't todos. Done todos stay in the file until
you delete them.

All the rules: https://github.com/Wang-Ji20/topdo/blob/main/docs/design/format.md
)");
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
