#include "cat.h"

Cat::Cat(QObject *parent) : QObject{parent} {}

void Cat::setAge(int inp_age) noexcept { age = inp_age; }

int Cat::getAge() const noexcept { return age; }

void Cat::setName(QStringView inp_name) {
  name = QString{inp_name.data(), static_cast<int>(inp_name.size())};
}

QStringView Cat::getName() const noexcept { return {name}; }
