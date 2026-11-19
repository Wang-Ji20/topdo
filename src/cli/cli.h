#pragma once

#include <QStringList>
#include <QTextStream>

namespace topdo {

// Runs the topdo command; see docs/design/cli.md. `arguments` starts with the program name, as in
// QCoreApplication::arguments(). Returns the exit status.
int runCli(const QStringList &arguments, QTextStream &out, QTextStream &err);

} // namespace topdo
