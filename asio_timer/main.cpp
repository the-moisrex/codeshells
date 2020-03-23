#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

using std::cout;
using std::endl;
using namespace boost::asio;
using boost::asio::ip::tcp;

int main() {

  io_context io;
  steady_timer timer{io, chrono::seconds(5)};

  timer.wait();

  cout << "Hello world" << endl;
}
