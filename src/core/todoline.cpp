#include "todoline.h"

#include <QRegularExpression>

namespace topdo {

namespace {

constexpr int TabWidth = 4;

} // namespace

int indentation(const QString &line) {
    int columns = 0;
    for (const QChar ch : line) {
        if (ch == u' ') {
            columns += 1;
        } else if (ch == u'\t') {
            columns += TabWidth;
        } else {
            break;
        }
    }
    return columns;
}

bool isBlank(const QString &line) { return line.trimmed().isEmpty(); }

bool isFence(const QString &line) {
    const QString trimmed = line.trimmed();
    return trimmed.startsWith(u"```") || trimmed.startsWith(u"~~~");
}

std::optional<QString> headingTitle(const QString &line) {
    static const QRegularExpression heading(QStringLiteral("^#{1,6} (.*)$"));
    const QRegularExpressionMatch match = heading.match(line);
    if (!match.hasMatch()) {
        return std::nullopt;
    }
    return match.captured(1).trimmed();
}

std::optional<TodoLine> matchTodoLine(const QString &line) {
    static const QRegularExpression todo(
        QStringLiteral(R"(^[ \t]*[-*+][ \t]+\[([ xX])\][ \t]+(.*)$)"));
    const QRegularExpressionMatch match = todo.match(line);
    if (!match.hasMatch()) {
        return std::nullopt;
    }
    return TodoLine{
        .indent = indentation(line),
        .markColumn = match.capturedStart(1),
        .textColumn = match.capturedStart(2),
        .done = match.captured(1) != u" ",
        .text = match.captured(2),
    };
}

} // namespace topdo
