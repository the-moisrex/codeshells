#ifndef CAT_H
#define CAT_H

#include <QObject>
#include <QString>
#include <QStringView>

class Cat : public QObject {
  Q_OBJECT
public:
  explicit Cat(QObject *parent = nullptr);

  void setAge(int inp_age) noexcept;
  int getAge() const noexcept;

  void setName(QStringView inp_name);
  QStringView getName() const noexcept;

signals:

private:
  int age;
  QString name;
};

#endif // CAT_H
