#include <iostream>

using namespace std;

        template <typename CharT = char>
        static constexpr CharT percent_hex_table[1024] =
          "%00\0%01\0%02\0%03\0%04\0%05\0%06\0%07\0"
          "%08\0%09\0%0A\0%0B\0%0C\0%0D\0%0E\0%0F\0"
          "%10\0%11\0%12\0%13\0%14\0%15\0%16\0%17\0"
          "%18\0%19\0%1A\0%1B\0%1C\0%1D\0%1E\0%1F\0"
          "%20\0%21\0%22\0%23\0%24\0%25\0%26\0%27\0"
          "%28\0%29\0%2A\0%2B\0%2C\0%2D\0%2E\0%2F\0"
          "%30\0%31\0%32\0%33\0%34\0%35\0%36\0%37\0"
          "%38\0%39\0%3A\0%3B\0%3C\0%3D\0%3E\0%3F\0"
          "%40\0%41\0%42\0%43\0%44\0%45\0%46\0%47\0"
          "%48\0%49\0%4A\0%4B\0%4C\0%4D\0%4E\0%4F\0"
          "%50\0%51\0%52\0%53\0%54\0%55\0%56\0%57\0"
          "%58\0%59\0%5A\0%5B\0%5C\0%5D\0%5E\0%5F\0"
          "%60\0%61\0%62\0%63\0%64\0%65\0%66\0%67\0"
          "%68\0%69\0%6A\0%6B\0%6C\0%6D\0%6E\0%6F\0"
          "%70\0%71\0%72\0%73\0%74\0%75\0%76\0%77\0"
          "%78\0%79\0%7A\0%7B\0%7C\0%7D\0%7E\0%7F\0"
          "%80\0%81\0%82\0%83\0%84\0%85\0%86\0%87\0"
          "%88\0%89\0%8A\0%8B\0%8C\0%8D\0%8E\0%8F\0"
          "%90\0%91\0%92\0%93\0%94\0%95\0%96\0%97\0"
          "%98\0%99\0%9A\0%9B\0%9C\0%9D\0%9E\0%9F\0"
          "%A0\0%A1\0%A2\0%A3\0%A4\0%A5\0%A6\0%A7\0"
          "%A8\0%A9\0%AA\0%AB\0%AC\0%AD\0%AE\0%AF\0"
          "%B0\0%B1\0%B2\0%B3\0%B4\0%B5\0%B6\0%B7\0"
          "%B8\0%B9\0%BA\0%BB\0%BC\0%BD\0%BE\0%BF\0"
          "%C0\0%C1\0%C2\0%C3\0%C4\0%C5\0%C6\0%C7\0"
          "%C8\0%C9\0%CA\0%CB\0%CC\0%CD\0%CE\0%CF\0"
          "%D0\0%D1\0%D2\0%D3\0%D4\0%D5\0%D6\0%D7\0"
          "%D8\0%D9\0%DA\0%DB\0%DC\0%DD\0%DE\0%DF\0"
          "%E0\0%E1\0%E2\0%E3\0%E4\0%E5\0%E6\0%E7\0"
          "%E8\0%E9\0%EA\0%EB\0%EC\0%ED\0%EE\0%EF\0"
          "%F0\0%F1\0%F2\0%F3\0%F4\0%F5\0%F6\0%F7\0"
          "%F8\0%F9\0%FA\0%FB\0%FC\0%FD\0%FE\0%FF";


auto main() -> int {

    for (std::size_t index = 0; index != 1024; ++index)  {
        char ch = percent_hex_table<char>[index];
        if (ch == '\0') {
            cout << "0, ";
        } else {
        cout << "'" << ch << "', ";
        }
    }
    cout << endl;

  return 0;
}
