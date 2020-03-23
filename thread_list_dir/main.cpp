#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace std::filesystem;

class FileMonitor {
private:
  queue<path> paths;
  queue<string> files;
  mutex paths_mtx;
  mutex files_mtx;
  condition_variable waiter;

  queue<thread> pool;

  bool is_finished = false;
  mutex finished_mtx;

  void check_dir() {
    unique_lock<mutex> file_lock(files_mtx, defer_lock);
    unique_lock<mutex> path_lock(paths_mtx);
    path dir = paths.front();
    paths.pop();
    path_lock.unlock();

    if (!is_directory(dir)) {
      cout << dir.string() << endl;
      return;
    }

    for (;;) {
      try {
        // read the files:
        for (auto const& node : directory_iterator(dir)) {
            if (is_directory(node)) {
                path_lock.lock();
                paths.push(node);
                path_lock.unlock();
            } else {
                file_lock.lock();
                files.push(node.path().string());
                file_lock.unlock();
            }
        }
      } catch (filesystem_error const &err) {
        cerr << err.what() << endl;
      }
    }
  }

public:
  FileMonitor(path __path) {
    paths.emplace(std::move(__path));
    for (int i = 0; i < 10; i++)
      pool.emplace(&FileMonitor::check_dir, this);
  }

  ~FileMonitor() {
    while (!pool.empty()) {
      auto &t = pool.front();
      if (t.joinable())
        t.join();
      pool.pop();
    }
  }

  bool finished() {
    scoped_lock<mutex> lock(finished_mtx);
    return is_finished;
  }

  void print() {
    while (!finished()) {
      unique_lock<mutex> lock(files_mtx);
      waiter.wait(lock, [&] { return !files.empty(); });

      while (!files.empty()) {
        auto file = files.front();
        cout << file << endl;
        files.pop();
      }

      lock.unlock();
    }
  }
};

int main() {
  FileMonitor fm("/home/moisrex/");
fm.print();

  return 0;
}

