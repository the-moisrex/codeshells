#include "pch.h"

using namespace boost::asio;
using std::cout;
using std::endl;

void __timer() {
  io_context io;

  cout << "wait started" << endl;

  steady_timer t(io, chrono::seconds(1));
  t.wait();

  cout << "Done" << endl;
}

void __async_timer() {

  io_context io;

  cout << "wait started" << endl;

  steady_timer timer(io, chrono::seconds(2));

  timer.async_wait(
      [](boost::system::error_code const &) { cout << "done" << endl; });

  cout << "No wait to get here!" << endl;
  io.run();
  cout << "everything is finished" << endl;
}

void __argument_binder() {
  io_context io;

  int count = 0;

  steady_timer timer(io, chrono::seconds(1));
  std::function<void(const boost::system::error_code &, steady_timer *timer,
                     int *count)>
      callback;
  callback = [&](const boost::system::error_code &, steady_timer *timer,
                 int *count) {
    if (*count < 5) {
      cout << *count << endl;
      ++(*count);
      timer->expires_at(timer->expiry() + chrono::seconds(1));
      timer->async_wait(
          boost::bind(callback, placeholders::error, timer, count));
    }
  };
  timer.async_wait(
      boost::bind(callback, boost::asio::placeholders::error, &timer, &count));
  io.run();
}




///////////////////////////////////////////////////////////////////////////



class __printer_v1 {
  protected:
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::steady_timer timer1_;
    boost::asio::steady_timer timer2_;
    int count_;
  public:

    __printer_v1(boost::asio::io_context &io)
      : strand_(make_strand(io)),
        timer1_(io, chrono::seconds(1)),
        timer2_(io, chrono::seconds(1)),
        count_(0)
    {
      timer1_.async_wait(
          bind_executor(strand_, 
              boost::bind(&__printer_v1::print1, this)
            )
          );
      timer2_.async_wait(
          bind_executor(strand_,
              boost::bind(&__printer_v1::print2, this)
            )
          );
    }

    ~__printer_v1() {
        cout << "Final count is: " << count_ << endl;
    }

    void print1() {
      if (count_ < 10) {
        cout << "Timer 1: " << count_ << endl;
        ++count_;
        timer1_.expires_at(timer1_.expiry() + chrono::seconds(1));
        timer1_.async_wait(
              bind_executor(strand_, boost::bind(&__printer_v1::print1, this))
            );
      }
    }

    void print2() {
      if (count_ < 10) {
          cout << "Timer 2: " << count_ << endl;
          ++count_;
          timer2_.expires_at(timer2_.expiry() + chrono::seconds(1));
          timer2_.async_wait(bind_executor(strand_, boost::bind(&__printer_v1::print2, this)));
      }
    }
};

void __sync_handlers_multithreaded() {
  io_context io;
  __printer_v1 printer(io);
  boost::thread tr(boost::bind(&io_context::run, &io));
  io.run();
  tr.join();
}


auto main() -> int {

  //__timer();
  //__async_timer();
  //__argument_binder();
  __sync_handlers_multithreaded();

  return 0;
}
