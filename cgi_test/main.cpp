#include <iostream>
#include <string>
#include <string_view>


using namespace std;

std::string_view env(std::string_view const& key) {
	if (auto e = getenv(key.data()))
		return e;
	return {};
}

std::string_view method() {
	return env("REQUEST_METHOD");
}


auto main() -> int {

	cout << method() << endl;

  return 0;
}
