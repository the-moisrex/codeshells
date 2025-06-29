import standard;

#include <cstdint>
#include <filesystem>
#include <print>
#include <ranges>

using namespace std;
struct evdev {
    std::filesystem::path p;
};

/**
 * @brief Factory function to create a view over all valid input devices.
 *
 * @param dir The path to the input device directory. Defaults to "/dev/input".
 * @return A lazy view object that can be used in a range-based for loop.
 */
[[nodiscard]] static auto
all_input_devices(std::filesystem::path const& dir = "/dev/input") {
    using std::filesystem::directory_entry;
    using std::filesystem::directory_iterator;
    using std::ranges::views::filter;
    using std::ranges::views::transform;

    // Create a view over the directory entries. This may throw if the path is
    // invalid. We wrap it to return an empty view on error, making it safer for
    // the caller.
    auto dir_view =
        is_directory(dir) ? directory_iterator{dir} : directory_iterator{};

    // Define the pipeline of operations:
    return dir_view
           // 1. Filter the directory entries to find potential device files.
           | filter([](directory_entry const& entry) {
                 std::string const filename = entry.path().filename().string();
                 return entry.is_character_file() &&
                        filename.starts_with("event");
             })
           // 2. Transform the valid directory entries into evdev objects.
           | transform([](directory_entry const& entry) {
                 return evdev{entry.path()};
             });
}

int main() {

    auto devs = all_input_devices();
    for (auto const& dev : devs) {
        println("Number: {}", dev.p.string());
    }

    return 0;
}

// view ./build/optimizations.txt
