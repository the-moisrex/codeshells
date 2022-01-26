#include "pch.h"

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

using std::cout;
using std::cerr;
using std::endl;
using std::string;

auto main() -> int {

  io_context io;

  signal_set signals(io, SIGINT, SIGTERM);
  signals.async_wait([] (system::error_code const& err, int signal_num) {
        if (!err) {
            cerr << signal_num << " <-----" << endl;
        }
      });

  io.run();


  return 0;
}
