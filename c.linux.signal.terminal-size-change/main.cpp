#include <iostream>
#include <thread>

#include <signal.h>
using namespace std;

void handle_winch(int sig) {
    // Here you would implement the actions to be taken on window size change
    std::cout << "Window size changed!" << std::endl;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_winch;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGWINCH, &sa, NULL) == -1) {
        std::perror("sigaction");
        return 1;
    }

    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        cout << "working" << endl;
    }

    return 0;
}
