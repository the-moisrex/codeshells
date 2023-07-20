#include "cat.h"
#include <QDebug>

Cat::Cat(QObject* parent) : QObject{parent} {}

Cat::~Cat() {
    qInfo() << "Destructing" << this << "child of" << this->parent();
}
