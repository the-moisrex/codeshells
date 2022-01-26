#include <iostream>


#include <endian.h>


namespace Encoding {

inline uint16_t Swap16(uint16_t v)
{
    return (((v & 0x00ffU) << 8) & 0xff00) | (((v & 0xff00U) >> 8) & 0x00ff);
}

inline uint32_t Swap32(uint32_t v)
{
    return ((v & static_cast<uint32_t>(0x000000ffUL)) << 24) | ((v & static_cast<uint32_t>(0x0000ff00UL)) << 8) |
           ((v & static_cast<uint32_t>(0x00ff0000UL)) >> 8) | ((v & static_cast<uint32_t>(0xff000000UL)) >> 24);
}

inline uint64_t Swap64(uint64_t v)
{
    return ((v & static_cast<uint64_t>(0x00000000000000ffULL)) << 56) |
           ((v & static_cast<uint64_t>(0x000000000000ff00ULL)) << 40) |
           ((v & static_cast<uint64_t>(0x0000000000ff0000ULL)) << 24) |
           ((v & static_cast<uint64_t>(0x00000000ff000000ULL)) << 8) |
           ((v & static_cast<uint64_t>(0x000000ff00000000ULL)) >> 8) |
           ((v & static_cast<uint64_t>(0x0000ff0000000000ULL)) >> 24) |
           ((v & static_cast<uint64_t>(0x00ff000000000000ULL)) >> 40) |
           ((v & static_cast<uint64_t>(0xff00000000000000ULL)) >> 56);
}

inline uint32_t Reverse32(uint32_t v)
{
    v = ((v & 0x55555555) << 1) | ((v & 0xaaaaaaaa) >> 1);
    v = ((v & 0x33333333) << 2) | ((v & 0xcccccccc) >> 2);
    v = ((v & 0x0f0f0f0f) << 4) | ((v & 0xf0f0f0f0) >> 4);
    v = ((v & 0x00ff00ff) << 8) | ((v & 0xff00ff00) >> 8);
    v = ((v & 0x0000ffff) << 16) | ((v & 0xffff0000) >> 16);

    return v;
}

#define BitVectorBytes(x) static_cast<uint8_t>(((x) + (CHAR_BIT - 1)) / CHAR_BIT)

namespace BigEndian {

#if BYTE_ORDER_BIG_ENDIAN

inline uint16_t HostSwap16(uint16_t v)
{
    return v;
}
inline uint32_t HostSwap32(uint32_t v)
{
    return v;
}
inline uint64_t HostSwap64(uint64_t v)
{
    return v;
}

#else /* BYTE_ORDER_LITTLE_ENDIAN */

inline uint16_t HostSwap16(uint16_t v)
{
    return Swap16(v);
}
inline uint32_t HostSwap32(uint32_t v)
{
    return Swap32(v);
}
inline uint64_t HostSwap64(uint64_t v)
{
    return Swap64(v);
}

#endif // LITTLE_ENDIAN

/**
 * This function reads a `uint16_t` value from a given buffer assuming big-ending encoding.
 *
 * @param[in] aBuffer   Pointer to buffer to read from.
 *
 * @returns The `uint16_t` value read from buffer.
 *
 */
inline uint16_t ReadUint16(const uint8_t *aBuffer)
{
    return static_cast<uint16_t>((aBuffer[0] << 8) | aBuffer[1]);
}

/**
 * This function reads a `uint32_t` value from a given buffer assuming big-ending encoding.
 *
 * @param[in] aBuffer   Pointer to buffer to read from.
 *
 * @returns The `uint32_t` value read from buffer.
 *
 */
inline uint32_t ReadUint32(const uint8_t *aBuffer)
{
    return ((static_cast<uint32_t>(aBuffer[0]) << 24) | (static_cast<uint32_t>(aBuffer[1]) << 16) |
            (static_cast<uint32_t>(aBuffer[2]) << 8) | (static_cast<uint32_t>(aBuffer[3]) << 0));
}

/**
 * This function writes a `uint16_t` value to a given buffer using big-ending encoding.
 *
 * @param[in]  aValue    The value to write to buffer.
 * @param[out] aBuffer   Pointer to buffer where the value will be written.
 *
 */
inline void WriteUint16(uint16_t aValue, uint8_t *aBuffer)
{
    aBuffer[0] = (aValue >> 8) & 0xff;
    aBuffer[1] = (aValue >> 0) & 0xff;
}

/**
 * This function writes a `uint32_t` value to a given buffer using big-ending encoding.
 *
 * @param[in]  aValue    The value to write to buffer.
 * @param[out] aBuffer   Pointer to buffer where the value will be written.
 *
 */
inline void WriteUint32(uint32_t aValue, uint8_t *aBuffer)
{
    aBuffer[0] = (aValue >> 24) & 0xff;
    aBuffer[1] = (aValue >> 16) & 0xff;
    aBuffer[2] = (aValue >> 8) & 0xff;
    aBuffer[3] = (aValue >> 0) & 0xff;
}

} // namespace BigEndian

namespace LittleEndian {

#if BYTE_ORDER_BIG_ENDIAN

inline uint16_t HostSwap16(uint16_t v)
{
    return Swap16(v);
}
inline uint32_t HostSwap32(uint32_t v)
{
    return Swap32(v);
}
inline uint64_t HostSwap64(uint64_t v)
{
    return Swap64(v);
}

#else /* BYTE_ORDER_LITTLE_ENDIAN */

inline uint16_t HostSwap16(uint16_t v)
{
    return v;
}
inline uint32_t HostSwap32(uint32_t v)
{
    return v;
}
inline uint64_t HostSwap64(uint64_t v)
{
    return v;
}

#endif

/**
 * This function reads a `uint16_t` value from a given buffer assuming little-ending encoding.
 *
 * @param[in] aBuffer   Pointer to buffer to read from.
 *
 * @returns The `uint16_t` value read from buffer.
 *
 */
inline uint16_t ReadUint16(const uint8_t *aBuffer)
{
    return static_cast<uint16_t>(aBuffer[0] | (aBuffer[1] << 8));
}

/**
 * This function reads a `uint32_t` value from a given buffer assuming little-ending encoding.
 *
 * @param[in] aBuffer   Pointer to buffer to read from.
 *
 * @returns The `uint32_t` value read from buffer.
 *
 */
inline uint32_t ReadUint32(const uint8_t *aBuffer)
{
    return ((static_cast<uint32_t>(aBuffer[0]) << 0) | (static_cast<uint32_t>(aBuffer[1]) << 8) |
            (static_cast<uint32_t>(aBuffer[2]) << 16) | (static_cast<uint32_t>(aBuffer[3]) << 24));
}

/**
 * This function writes a `uint16_t` value to a given buffer using little-ending encoding.
 *
 * @param[in]  aValue    The value to write to buffer.
 * @param[out] aBuffer   Pointer to buffer where the value will be written.
 *
 */
inline void WriteUint16(uint16_t aValue, uint8_t *aBuffer)
{
    aBuffer[0] = (aValue >> 0) & 0xff;
    aBuffer[1] = (aValue >> 8) & 0xff;
}

/**
 * This function writes a `uint32_t` value to a given buffer using little-ending encoding.
 *
 * @param[in]  aValue   The value to write to buffer.
 * @param[out] aBuffer  Pointer to buffer where the value will be written.
 *
 */
inline void WriteUint32(uint32_t aValue, uint8_t *aBuffer)
{
    aBuffer[0] = (aValue >> 0) & 0xff;
    aBuffer[1] = (aValue >> 8) & 0xff;
    aBuffer[2] = (aValue >> 16) & 0xff;
    aBuffer[3] = (aValue >> 24) & 0xff;
}

} // namespace LittleEndian
} // namespace Encoding








