#include <iostream>
#include <stdlib.h>
#ifdef __WIN32__
#include <malloc.h>
#else
#include <stdlib.h>
#endif

using namespace std;

struct A {

  int *val;

  A(int v) :
#ifdef __WIN32__
    val {static_cast<int *>(_malloca(sizeof(int)))}
#else
    val {static_cast<int *>(alloca(sizeof(int)))}
#endif
    {
    *val = v;
  }

#ifdef __WIN32__
  constexpr ~A() { _freea(val); }
#endif
};

auto main() -> int {

  A aa(23);
  A bb(43);
  cout << *aa.val << endl;
  cout << *bb.val << endl;

  A* a = new A(23);
  A* b = new A(43);
  cout << *a->val << endl;
  cout << *b->val << endl;
  delete a;
  delete b;
  return 0;
}
