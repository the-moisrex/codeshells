#ifndef CAT_H
#define CAT_H

#include <QObject>

class Cat : public QObject {
    Q_OBJECT
  public:
    explicit Cat(QObject* parent = nullptr);
    ~Cat();

  signals:
};

#endif // CAT_H
