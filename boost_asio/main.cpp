#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace boost::asio;

void print(const boost::system::error_code& err, steady_timer* timer,
           int* count) {
    if (*count < 5) {
        std::cout << "counting like a human: " << *count << std::endl;
        (*count)++;
        timer->expires_at(timer->expiry() + chrono::seconds(1));
        timer->async_wait(
            boost::bind(print, placeholders::error, timer, count));
    }
}

class printer {
  private:
    int count = 0;
    boost::asio::steady_timer timer;

  public:
    explicit printer(boost::asio::io_context& ioc)
        : timer(ioc, boost::asio::chrono::seconds(1)) {
        timer.async_wait(boost::bind(&printer::print, this,
                                     boost::asio::placeholders::error));
    }

    void print(const boost::system::error_code& err) {
        if (count < 5) {
            std::cout << "Counting like a boss: " << count << std::endl;
            count++;
            timer.expires_at(timer.expiry() + chrono::seconds(1));
            timer.async_wait(boost::bind(&printer::print, this,
                                         boost::asio::placeholders::error));
        }
    }

    ~printer() { std::cout << "I'm going to kill myself now" << std::endl; }
};

auto main() -> int {
    io_context ioc;
    printer p(ioc);
    ioc.run();

    return 0;
}
