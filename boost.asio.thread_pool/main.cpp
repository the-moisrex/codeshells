#include "pch.h"

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

using std::cout;
using std::cerr;
using std::endl;
using std::string;

auto main() -> int {

  thread_pool pool(20);

  for (int i = 0; i < 200; i++) {
    post(pool, [i] () {
          cout << i << " " << "testing" << endl;
        });
  }


  return 0;
}
