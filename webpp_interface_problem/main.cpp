#include <iostream>

using namespace std;


class cgi {
	public:

		std::string getName() { return "hello"; }
};

template<typename Interface>
class server : public Interface {
	public:

		std::string getName() {
			getName();
		}

};



class request {
	
	public:
		base_interface *interface;

	request(base_interface* intface) : interface(intface) {}

		std::string getName() { return interface->getName(); }
};

auto main() -> int {

	cgi c;


	request req(&c);
	cout << req.getName() << endl;

  return 0;
}
