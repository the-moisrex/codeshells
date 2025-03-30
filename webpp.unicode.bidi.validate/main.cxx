import standard;

#include <print>
#include <webpp/unicode/bidi.hpp>

using namespace webpp;
using namespace std;

int main() {

    u32string_view const invalid_computer_word =
        U"\u0786" // THAANA LETTER KAAFU (AL)
        U"\u07AE" // THAANA OBOFILI (NSM)
        U"\u0782" // THAANA LETTER NOONU (AL)
        U"\u07B0" // THAANA SUKUN (NSM)
        U"\u0795" // THAANA LETTER PAVIYANI (AL)
        U"\u07A9" // THAANA LETTER EEBEEFILI (AL)
        U"\u0793" // THAANA LETTER TAVIYANI (AL)
        U"\u07A6" // THAANA ABAFILI (NSM)
        U"\u0783" // THAANA LETTER RAA (AL)
        U"\u07AA" // THAANA UBUFILI (NSM)
        U"\u0294" // 0294          ; L # Lo       LATIN LETTER GLOTTAL STOP
        U"\u07AA" // THAANA UBUFILI (NSM)
        ;

    bool res = unicode::validate_bidi_rule(invalid_computer_word.begin(),
                                           invalid_computer_word.end());

    if (res) {
        println("Yes");
    } else {
        println("No");
    }

    return 0;
}