inline uint32_t Swap32(uint32_t v)
{
    return ((v & static_cast<uint32_t>(0x000000ffUL)) << 24) | ((v & static_cast<uint32_t>(0x0000ff00UL)) << 8) |
           ((v & static_cast<uint32_t>(0x00ff0000UL)) >> 8) | ((v & static_cast<uint32_t>(0xff000000UL)) >> 24);
}


typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

        static constexpr auto IPV6_ADDR_SIZE = 16; // Bytes

        union fields_t {
            uint8_t m8[IPV6_ADDR_SIZE];                      // 8-bit
            uint16_t m16[IPV6_ADDR_SIZE / sizeof(uint16_t)]; // 16-bit
            uint32_t m32[IPV6_ADDR_SIZE / sizeof(uint32_t)]; // 32-bit
            uint64_t m64[IPV6_ADDR_SIZE / sizeof(uint64_t)]; // 64-bit
        };

int main() {
	using namespace std;

  cout << sizeof(uint8_t) << endl;
  cout << sizeof(uint16_t) << endl;
  cout << sizeof(uint32_t) << endl;
  cout << sizeof(uint64_t) << endl;

  
  cout << Swap32(1) << endl;

#if __BYTE_ORDER == __LITTLE_ENDIAN
  cout << "little endian" << endl;
#else
    cout << "big endian" << endl;
#endif

int n = 1;
// little endian if true
if(*(char *)&n == 1) {
    cout << "little endian" << endl;
} else {
    cout << "big endian" << endl;
}


  cout << sizeof(uint128_t) << endl;


  fields_t fields;
  fields.m64[0] = 1;
  fields.m64[1] = 0;

  cout << boolalpha << (fields.m8[0] == 1) << endl;
  cout << 0xff020000 << endl;
  cout << Encoding::BigEndian::HostSwap32(0xff020000) << endl;
  cout << Encoding::LittleEndian::HostSwap32(0xff020000) << endl;

	return 0;
}


