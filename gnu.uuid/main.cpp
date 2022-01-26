#include <iostream>
#include <uuid/uuid.h>

using namespace std;

auto main() -> int {

  uuid_t id;
  uuid_generate(id);

  std::string res;
  res.resize(16 * 2);
  uuid_unparse(id, res.data());

  cout << res << endl;

  return 0;
}
