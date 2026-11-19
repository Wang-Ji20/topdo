#include "cli.h"
#include "store.h"
#include "todos.h"

#include <QCommandLineParser>
#include <QHash>
#include <QSet>

namespace topdo {

namespace {

constexpr int Failure = 1;
constexpr int UsageError = 2;

// What every command gets: the todo file and the arguments after the command name.
struct Request {
    QString path;
    QStringList arguments;
    bool all = false;
};

QString numberColumn(qsizetype number, int width) {
    return QString::number(number).rightJustified(width) + u"  ";
}

int listCommand(const Request &request, QTextStream &out, QTextStream &err) {
    const std::expected<Document, QString> document = load(request.path);
    if (!document) {
        err << "topdo: " << document.error() << '\n';
        return Failure;
    }
    const TodoList list = findTodos(*document);
    const int width = static_cast<int>(QString::number(list.todos.size()).size());
    QHash<qsizetype, qsizetype> todoAtLine;
    for (qsizetype i = 0; i < list.todos.size(); ++i) {
        todoAtLine.insert(list.todos.at(i).line, i);
    }
    QSet<qsizetype> headingLines;
    for (const Section &section : list.sections) {
        headingLines.insert(section.line);
    }
    for (qsizetype line = 0; line < document->lines.size(); ++line) {
        const QString &text = document->lines.at(line);
        if (headingLines.contains(line)) {
            out << text << '\n';
        }
        const auto todo = todoAtLine.constFind(line);
        if (todo != todoAtLine.constEnd() && (request.all || !list.todos.at(*todo).done)) {
            out << numberColumn(*todo + 1, width) << text << '\n';
        }
    }
    return 0;
}

int pathCommand(const Request &request, QTextStream &out) {
    out << request.path << '\n';
    return 0;
}

int usageError(const QCommandLineParser &parser, const QString &message, QTextStream &err) {
    err << "topdo: " << message << "\n\n" << parser.helpText();
    return UsageError;
}

} // namespace

int runCli(const QStringList &arguments, QTextStream &out, QTextStream &err) {
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("A todo list in a Markdown file. Commands: list, path."));
    parser.addPositionalArgument(QStringLiteral("command"), QStringLiteral("What to do."));
    parser.addOption(
        {QStringLiteral("dir"), QStringLiteral("Use this todo folder."), QStringLiteral("folder")});
    parser.addOption({QStringLiteral("all"), QStringLiteral("list: also show done todos.")});
    if (!parser.parse(arguments)) {
        return usageError(parser, parser.errorText(), err);
    }
    QStringList positional = parser.positionalArguments();
    if (positional.isEmpty()) {
        return usageError(parser, QStringLiteral("no command given"), err);
    }
    const QString command = positional.takeFirst();
    const QString dir =
        parser.isSet("dir") ? parser.value("dir") : configuredDir(defaultConfigFile());
    const Request request{
        .path = todoFilePath(dir),
        .arguments = positional,
        .all = parser.isSet("all"),
    };
    if (command == u"list") {
        return listCommand(request, out, err);
    }
    if (command == u"path") {
        return pathCommand(request, out);
    }
    return usageError(parser, QStringLiteral("unknown command \"%1\"").arg(command), err);
}

} // namespace topdo
