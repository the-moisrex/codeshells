#include "./cat.h"
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <string>

void printHeirarchy(QObject* obj, std::string spaces = "") {
    qInfo() << spaces.c_str() << "-" << obj;
    spaces += " ";
    foreach (QObject* child, obj->children()) {
        printHeirarchy(child, spaces);
    }
}

Cat* getCat(QObject* parent = nullptr) {
    // automatically gets deleted when the parent is destructed
    return new Cat(parent);
}

int main(int argc, char* argv[]) {
    QCoreApplication a(argc, argv);

    QTimer timer;
    timer.singleShot(3000, &a, &QCoreApplication::quit);

    auto* kitty = getCat(&a);
    kitty->setObjectName("Susan");

    auto* stray = getCat();
    stray->setObjectName("Stray");

    // if we don't give it a parent, it's a memory leak
    stray->setParent(kitty);

    printHeirarchy(&a);

    int res = a.exec();
    qInfo() << "Done with Exit code:" << res;
    return res;
}
