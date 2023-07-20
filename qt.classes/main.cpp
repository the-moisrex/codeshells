#include "test.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char* argv[]) {
    QCoreApplication a(argc, argv);

    qInfo() << "Sizeof a test: " << sizeof(test);

    test t;

    // connect the signal and the slot
    QObject::connect(&t, &test::close, &a, &QCoreApplication::quit,
                     Qt::QueuedConnection);

    t.print_stuff();

    int res = a.exec();
    qInfo() << "Exit value: " << res;
    return res;
}
