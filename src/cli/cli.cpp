#include "cli.h"
#include "edits.h"
#include "store.h"
#include "todos.h"

#include <QCommandLineParser>
#include <QHash>
#include <QSet>

#include <algorithm>
#include <functional>

namespace topdo {

namespace {

constexpr int Failure = 1;
constexpr int UsageError = 2;

// What every command gets: the todo file, the arguments after the command name, and the options.
struct Request {
    QString path;
    QStringList arguments;
    bool all = false;
    std::optional<QString> section;
    std::optional<QString> under; // add: the number of the todo to add a subtask to
};

// A change to the todo file that can fail, for example on a number that isn't a todo.
using Edit = std::function<std::expected<Document, QString>(const Document &)>;

int fail(QTextStream &err, const QString &message) {
    err << "topdo: " << message << '\n';
    return Failure;
}

QString numberColumn(qsizetype number, int width) {
    return QString::number(number).rightJustified(width) + u"  ";
}

int listCommand(const Request &request, QTextStream &out, QTextStream &err) {
    const std::expected<Document, QString> document = load(request.path);
    if (!document) {
        return fail(err, document.error());
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

// Loads the file, applies the edit, and saves once, only if the edit succeeded and changed it.
int editCommand(const Request &request, const Edit &edit, QTextStream &err) {
    const std::expected<Document, QString> document = load(request.path);
    if (!document) {
        return fail(err, document.error());
    }
    const std::expected<Document, QString> edited = edit(*document);
    if (!edited) {
        return fail(err, edited.error());
    }
    if (*edited == *document) {
        return 0;
    }
    const std::expected<void, QString> saved = save(request.path, *edited);
    if (!saved) {
        return fail(err, saved.error());
    }
    return 0;
}

// The todo indexes that numbers (counting from 1) name, highest first and without repeats.
std::expected<QList<qsizetype>, QString> todoIndexes(const QStringList &numbers, qsizetype count) {
    QList<qsizetype> indexes;
    for (const QString &number : numbers) {
        bool ok = false;
        const qsizetype value = number.toLongLong(&ok);
        if (!ok || value < 1 || value > count) {
            return std::unexpected(QStringLiteral("no todo %1").arg(number));
        }
        indexes.append(value - 1);
    }
    std::ranges::sort(indexes, std::greater{});
    const auto repeats = std::ranges::unique(indexes);
    indexes.erase(repeats.begin(), repeats.end());
    return indexes;
}

// Applies `edit` to each numbered todo, highest first, so that removing a todo doesn't shift the
// numbers of the ones still to come.
Edit eachTodo(const QStringList &numbers,
              const std::function<Document(const Document &, qsizetype)> &edit) {
    return [=](const Document &document) -> std::expected<Document, QString> {
        const TodoList list = findTodos(document);
        const auto indexes = todoIndexes(numbers, list.todos.size());
        if (!indexes) {
            return std::unexpected(indexes.error());
        }
        Document edited = document;
        for (const qsizetype index : *indexes) {
            edited = edit(edited, index);
        }
        return edited;
    };
}

Edit addToSection(const QString &text, const std::optional<QString> &section) {
    return [=](const Document &document) -> std::expected<Document, QString> {
        if (!section) {
            return addTodo(document, text, std::nullopt);
        }
        const TodoList list = findTodos(document);
        const auto found = std::ranges::find(list.sections, *section, &Section::title);
        if (found == list.sections.end()) {
            return std::unexpected(QStringLiteral("no section \"%1\"").arg(*section));
        }
        return addTodo(document, text, found - list.sections.begin());
    };
}

Edit addUnder(const QString &text, const QString &number) {
    const auto addSubtaskTo = [text](const Document &document, qsizetype todo) {
        return addSubtask(document, todo, text);
    };
    return eachTodo({number}, addSubtaskTo);
}

// The words of a todo's text, joined with spaces. Line breaks become spaces too, so that one todo
// stays one line.
QString joinText(const QStringList &words) { return words.join(u' ').simplified(); }

int usageError(const QCommandLineParser &parser, const QString &message, QTextStream &err) {
    err << "topdo: " << message << "\n\n" << parser.helpText();
    return UsageError;
}

int runCommand(const QString &command,
               const Request &request,
               const QCommandLineParser &parser,
               QTextStream &out,
               QTextStream &err) {
    const QStringList &arguments = request.arguments;
    if (command == u"list") {
        return listCommand(request, out, err);
    }
    if (command == u"path") {
        return pathCommand(request, out);
    }
    if (command == u"add") {
        const QString text = joinText(arguments);
        if (text.isEmpty()) {
            return usageError(parser, QStringLiteral("add needs the todo's text"), err);
        }
        if (request.under && request.section) {
            return usageError(
                parser, QStringLiteral("add takes --section or --under, not both"), err);
        }
        const Edit edit =
            request.under ? addUnder(text, *request.under) : addToSection(text, request.section);
        return editCommand(request, edit, err);
    }
    const bool numbered = command == u"done" || command == u"undone" || command == u"rm";
    if (numbered && arguments.isEmpty()) {
        return usageError(parser, QStringLiteral("%1 needs todo numbers").arg(command), err);
    }
    if (command == u"done") {
        return editCommand(request,
                           eachTodo(arguments,
                                    [](const Document &document, qsizetype todo) {
                                        return setDone(document, todo, true);
                                    }),
                           err);
    }
    if (command == u"undone") {
        return editCommand(request,
                           eachTodo(arguments,
                                    [](const Document &document, qsizetype todo) {
                                        return setDone(document, todo, false);
                                    }),
                           err);
    }
    if (command == u"rm") {
        return editCommand(request, eachTodo(arguments, removeTodo), err);
    }
    if (command == u"edit") {
        const QString text = joinText(arguments.mid(1));
        if (text.isEmpty()) {
            return usageError(parser, QStringLiteral("edit needs a todo number and text"), err);
        }
        const auto setTextTo = [text](const Document &document, qsizetype todo) {
            return setText(document, todo, text);
        };
        return editCommand(request, eachTodo({arguments.first()}, setTextTo), err);
    }
    return usageError(parser, QStringLiteral("unknown command \"%1\"").arg(command), err);
}

} // namespace

int runCli(const QStringList &arguments, QTextStream &out, QTextStream &err) {
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("A todo list in a Markdown file.\n\n"
                       "Commands:\n"
                       "  list                 the open todos, numbered\n"
                       "  add <text>...        add a todo\n"
                       "  done <n>...          tick todos\n"
                       "  undone <n>...        untick todos\n"
                       "  edit <n> <text>...   replace a todo's text\n"
                       "  rm <n>...            remove todos with their subtasks\n"
                       "  path                 the path of the todo file"));
    parser.addPositionalArgument(QStringLiteral("command"), QStringLiteral("What to do."));
    parser.addOption(
        {QStringLiteral("dir"), QStringLiteral("Use this todo folder."), QStringLiteral("folder")});
    parser.addOption({QStringLiteral("all"), QStringLiteral("list: also show done todos.")});
    parser.addOption({QStringLiteral("section"),
                      QStringLiteral("add: add to this section."),
                      QStringLiteral("heading")});
    parser.addOption({QStringLiteral("under"),
                      QStringLiteral("add: add as a subtask of this todo."),
                      QStringLiteral("n")});
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
        .section = parser.isSet("section") ? std::optional(parser.value("section")) : std::nullopt,
        .under = parser.isSet("under") ? std::optional(parser.value("under")) : std::nullopt,
    };
    return runCommand(command, request, parser, out, err);
}

} // namespace topdo
