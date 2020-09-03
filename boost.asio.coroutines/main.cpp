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
  tcp::endpoint tcp_endpoint{tcp::v4(), 2020};
  tcp::acceptor tcp_acceptor{io, tcp_endpoint};

  std::list<tcp::socket> tcp_sockets;
  

  tcp_acceptor.listen();
  spawn(io, [&](yield_context yield) {
        for (int i = 0; i < 2; i++) {
          tcp_sockets.emplace_back(io);
          tcp_acceptor.async_accept(tcp_sockets.back(), yield);
          spawn(io, [&] (yield_context yield) {
                  std::time_t now = std::time(nullptr);
                  std::string data = std::ctime(&now);
                  auto& sock = tcp_sockets.back();
                  async_write(sock, buffer(data), yield);
                  sock.shutdown(tcp::socket::shutdown_send);
              });
        }
      });

  io.run();

  return 0;
}
