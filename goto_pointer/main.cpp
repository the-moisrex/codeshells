#include <iostream>

using namespace std;


void* fun_father() {
cpu:
    cout << "cool" << endl;
    void* a = &&cpu;
    return a;
}

void fun(void* cpu) {

    void* labels[] = {
        &&one,
        &&two,
        &&three
    };

    goto* labels[1];

one:
    cout << "one" << endl;
    goto* cpu;
two:
    cout << "two" << endl;
    goto* cpu;
three:
    cout << "three" << endl;
    goto* cpu;
}


auto main() -> int {
    fun(fun_father());

  return 0;
}
