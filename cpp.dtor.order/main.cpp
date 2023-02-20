#include <iostream>

using namespace std;

struct E {
     E() { cout << "Construct E" << endl; }
     ~E() { cout << "Destruct E" << endl; }
     int res = 0;
};

class B {
public:
  B() { cout << "Construct B" << endl; }

  ~B() { cout << "Destruct B" << endl; }
};

class D : public B {
public:

  E e;
  D() { cout << "Construct D" << endl; }

  ~D() { cout << "Destruct D" << endl; }
};

int main(int argc, char **argv) {
  D d;
  return d.e.res;
}
