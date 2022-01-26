#include "pch.h"
#include <system_error>

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

using std::cout;
using std::cerr;
using std::endl;
using std::string;


class session {
  private:

    tcp::socket _socket;
    std::array<char, 1024> buf;

    void read() {
        cout << "Reading ..." << endl;
        _socket.async_read_some(buffer(buf, 80), [this] (std::error_code const& ec2, std::size_t length){
            cout << "Reading: " << length << endl;
            if (!ec2) {
              buf[buf.size() - 2] = '\0';
              std::string_view data{buf.data(), 1024};
              cout << data << endl;
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
        async_write(_socket, buffer(res, sizeof(res) / sizeof(char)), [](std::error_code const& ec3, std::size_t len) {
              cout << "Done" << endl;
            });

    }

  public:

    session(tcp::socket &&_socket) : _socket(std::move(_socket)) {
        read();
    }

};


auto main() -> int {

  try {
  io_context io;
  tcp::endpoint addr(tcp::v4(), 4000);
  tcp::acceptor acceptor(io, addr);
  std::function<void()> do_accept;
  std::vector<session> sessions;

  do_accept = [&] {
    try {
    acceptor.async_accept([&] (std::error_code const& ec, tcp::socket socket) {
        cout << ec << " " << "Connection established." << endl;
        if (!ec)
          sessions.emplace_back(std::move(socket));
        do_accept();
    });
    } catch (std::exception const& err) {
        cerr << err.what() << endl;
    }
  };
  do_accept();



  std::error_code ec;
  boost::system::error_code bec;
  io.run(bec);

  } catch (std::exception const& err) {
      cerr << err.what() << endl;
  } catch (...) {
      cerr << "unknown error" << endl;
  }
  return 0;
}
