#pragma once

#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <list>

namespace v1 {

class notification_queue {

  using lock_t = std::unique_lock<std::mutex>;

  std::deque<std::function<void()>> _q;
  bool _done{false};
  std::mutex _mutex;
  std::condition_variable _ready;

public:
  void done() {
    {
      lock_t lock{_mutex};
      _done = true;
    }
    _ready.notify_all();
  }

  bool pop(std::function<void()> &func) {
    lock_t locked{_mutex};
    while (_q.empty() && !_done) {
      _ready.wait(locked);
    }
    if (_q.empty())
      return false;
    func = std::move(_q.front());
    _q.pop_front();
    return true;
  }

  template <typename T> void push(T &&f) {
    {
      lock_t lock{_mutex};
      _q.emplace_back(std::forward<T>(f));
    }
    _ready.notify_one();
  }
};

class task_system {
  const unsigned _count{std::thread::hardware_concurrency() - 1};
  std::vector<std::thread> _threads;
  notification_queue _q;

  void run(unsigned /* i */) {
    for (;;) {
      std::function<void()> func;
      if (!_q.pop(func)) {
        break;
      }
      func();
    }
  }

public:
  task_system() {
    for (unsigned n = 0; n != _count; ++n) {
      _threads.emplace_back([&, n] { run(n); });
    }
  }

  ~task_system() {
    _q.done();
    for (auto &e : _threads)
      e.join();
  }

  template <typename F> void async_(F &&f) { _q.push(std::forward<F>(f)); }
};

} // namespace v1




















namespace v2 {

class notification_queue {

  using lock_t = std::unique_lock<std::mutex>;

  std::deque<std::function<void()>> _q;
  bool _done{false};
  std::mutex _mutex;
  std::condition_variable _ready;

public:

  void done() {
    {
      lock_t lock{_mutex};
      _done = true;
    }
    _ready.notify_all();
  }

  bool pop(std::function<void()> &func) {
    lock_t locked{_mutex};
    while (_q.empty() && !_done) {
      _ready.wait(locked);
    }
    if (_q.empty())
      return false;
    func = std::move(_q.front());
    _q.pop_front();
    return true;
  }

  template <typename T> void push(T &&f) {
    {
      lock_t lock{_mutex};
      _q.emplace_back(std::forward<T>(f));
    }
    _ready.notify_one();
  }
};

class task_system {
  const unsigned _count{std::thread::hardware_concurrency() - 1};
  std::vector<std::thread> _threads;
  std::vector<notification_queue> _q{_count};
  std::atomic<unsigned> _index{0};

  void run(unsigned i) {
    for (;;) {
      std::function<void()> func;
      if (!_q[i].pop(func)) {
        break;
      }
      func();
    }
  }

public:
  task_system() {
    for (unsigned n = 0; n != _count; ++n) {
      _threads.emplace_back([&, n] { run(n); });
    }
  }

  ~task_system() {
    for(auto& e: _q) 
      e.done();
    for (auto &e : _threads)
      e.join();
  }

  template <typename F> void async_(F &&f) { 
    auto i = _index++;
    _q[i % _count].push(std::forward<F>(f));
  }
};

} // namespace v2




















namespace v3 {

class notification_queue {

  using lock_t = std::unique_lock<std::mutex>;

  std::deque<std::function<void()>> _q;
  bool _done{false};
  std::mutex _mutex;
  std::condition_variable _ready;

public:

  bool try_pop(std::function<void()>& func) {
    lock_t lock(_mutex, std::try_to_lock);
    if (!lock || _q.empty())
      return false;
    func = std::move(_q.front());
    _q.pop_front();
    return true;
  }

  template <typename F>
  bool try_push(F&&f) {
    {
      lock_t lock{_mutex, std::try_to_lock};
      if (!lock)
        return false;
      _q.emplace_back(std::forward<F>(f));
    }
    _ready.notify_one();
    return true;
  }

  void done() {
    {
      lock_t lock{_mutex};
      _done = true;
    }
    _ready.notify_all();
  }

  bool pop(std::function<void()> &func) {
    lock_t locked{_mutex};
    while (_q.empty() && !_done) {
      _ready.wait(locked);
    }
    if (_q.empty())
      return false;
    func = std::move(_q.front());
    _q.pop_front();
    return true;
  }

  template <typename T> void push(T &&f) {
    {
      lock_t lock{_mutex};
      _q.emplace_back(std::forward<T>(f));
    }
    _ready.notify_one();
  }
};

constexpr int K = 10;
class task_system {
  const unsigned _count{std::thread::hardware_concurrency() - 1};
  std::vector<std::thread> _threads;
  std::vector<notification_queue> _q{_count};
  std::atomic<unsigned> _index{0};

  void run(unsigned i) {
    for (;;) {
      std::function<void()> func;
      for (unsigned n =0; n != _count; ++n) {
          if (_q[(i + n) % _count].try_pop(func))
            break;
      }
      if (!func && !_q[i].pop(func)) {
        break;
      }
      func();
    }
  }

public:
  task_system() {
    for (unsigned n = 0; n != _count; ++n) {
      _threads.emplace_back([&, n] { run(n); });
    }
  }

  ~task_system() {
    for(auto& e: _q) 
      e.done();
    for (auto &e : _threads)
      e.join();
  }

  template <typename F> void async_(F &&f) { 
    auto i = _index++;
    for (unsigned n = 0; n != _count * K; ++n) {
        if (_q[(i + n) % _count].try_push(std::forward<F>(f)))
          return;
    }
    _q[i % _count].push(std::forward<F>(f));
  }
};

} // namespace v2



