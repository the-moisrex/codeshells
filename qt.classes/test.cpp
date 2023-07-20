#include "test.h"
#include <QDebug>

test::test(QObject* parent) : QObject{parent} {}

void test::print_stuff() {
    qInfo() << "Printing stuff.";
    emit close();
}
