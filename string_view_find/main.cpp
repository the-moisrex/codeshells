#include <iostream>
#include <string_view>
#include <string>

using namespace std;

auto main() -> int {

	string url = "//[::1]:8080/folder/file.md?name=value&name2=value2#str";
	string_view a = url;

	cout << a << endl;
	cout << a.substr(0, 51).find('?') << endl;


  return 0;
}
