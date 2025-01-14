import standard;

#include <print>

int main() {
    using std::print;

    for (int i = 0;; ++i) {
        switch (i) {
        case 0:
            print("[");
            if (false) case 1 ... 9: 
                print(", ");
            print("{}", i);
            continue;
        case 10:
            print("]");
            break;
        }
        break;
    }

    return 0;
}

