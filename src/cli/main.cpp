#include "cli.h"

#include <QCoreApplication>

#include <cstdio>

int main(int argc, char *argv[]) {
    const QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    QTextStream err(stderr);
    return topdo::runCli(QCoreApplication::arguments(), out, err);
}
