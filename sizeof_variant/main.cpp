#include <iostream>
#include <string>
#include <string_view>
#include <variant>
#include <array>
#include <optional>


       template <typename T>
        struct uri_segments {

            /**
             * This is the "scheme" element of the URI.
             */
            T scheme;

            /**
             * This is the "UserInfo" element of the URI.
             */
            T user_info;

            /**
             * This is the "host" element of the URI.
             */
            T host;

            /**
             * This is the port number element of the URI.
             */
            T port;

            /**
             * This is the "path" element of the URI,
             * as a sequence of segments.
             */
            T path;

            /**
             * This is the "query" element of the URI,
             * if it has one.
             */
            T query;

            /**
             * This is the "fragment" element of the URI,
             * if it has one.
             */
            T fragment;
        };

        using uri_data_t =
            std::variant<std::string_view, uri_segments<std::string_view>,
                         uri_segments<std::string>>;



struct new_design {
	std::string own;
	std::string_view scheme, user_info, host, port, path, query, fragment;
};

struct new_new_design {
	std::string own;
	std::string scheme, user_info, host, port, path, query, fragment;
};
struct new_new_new_design {
	std::string own;
	uint8_t t;
	std::string_view scheme, user_info, host, port, path, query, fragment;
};

struct new_new_new_new_design {
	std::string own;
	std::size_t a[7];

	constexpr static auto scheme = 0;
	constexpr static auto user_info = 1;
};
struct new_new_new_new_new_design {
	std::string own;
	std::array<std::size_t, 7> a;

	constexpr static auto scheme = 0;
	constexpr static auto user_info = 1;
};
struct modern_design {
	std::string own;
	std::size_t scheme_end, authority_start, user_info_end, port_start,
		port_end, query_start, fragment_start;
};

using namespace std;


struct optional_design {
	optional<uint32_t> a;
};

struct bool_design {
	uint32_t a;
	bool b;
};

struct bool_design2 {
	uint32_t a;
	uint8_t b;
	bool c;
};

struct variant_design {
	variant<std::string_view, uint32_t> a;
};


        using octets8_t = std::array<uint8_t, 16u>;
        using octets16_t = std::array<uint16_t, 8u>;
        using octets32_t = std::array<uint32_t, 4u>;
        using octets64_t = std::array<uint64_t, 2u>;
        using octets_t = octets8_t;

    class ipv6 {
      public:
        mutable std::variant<std::string_view, octets_t> data;
	uint8_t prefix;
    };

    class ipv62 {
      public:
        mutable octets_t data;
	uint8_t prefix;
    };


auto main() -> int {

	std::string hello = "this is a a long long long string";

	

  cout << "string: " << sizeof(std::string) << endl
	  << "string hello: " << sizeof(hello) << endl
	  << "string hello data: " << (hello.size() * sizeof(char)) << endl
	  << "string_view: " << sizeof(std::string_view) << endl
	  << "uri_segments<string_view>: " << sizeof(uri_segments<std::string_view>) << endl
	  << "uri_segments<string>: " << sizeof(uri_segments<std::string>) << endl
	  << "uri_data_t: " << sizeof(uri_data_t) << endl
	  << "new_design: " << sizeof(new_design) << endl
	  << "new_new_design: " << sizeof(new_new_design) << endl
	  << "new_new_new_design: " << sizeof(new_new_new_design) << endl
	  << "new_new_new_new_design: " << sizeof(new_new_new_new_design) << endl
	  << "new_new_new_new_new_design: " << sizeof(new_new_new_new_new_design) << endl
	  << "modern_design: " << sizeof(modern_design) << endl
	  << "new_design padding: " << (sizeof(new_design) - (sizeof(string) + (7 * sizeof(string_view)))) << endl
	  << "new_new_design padding: " << (sizeof(new_new_design) - (sizeof(string) + (7 * sizeof(string)))) << endl
	  << "new_new_new_design padding: " << (sizeof(new_new_new_design) - (sizeof(string) + (7 * sizeof(string_view)) +  sizeof(uint8_t))) << endl
	  << "new_new_new_new_design padding: " << (sizeof(new_new_new_new_design) - (sizeof(string) + (7 * sizeof(std::size_t)) )) << endl
	  << "new_new_new_new_new_design padding: " << (sizeof(new_new_new_new_new_design) - (sizeof(string) + (7 * sizeof(std::size_t)) )) << endl;



	cout << "---------------------------------" << endl;

	cout << "uint32_t: " << sizeof(uint32_t) << endl <<
		"optional<uint32_t>: " << sizeof(optional<uint32_t>) << endl <<
		"optional_design: " << sizeof(optional_design) << endl << 
		"bool_design: " << sizeof(bool_design) << endl <<
		"bool_design2: " << sizeof(bool_design2) << endl <<
		"variant_design: " << sizeof(variant_design) << endl <<
		"padding optional_design: " << sizeof(optional_design) - (sizeof(optional<uint32_t>)) << endl <<
		"padding variant_design: " << sizeof(variant_design) - (sizeof(variant<std::string_view, uint32_t>)) << endl <<
		"padding bool_design: " << sizeof(bool_design) - (sizeof(uint32_t) + sizeof(bool)) << endl << 
		"padding bool_design2: " << sizeof(bool_design2) - (sizeof(uint32_t) + sizeof(uint8_t) + sizeof(bool)) << endl;



	cout << "-----------------------------------" << endl;


	cout << "ipv6: " << sizeof(ipv6) << endl <<
		"ipv6 2: " << sizeof(ipv62) << endl <<
		"octets: " << sizeof(octets_t) << endl << 
		"ipv6 padding: " << sizeof(ipv6) - (sizeof(decltype(ipv6::data)) + sizeof(uint8_t)) << endl << 
		"ipv6 2 padding: " << sizeof(ipv62) - (sizeof(octets_t) + sizeof(uint8_t)) << endl;

	

  return 0;
}
