import standard;

#include <array>
#include <cstdint>
#include <iterator>
#include <print>
#include <set>
#include <string>
#include <type_traits>

namespace stl {
using namespace std;
}
template <typename T>
concept UTF8 = (sizeof(T) == sizeof(char8_t));

template <typename T>
concept UTF16 = (sizeof(T) == sizeof(char16_t));

template <typename T>
concept WChar = (sizeof(T) >= sizeof(char32_t));

template <typename T>
concept UTF32 = WChar<T>;

template <typename D>
concept Destructible = requires(D obj) {
  { obj.~D() } noexcept;
};

template <typename T>
concept MoveAssignable = stl::is_move_assignable_v<T>;

template <typename T>
concept CopyAssignable = stl::is_copy_assignable_v<T>;

template <typename T>
concept CopyConstructible = stl::is_copy_constructible_v<T>;

template <typename T>
concept DefaultConstructible = stl::is_default_constructible_v<T>;

/// Basic Multilingual Plane (BMP)
template <typename u32 = char32_t> static constexpr u32 max_bmp = 0x0000'FFFF;

// Leading (high) surrogates: 0xd800 - 0xdbff
// Trailing (low) surrogates: 0xdc00 - 0xdfff
template <typename u16 = char16_t>
static constexpr u16 lead_surrogate_min = 0xD800;

/*
 * Check whether a Unicode (5.2) char is in a valid range.
 *
 * The first check comes from the Unicode guarantee to never encode
 * a point above 0x0010ffff, since UTF-16 couldn't represent it.
 *
 * The second check covers surrogate pairs (category Cs).
 */
