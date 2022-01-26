#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>

int main() {
	using namespace std;

	vector<int> ints {1, 2, 3 ,4 , 5, 6};

	auto it = std::find(ints.cend(), ints.cend(), 2);

	assert(it == ints.cend());

	if (it == ints.cend() ) {
		cout << "no error thrown" << endl;
	} else {
		cout << "should not be possible" << endl;
	}

	return 0;
}

