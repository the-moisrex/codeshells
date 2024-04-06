#include <iostream>

using namespace std;

// https://x.com/shachaf/status/1776752111688241204
struct range_struct {
    int i;
    int stop;
    int step;
    int state = 0;

    explicit range_struct(int stop, int step = 1) : stop{stop}, step{step} {}

    int resume() {
        if (state == 1) {
            goto continue_state;
        }
        for (i = 0; i < stop; ++i) {
            state = 1;
            return i;
        continue_state:
            continue;
        }
        return 0;
    }
};

auto main() -> int {

    range_struct coro(10);
    cout << coro.resume() << endl;
    cout << coro.resume() << endl;
    cout << coro.resume() << endl;
    cout << coro.resume() << endl;

    return 0;
}
