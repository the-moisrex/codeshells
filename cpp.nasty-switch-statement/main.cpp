#include <iostream>

using namespace std;

// from: https://probablydance.com/2021/10/31/c-coroutines-do-not-spark-joy/
struct range_struct {
    int i;
    int stop;
    int step;
    enum { At_start, In_loop, Done } state;

    explicit range_struct(int stop, int step = 1)
        : stop(stop), step(step), state(At_start) {}

    int resume() {
        switch (state) {
        case At_start:
            for (i = 0; i < stop;) {
                state = In_loop;
                return i;
            case In_loop:
                i += step;
            }
            state = Done;
        case Done:
            return 0;
        }
        return -1;
    }
};

auto main() -> int {

    range_struct s(10);
    s.resume();
    s.resume();
    s.resume();
    cout << s.i << endl;

    return 0;
}
