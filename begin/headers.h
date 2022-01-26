#include <string_view>
#include <map>
#include <string>
#include <memory>
#include "cookies.h"

namespace webpp {

    enum class header_type { RESPONSE, REQUEST };
    /**
     * The classes header and body are the "owners of data".
     * That means these classes will own (use std::string/... and not references
     * and pointers) the data they have.
     *
     * And they also use other "views of data" classes like cookie classes so
     * they can provide shaped and meaningful data to the developers.
     *
     * This class will only contain what's the same in both request and response
     * classes; the rest, is up to those classes.
     */
    template <typename Interface,
              header_type HeaderType = header_type::RESPONSE>
    class headers {
      private:
        using headers_t = std::multimap<std::string, std::string>;

        std::shared_ptr<Interface> interface;
        mutable headers_t data;
        mutable webpp::cookie_jar _cookies;
      public:
        // TODO: consider using custom iterators instead of multimap's

        using iterator = headers_t::iterator;
        using const_iterator = headers_t ::const_iterator;
        using reverse_iterator = headers_t::reverse_iterator;
        using const_reverse_iterator = headers_t::const_reverse_iterator;

        iterator begin() noexcept { return data.begin(); }

        const_iterator begin() const noexcept { return data.begin(); }

        const_iterator cbegin() const noexcept { return data.cbegin(); }

        iterator end() noexcept { return data.end(); }

        const_iterator end() const noexcept { return data.end(); }

        const_iterator cend() const noexcept { return data.cend(); }

        reverse_iterator rbegin() noexcept { return data.rbegin(); }

        const_reverse_iterator rbegin() const noexcept { return data.rbegin(); }

        reverse_iterator rend() noexcept { return data.rend(); }

        const_reverse_iterator rend() const noexcept { return data.rend(); }

        const_reverse_iterator crbegin() const noexcept {
            return data.crbegin();
        }

        const_reverse_iterator crend() const noexcept { return data.crend(); }
    };





} // namespace webpp



