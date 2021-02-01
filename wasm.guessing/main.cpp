#include <emscripten.h>

#include <iostream>
#include <random>
#include <string_view>
using namespace std;


void guessing_game(void* _guess) {
  const int guess = *static_cast<const int*>(_guess);
  int user_guess = 0;
  cout << "Enter a guess (1-100): \n";

  cin >> user_guess;

  if (user_guess > 100 || user_guess < 1) {
    cerr << "Invalid input dude";
    emscripten_cancel_main_loop();
  } else if (user_guess > guess) {
    cout << "Too high" << endl;
  } else if (user_guess < guess) {
    cout << "Too low" << endl;
  } else {
    cout << "Correct." << endl;
    emscripten_cancel_main_loop();
  }
}



int main() {
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<int> dist(0, 100);
  int guess = dist(gen);
  emscripten_set_main_loop_arg(guessing_game,
                               &guess,
                               0,
                               false);
  return 0;
}

