#include <system_error>
#include <vector>
#include <functional>
#include <string_view>
#include <string>
#include <iostream>
#include <array>
#include <experimental/socket>
#include <experimental/net>
#include <experimental/internet>
#include <experimental/buffer>

namespace std {
  namespace net = experimental::net;
}


class session {
  private:

    std::net::ip::tcp::socket _socket;
    std::array<char, 1024> buf;

    void read() {
      std::cout << "Reading ..." << std::endl;
        _socket.async_read_some(std::net::buffer(buf, 80), [this] (std::error_code const& ec2, std::size_t length){
            std::cout << "Reading: " << length << std::endl;
            if (!ec2) {
              buf[buf.size() - 2] = '\0';
              std::string_view data{buf.data(), 1024};
              std::cout << data << std::endl;
            }
            write();
        });
    }

    void write() {

        auto res = R"END(Status: 200 OK
Content-Length: 9
Content-Type: text/html; charset=utf-8

main page
)END";
        std::net::async_write(_socket, std::net::buffer(res, sizeof(res) / sizeof(char)), [](std::error_code const& ec3, std::size_t len) {
            std::cout << "Done" << std::endl;
            });

    }

  public:

    session(std::net::ip::tcp::socket &&_socket) : _socket(std::move(_socket)) {
        read();
    }

};


auto main() -> int {

  try {
    std::net::io_context io;
    std::net::ip::tcp::endpoint addr(std::net::ip::tcp::v4(), 4000);
    std::net::ip::tcp::acceptor acceptor(io, addr);
  std::function<void()> do_accept;
  std::vector<session> sessions;

  do_accept = [&] {
    try {
    acceptor.async_accept([&] (std::error_code const& ec, std::net::ip::tcp::socket socket) {
        std::cout << ec << " " << "Connection established." << std::endl;
        if (!ec)
          sessions.emplace_back(std::move(socket));
        do_accept();
    });
    } catch (std::exception const& err) {
      std::cerr << err.what() << std::endl;
    }
  };
  do_accept();



  io.run();

  } catch (std::exception const& err) {
    std::cerr << err.what() << std::endl;
  } catch (...) {
    std::cerr << "unknown error" << std::endl;
  }
  return 0;
}

