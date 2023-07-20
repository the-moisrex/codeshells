#ifndef TEST_H
#define TEST_H

#include <QObject>

class test : public QObject {
    Q_OBJECT
  public:
    explicit test(QObject* parent = nullptr);

  signals:
    void close();

  public slots:
    void print_stuff();
};

#endif // TEST_H
