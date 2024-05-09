#include <chrono>
#include <print>

using namespace std;

auto main() -> int {

  const auto duration = std::chrono::system_clock::now();

  try {
    auto &tzdb = std::chrono::get_tzdb();

    auto i = 0;
    for (auto &it : tzdb.zones) {
      auto sysinfo = it.get_info(duration);
      std::println("#{} | {}, TIME: {}", i++, sysinfo, it.to_local(duration));
    }
    std::println("Version: {}", tzdb.version);
  } catch (std::exception const &ex) {
    std::println("NO timezones: ", ex.what());
  }

  return 0;
}
