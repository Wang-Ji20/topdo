#pragma once

#include <QByteArray>
#include <QRandomGenerator>

namespace topdo::test {

// A random todo file for property tests: todos with every marker and checkbox variant, headings,
// code fences, notes and blank lines, as UTF-8 with "\n" line endings and a final newline. The
// same generator state always gives the same file, so a failure can be replayed from its seed.
QByteArray randomTodoFile(QRandomGenerator &random);

} // namespace topdo::test
