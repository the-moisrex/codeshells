#include "cat.h"
#include <QCoreApplication>
#include <QDebug>
#include <QMap>
#include <QRandomGenerator>
#include <QSharedPointer>
#include <QStringView>

using catMap = QMap<QString, QSharedPointer<Cat>>;

catMap getCats() {
  catMap cats;
  for (int i = 0; i < 5; ++i) {
    QSharedPointer<Cat> ptr{new Cat{}};
    ptr->setAge(QRandomGenerator::global()->bounded(1, 5));
    ptr->setName(QStringView{u"Unknown"});
    cats.insert("pet-" + QString::number(i), ptr);
  }

  return cats;
}

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);

  auto cats = getCats();
  qInfo() << cats;

  cats["pet-2"]->setName(u"Stray");

  foreach ( auto key, cats.keys()) {
    auto cat = cats.value(key);
    qInfo() << " - " << key << cat->getName() << cat->getAge();
  }

  int res = a.exec();
  qInfo() << "Done with exit code:" << res;
  return res;
}
