#pragma once

#include <QByteArray>
#include <QRandomGenerator>

namespace topdo::test {

// A random todo file for property tests: todo.txt lines with and without done marks, priorities,
// dates, projects and tags (valid and not), and blank lines, as UTF-8 with "\n" line endings and a
// final newline. The
// same generator state always gives the same file, so a failure can be replayed from its seed.
QByteArray randomTodoFile(QRandomGenerator &random);

} // namespace topdo::test
