#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

using namespace std;
using boost::asio::ip::tcp;

int main() {

  try {
    boost::asio::io_context io_ctx;
    tcp::resolver resolver(io_ctx);
    tcp::resolver::results_type endpoints =
        resolver.resolve("google.com", "daytime");

    tcp::socket socket(io_ctx);

    boost::asio::connect(socket, endpoints);

    for (;;) {
      boost::array<char, 128> buff;
      boost::system::error_code err;

      size_t len = socket.read_some(boost::asio::buffer(buff), err);

      if (err == boost::asio::error::eof) {
        break; // connection closed cleanly by pear
      } else if (err) {
        throw boost::system::system_error(err); // some other error
      }

      cout.write(buff.data(), len);
    }

  } catch (std::exception const &e) {
    cerr << e.what() << endl;
  }
}