template <typename u32>
[[nodiscard]] static constexpr bool
is_code_point_valid(u32 code_point) noexcept {
  using uu32 = stl::make_unsigned_t<u32>;
  return code_point < static_cast<u32>(0x11'0000U) &&
         ((static_cast<uu32>(code_point) & 0xFFFF'F800U) !=
          lead_surrogate_min<uu32>);
  // alternative implementation:
  // return (cp <= max_legal_utf32<u32> && !is_surrogate(cp));
}
namespace istl {

template <typename T, typename DefaultType = stl::size_t> struct size_type_of {
  using type = DefaultType;
};

template <typename T, typename DefaultType>
  requires requires { typename T::size_type; }
struct size_type_of<T, DefaultType> {
  using type = typename T::size_type;
};

/// Get the default size of the specified type or the default type that you
/// specify
template <typename T, typename DefaultType = stl::size_t>
using size_type_of_t = typename size_type_of<T, DefaultType>::type;

/// The simpler version of the above code, with the difference that can't
/// be used as concept for return types
template <typename T, typename... E>
concept part_of = (stl::same_as<T, E> || ...);

template <typename X>
concept CharTraits = requires {
  typename X::char_type;
  typename X::int_type;
  typename X::off_type;
  typename X::pos_type;
  typename X::state_type;
  requires requires(typename X::char_type c, typename X::char_type const *p,
                    typename X::char_type *s, stl::size_t n,
                    typename X::int_type e, typename X::char_type const &ch) {
    requires CopyAssignable<typename X::state_type>;
    requires Destructible<typename X::state_type>;
    requires CopyConstructible<typename X::state_type>;
    requires DefaultConstructible<typename X::state_type>;

    { X::eq(c, c) } -> stl::same_as<bool>;
    { X::lt(c, c) } -> stl::same_as<bool>;
    { X::compare(p, p, n) } -> stl::same_as<int>;
    { X::length(p) } -> stl::same_as<stl::size_t>;
    { X::find(p, n, ch) } -> stl::same_as<typename X::char_type const *>;
    { X::move(s, p, ch) } -> stl::same_as<typename X::char_type *>;
    { X::copy(s, p, n) } -> stl::same_as<typename X::char_type *>;
    { X::assign(s, n, c) } -> stl::same_as<typename X::char_type *>;
    { X::not_eof(e) } -> stl::same_as<typename X::int_type>;
    { X::to_char_type(e) } -> stl::same_as<typename X::char_type>;
    { X::to_int_type(c) } -> stl::same_as<typename X::int_type>;
    { X::eq_int_type(e, e) } -> stl::same_as<bool>;
    { X::eof() } -> stl::same_as<typename X::int_type>;
  };
};

template <typename T>
concept SingleCharacter =
    part_of<T, unsigned char, char, wchar_t, char16_t, char8_t, char32_t>;

template <typename T>
concept CharType = istl::SingleCharacter<stl::remove_cvref_t<T>>;

namespace details {
template <typename T> struct char_type_of {
  using type = void;
};

template <typename T>
  requires requires {
    typename T::value_type;
    requires stl::is_trivial_v<typename T::value_type>;
    requires stl::is_standard_layout_v<typename T::value_type>;
  }
struct char_type_of<T> {
  using type = typename T::value_type;
};

template <SingleCharacter T> struct char_type_of<T> {
  using type = T;
};

template <typename T> struct traits_extractor {
  using type = typename T::traits_type;
};

template <typename T> struct allocator_type_extractor {
  using type = typename T::allocator_type;
};

template <typename T>
concept has_allocator_type =
    requires { typename stl::remove_cvref_t<T>::allocator_type; };
} // namespace details

/**
 * Get the underlying character type in a string/string view/c style string
 */
template <typename T>
struct char_type_of
    : details::char_type_of<
          stl::decay_t<stl::remove_pointer_t<stl::decay_t<T>>>> {};

template <typename T> using char_type_of_t = typename char_type_of<T>::type;

template <typename T>
concept Iterable = requires(T iter) {
  { stl::begin(iter) } -> stl::input_iterator;
  { stl::end(iter) } -> stl::input_iterator;
};

template <typename T>
concept NothrowIterable = requires(T iter) {
  { stl::begin(iter) } noexcept -> stl::input_iterator;
  { stl::end(iter) } noexcept -> stl::input_iterator;
};

template <typename T>
using iterator_type_of = stl::remove_cvref_t<decltype(stl::begin(
    stl::declval<stl::remove_pointer_t<stl::remove_cvref_t<T>>>()))>;

template <typename T> struct appendable_value_type {
  using type =
      istl::char_type_of_t<stl::remove_pointer_t<stl::remove_cvref_t<T>>>;
};

template <typename T>
  requires(stl::forward_iterator<stl::remove_cvref_t<T>> &&
           !requires {
             typename stl::remove_pointer_t<stl::remove_cvref_t<T>>::value_type;
           })
struct appendable_value_type<T> {
  using type =
      typename stl::iterator_traits<stl::remove_cvref_t<T>>::value_type;
};

template <typename T>
using appendable_value_type_t = typename appendable_value_type<T>::type;

template <typename Iter, typename ValueType = appendable_value_type_t<Iter>>
concept AppendableIterator =
    stl::forward_iterator<Iter> && stl::is_copy_assignable_v<ValueType> &&
    !stl::is_const_v<ValueType>;

template <typename Iter, typename ValueType = appendable_value_type_t<Iter>>
concept NothrowAppendableIterator =
    AppendableIterator<Iter, ValueType> &&
    stl::is_nothrow_copy_assignable_v<ValueType>;

template <typename T, typename ValueType = appendable_value_type_t<T>>
concept AppendableString =
    !stl::is_const_v<stl::remove_pointer_t<stl::remove_cvref_t<T>>> &&
    !stl::is_const_v<ValueType> &&
    requires(stl::remove_pointer_t<stl::remove_cvref_t<T>> &out,
             ValueType val) { out.operator+=(val); };

template <typename T, typename ValueType = appendable_value_type_t<T>>
concept NothrowAppendableArray = AppendableString<T, ValueType> &&
                                 stl::is_nothrow_copy_assignable_v<ValueType>;

template <typename T, typename ValueType = appendable_value_type_t<T>>
concept Appendable = AppendableString<T, ValueType> ||
                     AppendableIterator<stl::remove_pointer_t<T>, ValueType>;

template <typename T, typename ValueType = appendable_value_type_t<T>>
concept AppendableStorage =
    Appendable<T, ValueType> ||
    (Iterable<T> &&
     Appendable<
         iterator_type_of<stl::remove_pointer_t<stl::remove_cvref_t<T>>>>);

template <typename T, typename ValueType = appendable_value_type_t<T>>
concept NothrowAppendable =
    Appendable<T, ValueType> && (NothrowAppendableIterator<T, ValueType> ||
                                 NothrowAppendableArray<T, ValueType>);

template <typename T, typename ValueType = appendable_value_type_t<T>>
concept NothrowAppendableStorage =
    AppendableStorage<T, ValueType> &&
    NothrowIterable<
        iterator_type_of<stl::remove_pointer_t<stl::remove_cvref_t<T>>>> &&
    NothrowAppendable<
        iterator_type_of<stl::remove_pointer_t<stl::remove_cvref_t<T>>>,
        ValueType>;

/**
 * Append
 * todo: add vectors, sets, ...
 * @tparam T can be an array, iterator, string, or similar types that can be
 * appended
 * @tparam ValueType are the values
 * @param out the output type that you want values to be appended to
 * @param value the values that you want to append
 */
template <Appendable T, typename... ValueType>
static constexpr void
iter_append(T &out, ValueType... value) noexcept(NothrowAppendable<T>) {
  using char_type = appendable_value_type_t<T>;
  if constexpr (AppendableString<T, char_type>) {
    if constexpr (stl::is_pointer_v<T>) {
      ((*out += static_cast<char_type>(value)), ...);
    } else {
      ((out += static_cast<char_type>(value)), ...);
    }
  } else {
    // pointer or an iterator
    ((*(out++) = static_cast<char_type>(value)), ...);
  }
}

/**
 * Get an appendable object out of the specified argument
 *
 */
template <AppendableStorage IterOrStorage>
[[nodiscard]] constexpr decltype(auto)
appendable_iter_of(IterOrStorage &obj) noexcept {
  if constexpr (Appendable<IterOrStorage>) {
    // It's either an iterator itself, or it's appendable itself
    return stl::addressof(obj);
  } else {
    // It's iterable
    return stl::begin(obj);
  }
}

} // namespace istl

using namespace std;

// From https://www.unicode.org/versions/Unicode15.1.0/ch03.pdf#G56669

/// SBase in the standard:
static constexpr auto hangul_syllable_base = 0xAC00UL;

/// LBase in the standard:
static constexpr auto hangul_leading_base = 0x1100UL;

/// VBase in the standard:
static constexpr auto hangul_vowel_base = 0x1161UL;

/// TBase in the standard:
static constexpr auto hangul_trailing_base = 0x11A7UL;

/// LCount in the standard:
static constexpr auto hangul_leading_count = 19UL;

/// VCount in the standard:
static constexpr auto hangul_vowel_count = 21UL;

/// TCount in the standard:
static constexpr auto hangul_trailing_count = 28UL;

/// Total count of Hangul blocks and syllables
/// NCount in the standard:
static constexpr auto hangul_block_count =
    hangul_vowel_count * hangul_trailing_count;

namespace unchecked {
/**
 * Append a code point to a string
 * "out" can be an iterator/pointer or a string
 */
template <istl::Appendable StrT,
          stl::integral SizeT = istl::size_type_of_t<StrT>,
          typename CharT = char32_t>
static constexpr SizeT
append(StrT &out, CharT code_point) noexcept(istl::NothrowAppendable<StrT>) {
  using istl::iter_append;

  using char_type = istl::appendable_value_type_t<StrT>;
  using uchar_t = stl::make_unsigned_t<CharT>;
  auto const ccp = static_cast<stl::uint32_t>(code_point);
  if constexpr (UTF8<char_type>) {
    if (ccp < 0x80U) { // one octet
      iter_append(out, ccp);
      return 1U;
    }
    if (ccp < 0x800) {                         // two octets
      iter_append(out, (ccp >> 6U) | 0xC0U);   // 0b110,'....
      iter_append(out, (ccp & 0x3FU) | 0x80U); // 0b10..'....
      return 2U;
    }
    if (ccp < 0x1'0000U) {                             // three octets
      iter_append(out, (ccp >> 12U) | 0xE0U);          // 0b1110'....
      iter_append(out, ((ccp >> 6U) & 0x3FU) | 0x80U); // 0b10..'....
      iter_append(out, (ccp & 0x3FU) | 0x80U);         // 0b10..'....
      return 3U;
    }
    // four octets
    iter_append(out, (ccp >> 18U) | 0xF0U);           // 0b1111'0...
    iter_append(out, ((ccp >> 12U) & 0x3FU) | 0x80U); // 0b10..'....
    iter_append(out, ((ccp >> 6U) & 0x3FU) | 0x80U);  // 0b10..'....
    iter_append(out, (ccp & 0x3FU) | 0x80U);          // 0b10..'....
    return 4U;
  } else if constexpr (UTF16<char_type>) {
    if (ccp <= max_bmp<char_type>) {
      iter_append(out, ccp); // normal case
      return 1U;
    }
    iter_append(out, 0xD7C0U + (static_cast<uchar_t>(ccp) >> 10U));
    iter_append(out, 0xDC00U + (static_cast<uchar_t>(ccp) & 0x3FFU));
    return 2U;
  } else { // for char32_t or others
    iter_append(out, ccp);
    return 1U;
  }
}

template <istl::AppendableStorage StrT = std::array<char8_t, 4UL>,
          typename CharT = char32_t, typename... Args>
[[nodiscard]] static constexpr StrT
to(CharT const code_point,
   Args &&...args) noexcept(istl::NothrowAppendable<StrT>) {
  StrT str{stl::forward<Args>(args)...};
  auto iter = istl::appendable_iter_of(str);
  append(iter, code_point);
  return str;
}

} // namespace unchecked

namespace checked {

template <typename Ptr, typename CharT = char32_t>
[[nodiscard("Use unicode::unchecked::append if the input codepoint is always "
            "valid.")]] static constexpr bool
append(Ptr &out, CharT code_point) noexcept {
  if (!is_code_point_valid(code_point)) {
    return false;
  }
  unchecked::append<Ptr, CharT>(out, code_point);
  return true;
}

} // namespace checked

/**
 * Get the required length of code points needed if we were to decompose the
 * specified code point.
 *
 * Attention: the returned length is UTF-32 and is not in UTF-8.
 * Attention: the code point MUST be a valid Hangul code point.
 */
template <typename CharT = char32_t, stl::unsigned_integral RetT = stl::size_t>
[[nodiscard]] static constexpr RetT
hangul_decompose_length(CharT const code_point) noexcept {
  if ((code_point - hangul_syllable_base) % hangul_trailing_count) {
    return 3U;
  }
  return 2U;
}

template <typename CharT = char32_t> struct decomposed_hangul {
  static constexpr CharT invalid_trailing = hangul_trailing_base;

  CharT leading;
  CharT vowel;
  CharT trailing = invalid_trailing;
};

/**
 * Decompose the Hangul code point and return its parts.
 * This function does not check if the inputted code point is Hangul or not.
 * https://www.unicode.org/versions/Unicode15.1.0/ch03.pdf#G56669
 *
 * @returns decomposed_hangul which contains all the parts of a hangul
 * decomposed code point
 */
template <istl::CharType CharT = char32_t>
[[nodiscard]] static constexpr decomposed_hangul<CharT>
decomposed_hangul_code_point(CharT const code_point) noexcept {
  auto const pos = code_point - hangul_syllable_base;

  // Calculating the indices:
  auto const leading_pos = pos / hangul_block_count;
  auto const vowel_pos = (pos % hangul_block_count) / hangul_trailing_count;
  auto const trailing_pos = pos % hangul_trailing_count;

  // Calculating the values:
  return decomposed_hangul<CharT>{
      .leading = static_cast<CharT>(hangul_leading_base + leading_pos),
      .vowel = static_cast<CharT>(hangul_vowel_base + vowel_pos),
      .trailing = static_cast<CharT>(
          trailing_pos != 0 ? hangul_trailing_base + trailing_pos
                            : decomposed_hangul<CharT>::invalid_trailing),
  };
}

/**
 * Append the decomposed Hangul code point to the output
 * This function does not check if the inputted code point is Hangul or not.
 *
 * @tparam CharT     char type
 * @tparam StrOrIter Can be a string/string-view/iterator/vector<CharT>/...
 */
template <istl::Appendable StrOrIter,
          stl::unsigned_integral SizeT = istl::size_type_of_t<StrOrIter>,
          istl::CharType CharT = char32_t>
static constexpr SizeT decompose_hangul_code_point(
    StrOrIter &out,
    CharT const code_point) noexcept(istl::NothrowAppendable<StrOrIter>) {
  using unchecked::append;
  auto const [leading, vowel, trailing] =
      decomposed_hangul_code_point(code_point);

  SizeT count = 0;
  count += append(out, leading);
  count += append(out, vowel);
  if (trailing != decomposed_hangul<CharT>::invalid_trailing) {
    count += append(out, trailing);
  }
  return count;
}

/// SCount in the standard:
static constexpr auto hangul_syllable_count =
    hangul_leading_count * hangul_block_count;

std::string utf32_to_utf8(std::u32string const &utf32_str) {
  std::string utf8_str;
  utf8_str.reserve(utf32_str.length() *
                   4); // Estimate maximum size of UTF-8 string

  for (char32_t const code_point : utf32_str) {
    if (!checked::append(utf8_str, code_point)) {
      throw stl::invalid_argument("Invalid code point");
    }
  }

  return utf8_str;
}

int main() {

  std::set<size_t> sizes;

  for (uint32_t index = hangul_syllable_base;
       index != (hangul_syllable_base + hangul_syllable_count); ++index) {
    auto const code_point = static_cast<char32_t>(index);
    array<char32_t, 2> arr{};
    arr[0] = code_point;
    auto const utf8 = utf32_to_utf8(stl::u32string{arr.data(), 1});
    auto const len = utf8.size();

    std::u32string out;
    auto const u32_len = decompose_hangul_code_point(out, code_point);

    auto converted = utf32_to_utf8(out);

    println("{} {} {} {}, out: {} {}; converted: {}", index, utf8, len,
            hangul_decompose_length(index), out.size(), u32_len,
            converted.size());
    sizes.insert(converted.size());
  }

  for (auto const len : sizes) {
    println("{}", len);
  }

  return 0;
}
