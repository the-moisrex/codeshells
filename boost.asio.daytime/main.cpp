#include "pch.h"

using std::cerr;
using std::cout;
using std::endl;
using namespace boost;
using namespace boost::asio;
using boost::asio::ip::tcp;

// example output: Mon Mar  2 14:33:21 2020
auto make_daytime_string() {
  using namespace std;
  time_t now = time(0);
  return ctime(&now);
};

void daytime_client() {

  try {
    io_context io;
    tcp::resolver resolver(io);
    tcp::resolver::results_type endpoints =
        resolver.resolve("127.0.0.1", "daytime");

    for (int i = 0; i < 22; i++) {
      tcp::socket socket(io);
      connect(socket, endpoints);

      for (;;) {
        array<char, 128> buf;
        system::error_code error;

        size_t len = socket.read_some(buffer(buf), error);

        if (error == error::eof) {
          break; // Connection closed cleanly by pear
        } else if (error) {
          throw system::system_error(error); // some other error
        }

        cout << i << "\t";
        cout.write(buf.data(), len);
      }
    }
  } catch (std::exception const &err) {
    cerr << err.what() << endl;
  }
}

void daytime_server() {

  try {
    io_context io;
    tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 13));
    for (int i = 0; i < 20; i++) {
      tcp::socket socket(io);
      acceptor.accept(socket);

      std::string message = make_daytime_string();

      system::error_code ignored_error;
      write(socket, buffer(message), ignored_error);
    }
  } catch (std::exception const &err) {
    cerr << err.what();
  }
}

void daytime_server2() {

  try {
    io_context io;
    tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 13));
    tcp::socket socket(io);

    for (int i = 0; i < 20; i++) {
      acceptor.accept(socket);

      std::string message = make_daytime_string();

      system::error_code ignored_error;
      write(socket, buffer(message), ignored_error);
      socket.close();
    }
  } catch (std::exception const &err) {
    cerr << err.what();
  }
}

namespace v1 {
class tcp_connection : public boost::enable_shared_from_this<tcp_connection> {
public:
  using pointer = boost::shared_ptr<tcp_connection>;

  static pointer create(io_context &io) {
    return pointer{new tcp_connection(io)};
  }

  tcp::socket &socket() { return _socket; }

  void start() {
    msg = make_daytime_string();

    async_write(_socket, buffer(msg),
                bind(&tcp_connection::handle_write, shared_from_this()
                     /*,boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred*/));
  }

private:
  tcp_connection(boost::asio::io_context &io) : _socket(io) {}

  void handle_write(system::error_code &, size_t /*byte_transferred*/) {}
  void handle_write() {}

  tcp::socket _socket;
  std::string msg;
};

class tcp_server {
protected:
  io_context &io;
  tcp::acceptor acceptor;

  void start_accept() {
    tcp_connection::pointer new_connection = tcp_connection::create(io);
    acceptor.async_accept(new_connection->socket(),
                          bind(&tcp_server::handle_accept, this, new_connection,
                               boost::asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer new_connection,
                     const system::error_code &error) {
    if (!error) {
      new_connection->start();
    }
    start_accept();
  }

public:
  tcp_server(io_context &io)
      : io(io), acceptor(io, tcp::endpoint(tcp::v4(), 13)) {
    start_accept();
  }
};
} // namespace v1

void async_daytime_server() {
  try {
    io_context io;
    v1::tcp_server server(io);
    io.run();
  } catch (std::exception const &err) {
    cerr << err.what() << endl;
  }
}

auto main() -> int {

  thread tr_server(&async_daytime_server);
  boost::this_thread::sleep_for(boost::chrono::milliseconds(200));

  thread tr_client(&daytime_client);

  tr_client.join();
  tr_server.join();

  return 0;
}
