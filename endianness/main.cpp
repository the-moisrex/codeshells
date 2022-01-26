#include <iostream>

using namespace std;

#if __BYTE_ORDER == __LITTLE_ENDIAN
template <class T> T readBigEndian(T value) {
  T result;
  char *pValue = (char *)&value - 1;
  char *pValueEnd = pValue + sizeof(T);
  char *pResult = (char *)&result + sizeof(T);
  while (pValue != pValueEnd)
    *--pResult = *++pValue;
  return result;
}
#elif __BYTE_ORDER == __BIG_ENDIAN
template <class T> T readBigEndian(T value) { return value; }
#endif

struct num {
  uint8_t a;
  uint8_t b;
};

auto main() -> int {
  num n{0x1, 0xFF};
  cout << hex;
  cout << *reinterpret_cast<uint16_t*>(&n) << endl;
  cout << *(uint16_t*)& n.a << endl;
  cout << readBigEndian(*(uint16_t*)&n.a) << endl;
  cout << (n.a << 8 | n.b) << endl;

  return 0;
}

