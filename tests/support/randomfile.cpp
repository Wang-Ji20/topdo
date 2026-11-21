#include "randomfile.h"

#include <QByteArrayList>

namespace topdo::test {

namespace {

QByteArray pick(QRandomGenerator &random, const QByteArrayList &choices) {
    const auto index = random.bounded(static_cast<int>(choices.size()));
    return choices.at(index);
}

QByteArray randomText(QRandomGenerator &random) {
    static const QByteArrayList words = {
        "Write",
        "report",
        "call",
        "+Work",
        "+Home",
        "@phone",
        "due:2026-11-21",
        "due:2026-13-40",
        "id:1",
        "id:2",
        "p:1",
        "p:2",
        "p:3",
        "pri:A",
        "pri:",
        "Ünïcödé",
        "中文",
        "x",
        "(A)",
        "2026-11-20",
        "a:b:c",
        "\t",
        "https://x.y",
    };
    const int count = random.bounded(6);
    QByteArrayList chosen;
    for (int i = 0; i < count; ++i) {
        chosen.append(pick(random, words));
    }
    return chosen.join(pick(random, {" ", " ", "  "}));
}

QByteArray randomTodo(QRandomGenerator &random) {
    const QByteArray done = pick(random, {"", "", "x ", "x ", "X ", "x"});
    const QByteArray priority = pick(random, {"", "", "(A) ", "(Z) ", "(b) ", "(A)"});
    const QByteArray completed = pick(random, {"", "2026-11-20 ", "2026-1-2 "});
    const QByteArray created = pick(random, {"", "", "2026-11-18 "});
    const QByteArray text = randomText(random);
    return done + priority + completed + created + text;
}

QByteArray randomLine(QRandomGenerator &random) {
    if (random.bounded(5) == 0) {
        return pick(random, {"", "", "   ", "\t"});
    }
    return randomTodo(random);
}

} // namespace

QByteArray randomTodoFile(QRandomGenerator &random) {
    QByteArray file;
    const int lineCount = random.bounded(30);
    for (int i = 0; i < lineCount; ++i) {
        file.append(randomLine(random));
        file.append('\n');
    }
    return file;
}

} // namespace topdo::test
