#pragma once

#include <QByteArray>
#include <QRandomGenerator>

namespace topdo::test {

// A random todo file for property tests: todos with every marker and checkbox variant, headings,
// code fences, notes and blank lines, mixed line endings, an optional byte order mark, a missing
// final newline, and some bytes that aren't valid UTF-8. The same generator state always gives
// the same file, so a failure can be replayed from its seed.
QByteArray randomTodoFile(QRandomGenerator &random);

} // namespace topdo::test
