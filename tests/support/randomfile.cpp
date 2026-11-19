#include "randomfile.h"

#include <QByteArrayList>

namespace topdo::test {

namespace {

QByteArray pick(QRandomGenerator &random, const QByteArrayList &choices) {
    const auto index = random.bounded(static_cast<int>(choices.size()));
    return choices.at(index);
}

bool chance(QRandomGenerator &random, int oneIn) { return random.bounded(oneIn) == 0; }

QByteArray randomText(QRandomGenerator &random) {
    static const QByteArrayList words = {
        "Write",
        "report",
        "call",
        "the",
        "bank",
        "Ünïcödé",
        "中文",
        "due:2026-11-21",
        "due:2026-13-40",
        "[ ]",
        "`code`",
        "*stars*",
        "\t",
        "end\rof",
        "\xff\xfe",
        "\xc3",
    };
    const int count = random.bounded(6);
    QByteArrayList chosen;
    for (int i = 0; i < count; ++i) {
        chosen.append(pick(random, words));
    }
    return chosen.join(' ');
}

QByteArray randomTodo(QRandomGenerator &random) {
    const QByteArray indent = pick(random, {"", "", "  ", "    ", "\t", "   "});
    const QByteArray marker = pick(random, {"-", "-", "*", "+", "1.", "2)"});
    const QByteArray checkbox = pick(random, {"[ ]", "[ ]", "[x]", "[X]", "[]", "[y]", "[  ]"});
    const QByteArray separator = pick(random, {" ", " ", "", "  ", "\t"});
    const QByteArray text = randomText(random);
    return indent + marker + separator + checkbox + " " + text;
}

QByteArray randomHeading(QRandomGenerator &random) {
    const QByteArray hashes(random.bounded(1, 8), '#');
    const QByteArray text = randomText(random);
    return hashes + pick(random, {" ", "", "  "}) + text;
}

QByteArray randomLine(QRandomGenerator &random) {
    switch (random.bounded(6)) {
    case 0:
    case 1:
        return randomTodo(random);
    case 2:
        return randomHeading(random);
    case 3:
        return pick(random, {"```", "~~~", "```md", "    code"});
    case 4:
        return pick(random, {"", "", "   "});
    default:
        return randomText(random);
    }
}

} // namespace

QByteArray randomTodoFile(QRandomGenerator &random) {
    QByteArray file = chance(random, 8) ? QByteArray("\xEF\xBB\xBF") : QByteArray();
    const int lineCount = random.bounded(30);
    for (int i = 0; i < lineCount; ++i) {
        file.append(randomLine(random));
        file.append(pick(random, {"\n", "\n", "\n", "\r\n"}));
    }
    if (chance(random, 4)) {
        file.append(randomLine(random)); // a last line without a line ending
    }
    return file;
}

} // namespace topdo::test
