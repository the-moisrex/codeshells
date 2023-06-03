#include <algorithm>
#include <experimental/iterator>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

using namespace std;

struct user {
  int id = 0;
  string name;
};

// doesn't work because ostream_joiner::difference_type is void while,
// ranges::copy requires "difference_type" to be size-like
// void print_users(ranges::input_range auto &&users) {
//   ranges::copy(users | views::transform(&user::name),
//                experimental::make_ostream_joiner(cout, ", "));
// }

void print_users(ranges::input_range auto &&users) {
  ranges::copy(ranges::ref_view(users) | views::transform(&user::name) |
                   views::take(users.size() - 1),
               ostream_iterator<string>(cout, ", "));
  cout << users.back().name << endl;
}

auto main() -> int {

  vector<user> users;
  users.emplace_back(user{1, "user1"});
  users.emplace_back(user{3, "user3"});
  users.emplace_back(user{2, "user2"});

  print_users(users);
  ranges::sort(users, {}, &user::id);
  print_users(users);

  return 0;
}
