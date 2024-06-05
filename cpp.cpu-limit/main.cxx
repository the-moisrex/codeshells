import standard;

#include <cstdint>
#include <iostream>
#include <optional>
#include <print>

using namespace std;

static constexpr uint8_t worlds = 7;

std::optional<double> CpuLimitRtc() {
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  const char *cpu_limit_c_str = std::getenv("CPU_LIMIT");
  if (!cpu_limit_c_str) {
    cout << "CPU_LIMIT env is unset, ignoring it" << endl;
    return {};
  }

  std::string cpu_limit(cpu_limit_c_str);
  cout << "CPU_LIMIT='" << cpu_limit << "'" << endl;

  try {
    size_t end{0};
    auto cpu_f = std::stod(cpu_limit, &end);
    if (cpu_limit.substr(end) != "c") {
      return {};
    }

    return {cpu_f};
  } catch (const std::exception &e) {
    cout << "Failed to parse CPU_LIMIT: " << e.what() << endl;
    ;
  }
  cout << "CPU_LIMIT env is invalid (" << cpu_limit << "), ignoring it" << endl;

  return {};
}

std::optional<double> CpuLimit() {
  static const auto limit = CpuLimitRtc();
  return limit;
}

bool IsInRtc() { return !!CpuLimitRtc(); }

int main() {

  println("CPU Limit {} worlds.", *CpuLimit());

  return 0;
}
