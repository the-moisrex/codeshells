#include <iostream>

using namespace std;

        template <typename Integer>
        constexpr bool octet(Integer o) noexcept {
            constexpr auto mask = static_cast<Integer>(1) << ((sizeof(Integer) * 8) - 1);
            while ((o & mask) == mask)
                o <<= 1;
            return o == 0;
        }


auto main() -> int {

	uint8_t a = 128;
	uint32_t b = 256;

	cout << boolalpha;
	cout << octet(a) << endl
		<< octet(b) << endl;


  return 0;
}
