#include <array>
#include <benchmark/benchmark.h>
#include <numeric>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <new>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

using namespace std;

namespace v1 {
template <class R>
  requires std::ranges::input_range<R> &&
           std::convertible_to<std::ranges::range_reference_t<R>,
                               std::string_view>
class string_views {
  using base_view = std::views::all_t<R>;

  static std::string_view
  to_view(std::ranges::range_reference_t<R> s) noexcept {
    return std::string_view{s};
  }

  using transform = decltype(&to_view);
  using view_type = std::ranges::transform_view<base_view, transform>;

  view_type view_;

public:
  template <class U>
    requires std::constructible_from<base_view, U>
  explicit string_views(U &&range)
      : view_(base_view{std::forward<U>(range)}, &to_view) {}

  auto begin() const { return view_.begin(); }
  auto end() const { return view_.end(); }

  auto size() const
    requires std::ranges::sized_range<base_view>
  {
    return view_.size();
  }
};

template <class R> string_views(R &&) -> string_views<std::views::all_t<R>>;

} // namespace v1

namespace v2 {
struct as_string_view {
  template <class S>
    requires std::convertible_to<S, std::string_view>
  std::string_view operator()(S &&s) const noexcept {
    return std::string_view{std::forward<S>(s)};
  }
};

template <class R>
  requires std::ranges::input_range<R> &&
           std::convertible_to<std::ranges::range_reference_t<R>,
                               std::string_view>
class string_views
    : public std::ranges::transform_view<std::views::all_t<R>, as_string_view> {
  using base_view = std::views::all_t<R>;
  using view_type = std::ranges::transform_view<base_view, as_string_view>;

public:
  template <class U>
    requires std::constructible_from<base_view, U>
  explicit string_views(U &&range)
      : view_type(base_view{std::forward<U>(range)}, as_string_view{}) {}
};

} // namespace v2

namespace v3 {

class string_views {
public:
  using value_type = std::string_view;

  class iterator {
  public:
    using iterator_concept = std::input_iterator_tag;
    using iterator_category = std::input_iterator_tag;
    using value_type = std::string_view;
    using difference_type = std::ptrdiff_t;
    using pointer = const std::string_view *;
    using reference = std::string_view;

    struct operations {
      bool (*next)(void *, value_type &);
      bool (*done)(const void *);
    };

    void *impl_ptr = nullptr;
    const operations *ops_ = nullptr;
    value_type current_{};
    bool has_value_ = false;

    void advance() {
      if (ops_ && ops_->next(impl_ptr, current_)) {
        has_value_ = true;
      } else {
        has_value_ = false;
      }
    }

    reference operator*() const { return current_; }
    pointer operator->() const { return &current_; }

    iterator &operator++() {
      advance();
      return *this;
    }

    void operator++(int) { advance(); }

    friend bool operator==(const iterator &it,
                           std::default_sentinel_t) noexcept {
      if (!it.has_value_)
        return true;
      if (it.ops_ && it.ops_->done(it.impl_ptr))
        return true;
      return false;
    }

    friend bool operator==(std::default_sentinel_t,
                           const iterator &it) noexcept {
      return it == std::default_sentinel_t{};
    }
  };

private:
  static constexpr std::size_t storage_size = 64;
  static constexpr std::size_t storage_align = alignof(std::max_align_t);

  struct range_operations {
    bool (*next)(void *, value_type &);
    bool (*done)(const void *);
    void (*destroy)(void *) noexcept;
    void (*move)(void *from, void *to) noexcept;
  };

  alignas(storage_align) std::byte storage_[storage_size]{};
  const range_operations *ops_ = nullptr;

  template <class I, class S> struct cursor {
    I current;
    S end;

    bool next(value_type &out) {
      if (current == end)
        return false;
      out = value_type{*current};
      ++current;
      return true;
    }

    bool done() const { return current == end; }
  };

  template <class C> static const range_operations &ops_for() {
    static constexpr range_operations table{
        [](void *p, value_type &out) -> bool {
          return static_cast<C *>(p)->next(out);
        },
        [](const void *p) -> bool { return static_cast<const C *>(p)->done(); },
        [](void *p) noexcept { std::destroy_at(static_cast<C *>(p)); },
        [](void *from, void *to) noexcept {
          auto *src = static_cast<C *>(from);
          std::construct_at(static_cast<C *>(to), std::move(*src));
          std::destroy_at(src);
        }};
    return table;
  }

  void reset() noexcept {
    if (ops_) {
      ops_->destroy(storage_);
      ops_ = nullptr;
    }
  }

public:
  string_views() noexcept = default;

  template <class R>
    requires std::ranges::input_range<R &> &&
             std::convertible_to<std::ranges::range_reference_t<R &>,
                                 value_type>
  explicit string_views(R &range) {
    using I = std::ranges::iterator_t<R &>;
    using S = std::ranges::sentinel_t<R &>;
    using C = cursor<I, S>;

    static_assert(sizeof(C) <= storage_size,
                  "string_views: iterator state exceeds inline storage");
    static_assert(alignof(C) <= storage_align,
                  "string_views: iterator state is over-aligned");
    static_assert(
        std::is_nothrow_move_constructible_v<C>,
        "string_views: iterator and sentinel must be nothrow movable");

    std::construct_at(reinterpret_cast<C *>(storage_),
                      C{std::ranges::begin(range), std::ranges::end(range)});

    ops_ = &ops_for<C>();
  }

  string_views(const string_views &) = delete;
  string_views &operator=(const string_views &) = delete;

  string_views(string_views &&other) noexcept
      : ops_(std::exchange(other.ops_, nullptr)) {
    if (ops_)
      ops_->move(other.storage_, storage_);
  }

  string_views &operator=(string_views &&other) noexcept {
    if (this != &other) {
      reset();
      ops_ = std::exchange(other.ops_, nullptr);
      if (ops_)
        ops_->move(other.storage_, storage_);
    }
    return *this;
  }

  ~string_views() { reset(); }

  iterator begin() {
    iterator it;
    it.impl_ptr = storage_;
    it.ops_ = reinterpret_cast<const iterator::operations *>(ops_);
    it.advance();
    return it;
  }

  std::default_sentinel_t end() const noexcept { return std::default_sentinel; }
};

} // namespace v3

namespace v4 {

class string_views {
public:
  using value_type = std::string_view;

  struct context {
    void *state = nullptr;
    bool (*next)(void *state, value_type &out) noexcept = nullptr;
  };

  class iterator {
  public:
    using iterator_concept = std::input_iterator_tag;
    using value_type = std::string_view;
    using difference_type = std::ptrdiff_t;
    using reference = std::string_view;

    context ctx_;
    value_type current_{};
    bool has_value_ = false;

    void advance() noexcept {
      if (ctx_.next && ctx_.next(ctx_.state, current_)) {
        has_value_ = true;
      } else {
        has_value_ = false;
      }
    }

    reference operator*() const noexcept { return current_; }

    iterator &operator++() noexcept {
      advance();
      return *this;
    }

    void operator++(int) noexcept { advance(); }

    friend bool operator==(const iterator &it,
                           std::default_sentinel_t) noexcept {
      return !it.has_value_;
    }

    friend bool operator==(std::default_sentinel_t,
                           const iterator &it) noexcept {
      return !it.has_value_;
    }
  };

private:
  context ctx_{};

public:
  constexpr string_views() noexcept = default;

  constexpr explicit string_views(context c) noexcept : ctx_(c) {}

  // Construct from any lvalue range using a local cursor helper
  template <class R>
    requires std::ranges::input_range<R &> &&
             std::convertible_to<std::ranges::range_reference_t<R &>,
                                 value_type>
  explicit string_views(R &range) {
    // The actual cursor state can reside on the caller's stack frame or
    // be managed by a helper factory function if needed.
  }

  iterator begin() const noexcept {
    iterator it{ctx_, {}, false};
    it.advance();
    return it;
  }

  constexpr std::default_sentinel_t end() const noexcept {
    return std::default_sentinel;
  }
};

} // namespace v4

namespace v5 {
struct as_string_view {
  template <class S>
    requires std::convertible_to<S, std::string_view>
  [[nodiscard]] constexpr std::string_view operator()(S &&s) const noexcept {
    return std::string_view{std::forward<S>(s)};
  }
};
using string_views =
    std::ranges::transform_view<std::span<std::string const>, as_string_view>;
} // namespace v5

namespace v6 {

class string_views {
public:
  // Iterator type for non-templated string_views
  class iterator {
  public:
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::string_view;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = std::string_view;

    constexpr iterator(const string_views *owner, std::size_t index) noexcept
        : owner_(owner), index_(index) {}

    constexpr std::string_view operator*() const noexcept {
      return owner_->get_at(index_);
    }

    constexpr iterator &operator++() noexcept {
      ++index_;
      return *this;
    }

    constexpr iterator operator++(int) noexcept {
      iterator tmp = *this;
      ++index_;
      return tmp;
    }

    friend constexpr bool operator==(const iterator &a,
                                     const iterator &b) noexcept {
      return a.index_ == b.index_;
    }

    friend constexpr bool operator!=(const iterator &a,
                                     const iterator &b) noexcept {
      return a.index_ != b.index_;
    }

  private:
    const string_views *owner_;
    std::size_t index_;
  };

  // Constructor accepting any range whose elements are convertible to
  // std::string_view
  template <class Range>
    requires(!std::is_same_v<std::decay_t<Range>, string_views>) &&
                std::ranges::range<Range> &&
                std::convertible_to<std::ranges::range_reference_t<Range>,
                                    std::string_view>
  constexpr explicit string_views(Range &r) noexcept
      : range_ptr_(const_cast<void *>(static_cast<const void *>(&r))),
        size_fn([](const void *ptr) noexcept -> std::size_t {
          auto &ref = *static_cast<Range *>(const_cast<void *>(ptr));
          return static_cast<std::size_t>(std::ranges::distance(ref));
        }),
        get_fn(
            [](const void *ptr, std::size_t idx) noexcept -> std::string_view {
              auto &ref = *static_cast<Range *>(const_cast<void *>(ptr));
              auto it = std::ranges::begin(ref);
              std::ranges::advance(it, idx);
              return std::string_view{*it}; // or *it
            }) {}

  // Default constructor (empty)
  constexpr string_views() noexcept = default;

  constexpr std::size_t size() const noexcept {
    return range_ptr_ ? size_fn(range_ptr_) : 0;
  }

  constexpr bool empty() const noexcept { return size() == 0; }

  constexpr iterator begin() const noexcept { return iterator{this, 0}; }

  constexpr iterator end() const noexcept { return iterator{this, size()}; }

private:
  void *range_ptr_ = nullptr;
  std::size_t (*size_fn)(const void *) noexcept = nullptr;
  std::string_view (*get_fn)(const void *, std::size_t) noexcept = nullptr;

  constexpr std::string_view get_at(std::size_t idx) const noexcept {
    return get_fn(range_ptr_, idx);
  }
};

} // namespace v6

namespace v7 {

template <typename CharT, typename TraitsT>
struct [[nodiscard]] as_string_view {
  using string_view_type = std::basic_string_view<CharT, TraitsT>;
  template <class S>
    requires std::convertible_to<S, string_view_type>
  [[nodiscard]] constexpr string_view_type operator()(S &&s) const noexcept {
    return string_view_type{std::forward<S>(s)};
  }
};

template <typename CharT = char, typename TraitsT = std::char_traits<CharT>>
struct [[nodiscard]] basic_string_views
    : std::ranges::transform_view<
          std::span<std::basic_string<CharT, TraitsT> const>,
          as_string_view<CharT, TraitsT>> {
  using base_type = std::ranges::transform_view<
      std::span<std::basic_string<CharT, TraitsT> const>,
      as_string_view<CharT, TraitsT>>;

  template <typename R>
  constexpr basic_string_views(R &&beg) noexcept : base_type{beg, {}} {}
};

using string_views = basic_string_views<>;
} // namespace v7

namespace v8 {

template <typename CharT, typename TraitsT>
struct [[nodiscard]] as_string_view {
  using string_view_type = std::basic_string_view<CharT, TraitsT>;
  template <class S>
    requires std::convertible_to<S, string_view_type>
  [[nodiscard]] constexpr string_view_type operator()(S &&s) const noexcept {
    return string_view_type{std::forward<S>(s)};
  }
};

namespace detail {

// Const random-access iterator over a type-erased source of string_views.
template <typename SV> class erased_sv_iterator {
public:
  using value_type = SV;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::random_access_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;

  // "deref(i) -> SV" where i is a byte offset from the source's begin().
  using deref_fn = SV (*)(const void *base, std::ptrdiff_t i) noexcept;

  constexpr erased_sv_iterator() = default;
  constexpr erased_sv_iterator(const void *base, std::ptrdiff_t i,
                               deref_fn f) noexcept
      : base_{base}, i_{i}, f_{f} {}

  SV operator*() const noexcept { return f_(base_, i_); }
  SV operator[](difference_type n) const noexcept { return f_(base_, i_ + n); }

  erased_sv_iterator &operator++() noexcept {
    ++i_;
    return *this;
  }
  erased_sv_iterator operator++(int) noexcept {
    auto t = *this;
    ++i_;
    return t;
  }
  erased_sv_iterator &operator--() noexcept {
    --i_;
    return *this;
  }
  erased_sv_iterator operator--(int) noexcept {
    auto t = *this;
    --i_;
    return t;
  }
  erased_sv_iterator &operator+=(difference_type n) noexcept {
    i_ += n;
    return *this;
  }
  erased_sv_iterator &operator-=(difference_type n) noexcept {
    i_ -= n;
    return *this;
  }
  friend erased_sv_iterator operator+(erased_sv_iterator it,
                                      difference_type n) noexcept {
    return it += n;
  }
  friend erased_sv_iterator operator+(difference_type n,
                                      erased_sv_iterator it) noexcept {
    return it += n;
  }
  friend erased_sv_iterator operator-(erased_sv_iterator it,
                                      difference_type n) noexcept {
    return it -= n;
  }

  friend difference_type operator-(const erased_sv_iterator &a,
                                   const erased_sv_iterator &b) noexcept {
    return a.i_ - b.i_;
  }
  friend bool operator==(const erased_sv_iterator &a,
                         const erased_sv_iterator &b) noexcept {
    return a.i_ == b.i_;
  }
  friend auto operator<=>(const erased_sv_iterator &a,
                          const erased_sv_iterator &b) noexcept {
    return a.i_ <=> b.i_;
  }

private:
  const void *base_ = nullptr;
  std::ptrdiff_t i_ = 0;
  deref_fn f_ = nullptr;
};

} // namespace detail

template <typename CharT = char, typename TraitsT = std::char_traits<CharT>>
struct [[nodiscard]] basic_string_views
    : std::ranges::view_interface<basic_string_views<CharT, TraitsT>> {
  using string_view_type = std::basic_string_view<CharT, TraitsT>;
  using iterator = detail::erased_sv_iterator<string_view_type>;
  using sentinel = iterator;

  // One entry point for every supported source; no allocation, no ownership.
  template <std::ranges::contiguous_range R>
    requires std::ranges::sized_range<R> &&
                 std::convertible_to<std::ranges::range_reference_t<R>,
                                     string_view_type>
  constexpr basic_string_views(R &&r) noexcept
      : base_{std::ranges::data(r)},
        size_{static_cast<std::ptrdiff_t>(std::ranges::size(r))},
        deref_{[](const void *b,
                  std::ptrdiff_t i) noexcept -> string_view_type {
          return string_view_type{std::ranges::data(
              *static_cast<std::add_pointer_t<R>>(const_cast<void *>(b)))[i]};
        }} {}

  constexpr iterator begin() const noexcept { return {base_, 0, deref_}; }
  constexpr iterator end() const noexcept { return {base_, size_, deref_}; }
  constexpr std::ptrdiff_t size() const noexcept { return size_; }

private:
  const void *base_ = nullptr;
  std::ptrdiff_t size_ = 0;
  string_view_type (*deref_)(const void *, std::ptrdiff_t) noexcept = nullptr;
};

using string_views = basic_string_views<>;
} // namespace v8

namespace v9 {

template <class CharT = char, class TraitsT = std::char_traits<CharT>>
class basic_string_views {
public:
  using string_type = std::basic_string<CharT, TraitsT>;
  using value_type = std::basic_string_view<CharT, TraitsT>;
  using size_type = std::size_t;

private:
  union storage {
    const string_type *strings;
    const value_type *views;

    constexpr storage(const string_type *p) noexcept : strings(p) {}
    constexpr storage(const value_type *p) noexcept : views(p) {}
  };

  storage data_{static_cast<const value_type *>(nullptr)};
  size_type size_ = 0;
  bool holds_strings_ = false;

public:
  constexpr basic_string_views() noexcept = default;

  constexpr basic_string_views(const string_type *p, size_type n) noexcept
      : data_(p), size_(n), holds_strings_(true) {}

  constexpr basic_string_views(const value_type *p, size_type n) noexcept
      : data_(p), size_(n), holds_strings_(false) {}

  // Accept mutable or const lvalue containers, but not temporary owners.
  template <class R>
    requires requires(R &r) {
      std::size(r);
      requires std::convertible_to<decltype(std::data(r)),
                                   const string_type *> ||
                   std::convertible_to<decltype(std::data(r)),
                                       const value_type *>;
    }
  constexpr basic_string_views(R &r) noexcept(
      noexcept(basic_string_views(std::data(r), std::size(r))))
      : basic_string_views(std::data(r), std::size(r)) {}

  [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
  [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }

  // Precondition: i < size().
  [[nodiscard]] constexpr value_type operator[](size_type i) const noexcept {
    if (holds_strings_)
      return value_type(data_.strings[i]);
    return data_.views[i];
  }

  // f receives either (const string_type*, size_type)
  // or (const value_type*, size_type). Both calls must have the same result
  // type.
  template <class F> constexpr decltype(auto) visit(F &&f) const {
    if (holds_strings_)
      return std::forward<F>(f)(data_.strings, size_);
    return std::forward<F>(f)(data_.views, size_);
  }

  class iterator {
    friend class basic_string_views;
    const basic_string_views *owner_ = nullptr;
    std::ptrdiff_t pos_ = 0;

    constexpr iterator(const basic_string_views *owner,
                       std::ptrdiff_t pos) noexcept
        : owner_(owner), pos_(pos) {}

  public:
    using value_type = typename basic_string_views::value_type;
    using difference_type = std::ptrdiff_t;
    using reference = value_type; // returned by value
    using pointer = void;
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::input_iterator_tag; // legacy category

    constexpr iterator() noexcept = default;

    constexpr reference operator*() const noexcept {
      return (*owner_)[static_cast<size_type>(pos_)];
    }
    constexpr reference operator[](difference_type n) const noexcept {
      return *(*this + n);
    }

    constexpr iterator &operator++() noexcept {
      ++pos_;
      return *this;
    }
    constexpr iterator &operator--() noexcept {
      --pos_;
      return *this;
    }
    constexpr iterator operator++(int) noexcept {
      auto old = *this;
      ++*this;
      return old;
    }
    constexpr iterator operator--(int) noexcept {
      auto old = *this;
      --*this;
      return old;
    }
    constexpr iterator &operator+=(difference_type n) noexcept {
      pos_ += n;
      return *this;
    }
    constexpr iterator &operator-=(difference_type n) noexcept {
      pos_ -= n;
      return *this;
    }

    friend constexpr iterator operator+(iterator i,
                                        difference_type n) noexcept {
      return i += n;
    }
    friend constexpr iterator operator+(difference_type n,
                                        iterator i) noexcept {
      return i += n;
    }
    friend constexpr iterator operator-(iterator i,
                                        difference_type n) noexcept {
      return i -= n;
    }
    friend constexpr difference_type operator-(iterator a,
                                               iterator b) noexcept {
      return a.pos_ - b.pos_; // precondition: same sequence
    }
    constexpr auto operator<=>(const iterator &) const noexcept = default;
  };

  [[nodiscard]] constexpr iterator begin() const noexcept { return {this, 0}; }
  [[nodiscard]] constexpr iterator end() const noexcept {
    return {this, static_cast<std::ptrdiff_t>(size_)};
  }
};

using string_views = basic_string_views<>;
} // namespace v9

namespace v10 {

// Non-owning. The source elements and their character storage must remain
// valid.
template <class CharT = char, class TraitsT = std::char_traits<CharT>>
class basic_string_views {
public:
  using string_type = std::basic_string<CharT, TraitsT>;
  using value_type = std::basic_string_view<CharT, TraitsT>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

private:
  static constexpr size_type string_bit =
      size_type{1} << (std::numeric_limits<size_type>::digits - 1);

  union storage {
    const string_type *strings;
    const value_type *views;
    constexpr storage(const string_type *p) noexcept : strings(p) {}
    constexpr storage(const value_type *p) noexcept : views(p) {}
  };

  storage data_{static_cast<const value_type *>(nullptr)};
  size_type tagged_size_ = 0;

  template <class T>
  static constexpr bool source_element =
      std::same_as<T, string_type> || std::same_as<T, value_type>;

public:
  constexpr basic_string_views() noexcept = default;

  // Precondition: p points to n readable elements (p may be null if n == 0).
  // n must fit both a difference_type and the untagged size bits.
  template <class T>
    requires source_element<T>
  constexpr basic_string_views(const T *p, size_type n) noexcept
      : data_(p),
        tagged_size_(n | (std::same_as<T, string_type> ? string_bit : 0)) {
    assert(n < string_bit && std::in_range<difference_type>(n));
    assert(p != nullptr || n == 0);
  }

  // Accept contiguous lvalue containers and built-in arrays, not temporary
  // owners.
  template <class R>
    requires requires(R &r) { basic_string_views(std::data(r), std::size(r)); }
  constexpr basic_string_views(R &r) noexcept(
      noexcept(basic_string_views(std::data(r), std::size(r))))
      : basic_string_views(std::data(r), std::size(r)) {}

  [[nodiscard]] constexpr size_type size() const noexcept {
    return tagged_size_ & ~string_bit;
  }
  [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

  // Returns a string_view by value, not a reference to a cached object.
  [[nodiscard]] constexpr value_type operator[](size_type i) const noexcept {
    assert(i < size());
    if (tagged_size_ & string_bit)
      return value_type{data_.strings[i]};
    return data_.views[i];
  }
  [[nodiscard]] constexpr value_type front() const noexcept {
    return (*this)[0];
  }
  [[nodiscard]] constexpr value_type back() const noexcept {
    return (*this)[size() - 1];
  }

  // Both callback overloads must have the same return type (void is fine).
  template <class F> constexpr decltype(auto) visit(F &&f) const {
    const size_type n = tagged_size_;
    if (n & string_bit)
      return std::forward<F>(f)(static_cast<const string_type *>(data_.strings),
                                n ^ string_bit);
    return std::forward<F>(f)(static_cast<const value_type *>(data_.views),
                              size_type{n});
  }

  // Dispatch once per traversal, not once per element.
  template <class F> constexpr void for_each(F &&f) const {
    visit([&f](const auto *p, size_type n) {
      for (size_type i = 0; i < n; ++i)
        f(value_type{p[i]});
    });
  }

  // Iterators refer to this view, which must stay alive and unchanged.
  class iterator {
  public:
    using value_type = typename basic_string_views::value_type;
    using difference_type = std::ptrdiff_t;
    using reference = value_type;
    using pointer = void;
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category =
        std::input_iterator_tag; // dereference is a prvalue

    constexpr iterator() noexcept = default;
    [[nodiscard]] constexpr reference operator*() const noexcept {
      return (*owner_)[static_cast<size_type>(pos_)];
    }
    [[nodiscard]] constexpr reference
    operator[](difference_type n) const noexcept {
      return *(*this + n);
    }
    constexpr iterator &operator++() noexcept {
      ++pos_;
      return *this;
    }
    constexpr iterator &operator--() noexcept {
      --pos_;
      return *this;
    }
    constexpr iterator operator++(int) noexcept {
      auto i = *this;
      ++*this;
      return i;
    }
    constexpr iterator operator--(int) noexcept {
      auto i = *this;
      --*this;
      return i;
    }
    constexpr iterator &operator+=(difference_type n) noexcept {
      pos_ += n;
      return *this;
    }
    constexpr iterator &operator-=(difference_type n) noexcept {
      pos_ -= n;
      return *this;
    }
    friend constexpr iterator operator+(iterator i,
                                        difference_type n) noexcept {
      return i += n;
    }
    friend constexpr iterator operator+(difference_type n,
                                        iterator i) noexcept {
      return i += n;
    }
    friend constexpr iterator operator-(iterator i,
                                        difference_type n) noexcept {
      return i -= n;
    }
    friend constexpr difference_type operator-(iterator a,
                                               iterator b) noexcept {
      assert(a.owner_ == b.owner_);
      return a.pos_ - b.pos_;
    }
    constexpr auto operator<=>(const iterator &) const noexcept = default;

  private:
    friend class basic_string_views;
    const basic_string_views *owner_ = nullptr;
    difference_type pos_ = 0;
    constexpr iterator(const basic_string_views *owner,
                       difference_type pos) noexcept
        : owner_(owner), pos_(pos) {}
  };

  [[nodiscard]] constexpr iterator begin() const noexcept { return {this, 0}; }
  [[nodiscard]] constexpr iterator end() const noexcept {
    return {this, static_cast<difference_type>(size())};
  }
};

using string_views = basic_string_views<>;

} // namespace v10

namespace v11 {

// Const random-access iterator over a type-erased source of string_views.
template <typename SV> class erased_sv_iterator {
public:
  using value_type = SV;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::random_access_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;

  // "deref(i) -> SV" where i is a byte offset from the source's begin().
  using deref_fn = SV (*)(const void *base, std::ptrdiff_t i) noexcept;

  constexpr erased_sv_iterator() = default;
  constexpr erased_sv_iterator(const void *base, std::ptrdiff_t i,
                               deref_fn f) noexcept
      : base_{base}, i_{i}, f_{f} {}

  SV operator*() const noexcept { return f_(base_, i_); }
  SV operator[](difference_type n) const noexcept { return f_(base_, i_ + n); }

  erased_sv_iterator &operator++() noexcept {
    ++i_;
    return *this;
  }
  erased_sv_iterator operator++(int) noexcept {
    auto t = *this;
    ++i_;
    return t;
  }
  erased_sv_iterator &operator--() noexcept {
    --i_;
    return *this;
  }
  erased_sv_iterator operator--(int) noexcept {
    auto t = *this;
    --i_;
    return t;
  }
  erased_sv_iterator &operator+=(difference_type n) noexcept {
    i_ += n;
    return *this;
  }
  erased_sv_iterator &operator-=(difference_type n) noexcept {
    i_ -= n;
    return *this;
  }
  friend erased_sv_iterator operator+(erased_sv_iterator it,
                                      difference_type n) noexcept {
    return it += n;
  }
  friend erased_sv_iterator operator+(difference_type n,
                                      erased_sv_iterator it) noexcept {
    return it += n;
  }
  friend erased_sv_iterator operator-(erased_sv_iterator it,
                                      difference_type n) noexcept {
    return it -= n;
  }

  friend difference_type operator-(const erased_sv_iterator &a,
                                   const erased_sv_iterator &b) noexcept {
    return a.i_ - b.i_;
  }
  friend bool operator==(const erased_sv_iterator &a,
                         const erased_sv_iterator &b) noexcept {
    return a.i_ == b.i_;
  }
  friend auto operator<=>(const erased_sv_iterator &a,
                          const erased_sv_iterator &b) noexcept {
    return a.i_ <=> b.i_;
  }

private:
  const void *base_ = nullptr;
  std::ptrdiff_t i_ = 0;
  deref_fn f_ = nullptr;
};

template <class CharT = char, class TraitsT = std::char_traits<CharT>>
class [[nodiscard]] basic_string_views {
public:
  using string_view_type = std::basic_string_view<CharT, TraitsT>;
  using string_type = std::basic_string<CharT, TraitsT>;
  using iterator = erased_sv_iterator<string_view_type>;
  using sentinel = iterator;

private:
  using index_type = std::ptrdiff_t;
  using deref_fn = typename iterator::deref_fn;

  // One function per ELEMENT type, not per container type.
  template <class T>
  static string_view_type read(const void *base, index_type i) noexcept {
    return string_view_type{static_cast<const T *>(base)[i]};
  }

  const void *base_ = nullptr;
  index_type size_ = 0;
  deref_fn deref_ = &read<string_view_type>;

public:
  constexpr basic_string_views() noexcept = default;

  // Lvalues only: don't silently retain a pointer into a temporary vector.
  // Precondition: size fits in ptrdiff_t and data addresses size elements.
  template <class R, class P = decltype(std::data(std::declval<R &>())),
            class T = std::remove_cv_t<std::remove_pointer_t<P>>>
    requires std::is_pointer_v<P> && std::convertible_to<P, const T *> &&
                 std::convertible_to<const T &, string_view_type> &&
                 std::is_nothrow_constructible_v<string_view_type, const T &> &&
                 requires(R &r) { std::size(r); }
  constexpr basic_string_views(R &r) noexcept(
      noexcept(std::data(r)) && noexcept(static_cast<index_type>(std::size(r))))
      : base_(std::data(r)), size_(static_cast<index_type>(std::size(r))),
        deref_(&read<T>) {}

  constexpr iterator begin() const noexcept { return {base_, 0, deref_}; }
  constexpr iterator end() const noexcept { return {base_, size_, deref_}; }
  constexpr index_type size() const noexcept { return size_; }
  constexpr bool empty() const noexcept { return size_ == 0; }

  string_view_type operator[](index_type i) const noexcept {
    return deref_(base_, i);
  }

  template <class F> void for_each(F &&f) const {
    const auto n = size_;
    const auto b = base_;
    const auto read_one = deref_;

    if (read_one == &read<string_view_type>) {
      const auto *p = static_cast<const string_view_type *>(b);
      for (index_type i = 0; i < n; ++i)
        f(string_view_type{p[i]});
    } else if (read_one == &read<string_type>) {
      const auto *p = static_cast<const string_type *>(b);
      for (index_type i = 0; i < n; ++i)
        f(string_view_type{p[i]});
    } else {
      for (index_type i = 0; i < n; ++i)
        f(read_one(b, i));
    }
  }
};

using string_views = basic_string_views<>;
} // namespace v11

namespace v12 {

namespace detail {

template <class SV, class T>
inline SV read_sv(const void *base, std::ptrdiff_t i) noexcept {
  return SV{static_cast<const T *>(base)[i]};
}

template <class SV> class erased_sv_iterator {
public:
  using value_type = SV;
  using difference_type = std::ptrdiff_t;
  using reference = SV;
  using pointer = void;
  using iterator_concept = std::random_access_iterator_tag;
  using iterator_category = std::input_iterator_tag; // prvalue dereference
  using deref_fn = SV (*)(const void *, difference_type) noexcept;

  constexpr erased_sv_iterator() noexcept = default;
  constexpr erased_sv_iterator(const void *p, difference_type i,
                               deref_fn read) noexcept
      : base_(p), pos_(i), read_(read) {}

  [[nodiscard]] reference operator*() const noexcept { return load(pos_); }
  [[nodiscard]] reference operator[](difference_type n) const noexcept {
    return load(pos_ + n);
  }
  constexpr erased_sv_iterator &operator++() noexcept {
    ++pos_;
    return *this;
  }
  constexpr erased_sv_iterator &operator--() noexcept {
    --pos_;
    return *this;
  }
  constexpr erased_sv_iterator operator++(int) noexcept {
    auto old = *this;
    ++*this;
    return old;
  }
  constexpr erased_sv_iterator operator--(int) noexcept {
    auto old = *this;
    --*this;
    return old;
  }
  constexpr erased_sv_iterator &operator+=(difference_type n) noexcept {
    pos_ += n;
    return *this;
  }
  constexpr erased_sv_iterator &operator-=(difference_type n) noexcept {
    pos_ -= n;
    return *this;
  }
  friend constexpr erased_sv_iterator operator+(erased_sv_iterator i,
                                                difference_type n) noexcept {
    return i += n;
  }
  friend constexpr erased_sv_iterator operator+(difference_type n,
                                                erased_sv_iterator i) noexcept {
    return i += n;
  }
  friend constexpr erased_sv_iterator operator-(erased_sv_iterator i,
                                                difference_type n) noexcept {
    return i -= n;
  }
  friend constexpr difference_type operator-(erased_sv_iterator a,
                                             erased_sv_iterator b) noexcept {
    assert(a.base_ == b.base_ && a.read_ == b.read_);
    return a.pos_ - b.pos_;
  }
  friend constexpr bool operator==(erased_sv_iterator a,
                                   erased_sv_iterator b) noexcept {
    return a.base_ == b.base_ && a.pos_ == b.pos_ && a.read_ == b.read_;
  }
  friend constexpr auto operator<=>(erased_sv_iterator a,
                                    erased_sv_iterator b) noexcept {
    assert(a.base_ == b.base_ && a.read_ == b.read_); // same sequence
    return a.pos_ <=> b.pos_;
  }

private:
  using string_type =
      std::basic_string<typename SV::value_type, typename SV::traits_type>;
  SV load(difference_type i) const noexcept {
    if (read_ == &read_sv<SV, SV>)
      return read_sv<SV, SV>(base_, i);
    if (read_ == &read_sv<SV, string_type>)
      return read_sv<SV, string_type>(base_, i);
    return read_(base_, i);
  }
  const void *base_ = nullptr;
  difference_type pos_ = 0;
  deref_fn read_ = nullptr;
};
} // namespace detail

template <class CharT = char, class TraitsT = std::char_traits<CharT>>
class [[nodiscard]] basic_string_views {
public:
  using string_view_type = std::basic_string_view<CharT, TraitsT>;
  using value_type = string_view_type;
  using string_type = std::basic_string<CharT, TraitsT>;
  using iterator = detail::erased_sv_iterator<value_type>;
  using sentinel = iterator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  constexpr basic_string_views() noexcept = default;

  // p denotes n contiguous live elements; null is allowed for an empty source.
  // Conversion of each element must yield a view into still-live storage.
  template <class T>
    requires(!std::is_volatile_v<T>) &&
                std::convertible_to<const T &, value_type> &&
                std::is_nothrow_constructible_v<value_type, const T &>
  constexpr basic_string_views(const T *p, size_type n) noexcept
      : base_(p), size_(n), read_(&detail::read_sv<value_type, T>) {
    assert((p || !n) && n <= static_cast<size_type>((
                                 std::numeric_limits<difference_type>::max)()));
  }

  template <class R>
    requires requires(R &r) { basic_string_views(std::data(r), std::size(r)); }
  constexpr basic_string_views(R &r) noexcept(
      noexcept(basic_string_views(std::data(r), std::size(r))))
      : basic_string_views(std::data(r), std::size(r)) {}

  [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
  [[nodiscard]] constexpr bool empty() const noexcept { return !size_; }
  [[nodiscard]] constexpr iterator begin() const noexcept {
    return {base_, 0, read_};
  }
  [[nodiscard]] constexpr iterator end() const noexcept {
    return {base_, static_cast<difference_type>(size_), read_};
  }
  [[nodiscard]] value_type operator[](size_type i) const noexcept {
    assert(i < size_);
    return begin()[static_cast<difference_type>(i)];
  }
  [[nodiscard]] value_type front() const noexcept { return (*this)[0]; }
  [[nodiscard]] value_type back() const noexcept { return (*this)[size_ - 1]; }

  template <class F> void for_each(F &&f) const {
    const auto b = base_;
    const auto n = static_cast<difference_type>(size_);
    const auto read = read_;
    if (read == &detail::read_sv<value_type, value_type>) {
      const auto *p = static_cast<const value_type *>(b);
      for (difference_type i = 0; i < n; ++i)
        f(value_type{p[i]});
    } else if (read == &detail::read_sv<value_type, string_type>) {
      const auto *p = static_cast<const string_type *>(b);
      for (difference_type i = 0; i < n; ++i)
        f(value_type{p[i]});
    } else {
      for (difference_type i = 0; i < n; ++i)
        f(read(b, i));
    }
  }

private:
  const void *base_ = nullptr;
  size_type size_ = 0;
  typename iterator::deref_fn read_ = &detail::read_sv<value_type, value_type>;
};

using string_views = basic_string_views<>;
static_assert(std::random_access_iterator<string_views::iterator>);

} // namespace v12

namespace v13 {
// Const random-access iterator over a type-erased source of string_views.
template <typename SV> struct [[nodiscard]] erased_sv_iterator {
  using value_type = SV;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::random_access_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;
  using size_type = SV::size_type;

  // "deref(i) -> SV" where i is a byte offset from the source's begin().
  using deref_fn = SV (*)(void const *base, std::ptrdiff_t index) noexcept;

  constexpr erased_sv_iterator() = default;

  constexpr erased_sv_iterator(void const *inp_base, size_type const inp_index,
                               deref_fn const inp_func) noexcept
      : base{inp_base}, index{inp_index}, func{inp_func} {}

  constexpr SV operator*() const noexcept { return func(base, index); }

  constexpr SV operator[](difference_type n) const noexcept {
    return func(base, index + n);
  }

  constexpr erased_sv_iterator &operator++() noexcept {
    ++index;
    return *this;
  }

  constexpr erased_sv_iterator operator++(int) noexcept {
    auto t = *this;
    ++index;
    return t;
  }

  constexpr erased_sv_iterator &operator--() noexcept {
    --index;
    return *this;
  }

  constexpr erased_sv_iterator operator--(int) noexcept {
    auto t = *this;
    --index;
    return t;
  }

  constexpr erased_sv_iterator &operator+=(difference_type n) noexcept {
    index += n;
    return *this;
  }

  constexpr erased_sv_iterator &operator-=(difference_type n) noexcept {
    index -= n;
    return *this;
  }

  friend constexpr erased_sv_iterator operator+(erased_sv_iterator iter,
                                                difference_type len) noexcept {
    return iter += len;
  }

  friend constexpr erased_sv_iterator
  operator+(difference_type len, erased_sv_iterator iter) noexcept {
    return iter += len;
  }

  friend constexpr erased_sv_iterator operator-(erased_sv_iterator iter,
                                                difference_type len) noexcept {
    return iter -= len;
  }

  friend constexpr difference_type
  operator-(erased_sv_iterator const &lhs,
            erased_sv_iterator const &rhs) noexcept {
    return lhs.index - rhs.index;
  }

  friend constexpr bool operator==(erased_sv_iterator const &lhs,
                                   erased_sv_iterator const &rhs) noexcept {
    return lhs.index == rhs.index;
  }

  friend constexpr auto operator<=>(erased_sv_iterator const &lhs,
                                    erased_sv_iterator const &rhs) noexcept {
    return lhs.index <=> rhs.index;
  }

private:
  void const *base = nullptr;
  size_type index = 0;
  deref_fn func = nullptr;
};

/**
 * Span of StringViews
 * @tparam CharT
 * @tparam TraitsT
 */
template <typename CharT = char, typename TraitsT = std::char_traits<CharT>>
struct [[nodiscard]] basic_string_views {
  using string_view_type = std::basic_string_view<CharT, TraitsT>;
  using string_type = std::basic_string<CharT, TraitsT>;
  using iterator = erased_sv_iterator<string_view_type>;
  using sentinel = iterator;
  using size_type = string_view_type::size_type;

private:
  using index_type = std::ptrdiff_t;
  using deref_fn = iterator::deref_fn;
  using advance_fn = void const *(*)(void const *base,
                                     size_type index) noexcept;

  struct [[nodiscard]] operations {
    deref_fn read;
    advance_fn advance;
  };

  // One function per ELEMENT type, not per container type.
  template <typename T>
  static constexpr operations ops_for{
      .read =
          +[](void const *base, index_type index) noexcept -> string_view_type {
        return string_view_type{static_cast<T const *>(base)[index]};
      },
      .advance = +[](void const *p, size_type n) noexcept -> void const * {
        return static_cast<T const *>(p) + n;
      },
  };

  void const *base = nullptr;
  size_type m_size = 0;
  operations const *ops = &ops_for<string_view_type>;

public:
  constexpr basic_string_views() noexcept = default;

  // Lvalues only: don't silently retain a pointer into a temporary
  // vector. Precondition: size fits in ptrdiff_t and data addresses size
  // elements.
  template <typename R, typename P = decltype(std::data(std::declval<R &>())),
            typename T = std::remove_cvref_t<std::remove_pointer_t<P>>>
    requires std::is_pointer_v<P> && std::convertible_to<P, T const *> &&
                 std::convertible_to<T const &, string_view_type> &&
                 std::is_nothrow_constructible_v<string_view_type, T const &> &&
                 requires(R &rng) { std::size(rng); }
  constexpr basic_string_views(R &rng) noexcept(
      noexcept(std::data(rng)) &&
      noexcept(static_cast<index_type>(std::size(rng))))
      : base(std::data(rng)), m_size(static_cast<index_type>(std::size(rng))),
        ops(&ops_for<T>) {}

  template <typename P = iterator,
            typename T = std::remove_cvref_t<std::remove_pointer_t<P>>>
  constexpr basic_string_views(P const begin, size_type const len) noexcept
      : base(begin), m_size(len), ops(&ops_for<T>) {}

  constexpr basic_string_views(basic_string_views const &) = default;
  constexpr basic_string_views(basic_string_views &&) noexcept = default;
  constexpr basic_string_views &operator=(basic_string_views const &) = default;
  constexpr basic_string_views &
  operator=(basic_string_views &&) noexcept = default;
  constexpr ~basic_string_views() noexcept = default;

  constexpr iterator begin() const noexcept { return {base, 0, ops->read}; }

  constexpr iterator end() const noexcept { return {base, m_size, ops->read}; }

  [[nodiscard]] constexpr size_type size() const noexcept { return m_size; }

  [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }

  [[nodiscard]] string_view_type operator[](index_type index) const noexcept {
    return ops->read(base, index);
  }

  [[nodiscard]] string_view_type at(index_type index) const {
    if (index >= static_cast<index_type>(m_size)) [[unlikely]] {
      throw std::out_of_range("basic_string_views::at: index out of range");
    }
    return ops->read(base, index);
  }

  [[nodiscard]] string_view_type back() const noexcept {
    assert(m_size > 0);
    return ops->read(base, m_size - 1);
  }

  [[nodiscard]] string_view_type front() const noexcept {
    assert(m_size > 0);
    return ops->read(base, 0);
  }

  [[nodiscard]] void const *data() const noexcept { return base; }

  [[nodiscard]] constexpr basic_string_views
  first(size_type const count) const noexcept {
    assert(count <= m_size);
    auto that = *this;
    that.m_size = count;
    return that;
  }

  [[nodiscard]] constexpr basic_string_views
  last(size_type const count) const noexcept {
    assert(count <= m_size);
    return basic_string_views(base + (m_size - count), count);
  }

  template <class F> void for_each(F &&func) const {
    if (ops->read == &ops_for<string_view_type>.read) {
      auto const *ptr = static_cast<string_view_type const *>(base);
      for (index_type i = 0; i < m_size; ++i) {
        func(string_view_type{ptr[i]});
      }
    } else if (ops->read == &ops_for<string_type>.read) {
      auto const *ptr = static_cast<string_type const *>(base);
      for (index_type i = 0; i < m_size; ++i) {
        func(string_view_type{ptr[i]});
      }
    } else {
      for (index_type i = 0; i < m_size; ++i) {
        func(ops->read(base, i));
      }
    }
  }
};

using string_views = basic_string_views<>;

} // namespace v13

namespace v14 {

// Const random-access iterator over a type-erased source of string_views.
template <typename SV> struct [[nodiscard]] erased_sv_iterator {
  using value_type = SV;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::random_access_iterator_tag;
  using iterator_category = std::random_access_iterator_tag;
  using size_type = SV::size_type;

  // "deref(i) -> SV" where i is a byte offset from the source's begin().
  using deref_fn = SV (*)(void const *base, std::ptrdiff_t index) noexcept;

  constexpr erased_sv_iterator() = default;

  constexpr erased_sv_iterator(void const *inp_base, size_type const inp_index,
                               deref_fn const inp_func) noexcept
      : base{inp_base}, index{inp_index}, func{inp_func} {}

  constexpr SV operator*() const noexcept { return func(base, index); }

  constexpr SV operator[](difference_type n) const noexcept {
    return func(base, index + n);
  }

  constexpr erased_sv_iterator &operator++() noexcept {
    ++index;
    return *this;
  }

  constexpr erased_sv_iterator operator++(int) noexcept {
    auto t = *this;
    ++index;
    return t;
  }

  constexpr erased_sv_iterator &operator--() noexcept {
    --index;
    return *this;
  }

  constexpr erased_sv_iterator operator--(int) noexcept {
    auto t = *this;
    --index;
    return t;
  }

  constexpr erased_sv_iterator &operator+=(difference_type n) noexcept {
    index += n;
    return *this;
  }

  constexpr erased_sv_iterator &operator-=(difference_type n) noexcept {
    index -= n;
    return *this;
  }

  friend constexpr erased_sv_iterator operator+(erased_sv_iterator iter,
                                                difference_type len) noexcept {
    return iter += len;
  }

  friend constexpr erased_sv_iterator
  operator+(difference_type len, erased_sv_iterator iter) noexcept {
    return iter += len;
  }

  friend constexpr erased_sv_iterator operator-(erased_sv_iterator iter,
                                                difference_type len) noexcept {
    return iter -= len;
  }

  friend constexpr difference_type
  operator-(erased_sv_iterator const &lhs,
            erased_sv_iterator const &rhs) noexcept {
    return lhs.index - rhs.index;
  }

  friend constexpr bool operator==(erased_sv_iterator const &lhs,
                                   erased_sv_iterator const &rhs) noexcept {
    return lhs.index == rhs.index;
  }

  friend constexpr auto operator<=>(erased_sv_iterator const &lhs,
                                    erased_sv_iterator const &rhs) noexcept {
    return lhs.index <=> rhs.index;
  }

private:
  void const *base = nullptr;
  size_type index = 0;
  deref_fn func = nullptr;
};

/**
 * Span of StringViews
 * @tparam CharT
 * @tparam TraitsT
 */
template <typename CharT = char, typename TraitsT = std::char_traits<CharT>>
struct [[nodiscard]] basic_string_views {
  using string_view_type = std::basic_string_view<CharT, TraitsT>;
  using string_type = std::basic_string<CharT, TraitsT>;
  using iterator = erased_sv_iterator<string_view_type>;
  using sentinel = iterator;
  using size_type = string_view_type::size_type;

private:
  using index_type = std::ptrdiff_t;
  using deref_fn = iterator::deref_fn;
  using advance_fn = void const *(*)(void const *base,
                                     size_type index) noexcept;

  struct [[nodiscard]] operations {
    deref_fn read;
    advance_fn advance;
  };

  // One function per ELEMENT type, not per container type.
  template <typename T>
  static constexpr operations ops_for{
      .read =
          +[](void const *base, index_type index) noexcept -> string_view_type {
        return string_view_type{static_cast<T const *>(base)[index]};
      },
      .advance = +[](void const *p, size_type n) noexcept -> void const * {
        return static_cast<T const *>(p) + n;
      },
  };

  void const *base = nullptr;
  size_type m_size = 0;
  operations ops = ops_for<string_view_type>;

public:
  constexpr basic_string_views() noexcept = default;

  // Lvalues only: don't silently retain a pointer into a temporary
  // vector. Precondition: size fits in ptrdiff_t and data addresses size
  // elements.
  template <typename R, typename P = decltype(std::data(std::declval<R &>())),
            typename T = std::remove_cvref_t<std::remove_pointer_t<P>>>
    requires std::is_pointer_v<P> && std::convertible_to<P, T const *> &&
                 std::convertible_to<T const &, string_view_type> &&
                 std::is_nothrow_constructible_v<string_view_type, T const &> &&
                 requires(R &rng) { std::size(rng); }
  constexpr basic_string_views(R &rng) noexcept(
      noexcept(std::data(rng)) &&
      noexcept(static_cast<index_type>(std::size(rng))))
      : base(std::data(rng)), m_size(static_cast<index_type>(std::size(rng))),
        ops(ops_for<T>) {}

  template <typename P = iterator,
            typename T = std::remove_cvref_t<std::remove_pointer_t<P>>>
  constexpr basic_string_views(P const begin, size_type const len) noexcept
      : base(begin), m_size(len), ops(ops_for<T>) {}

  constexpr basic_string_views(basic_string_views const &) = default;
  constexpr basic_string_views(basic_string_views &&) noexcept = default;
  constexpr basic_string_views &operator=(basic_string_views const &) = default;
  constexpr basic_string_views &
  operator=(basic_string_views &&) noexcept = default;
  constexpr ~basic_string_views() noexcept = default;

  constexpr iterator begin() const noexcept { return {base, 0, ops.read}; }

  constexpr iterator end() const noexcept { return {base, m_size, ops.read}; }

  [[nodiscard]] constexpr size_type size() const noexcept { return m_size; }

  [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }

  [[nodiscard]] string_view_type operator[](index_type index) const noexcept {
    return ops.read(base, index);
  }

  [[nodiscard]] string_view_type at(index_type index) const {
    if (index >= static_cast<index_type>(m_size)) [[unlikely]] {
      throw std::out_of_range("basic_string_views::at: index out of range");
    }
    return ops.read(base, index);
  }

  [[nodiscard]] string_view_type back() const noexcept {
    assert(m_size > 0);
    return ops.read(base, m_size - 1);
  }

  [[nodiscard]] string_view_type front() const noexcept {
    assert(m_size > 0);
    return ops.read(base, 0);
  }

  [[nodiscard]] void const *data() const noexcept { return base; }

  [[nodiscard]] constexpr basic_string_views
  first(size_type const count) const noexcept {
    assert(count <= m_size);
    auto that = *this;
    that.m_size = count;
    return that;
  }

  [[nodiscard]] constexpr basic_string_views
  last(size_type count) const noexcept {
    assert(count <= m_size);
    auto that = *this;
    if (count != m_size) [[likely]] {
      that.base = ops->advance(base, m_size - count);
    }
    that.m_size = count;
    return that;
  }

  template <class F> void for_each(F &&func) const {
    if (ops.read == &ops_for<string_view_type>.read) {
      auto const *ptr = static_cast<string_view_type const *>(base);
      for (index_type i = 0; i < m_size; ++i) {
        func(string_view_type{ptr[i]});
      }
    } else if (ops.read == &ops_for<string_type>.read) {
      auto const *ptr = static_cast<string_type const *>(base);
      for (index_type i = 0; i < m_size; ++i) {
        func(string_view_type{ptr[i]});
      }
    } else {
      for (index_type i = 0; i < m_size; ++i) {
        func(ops.read(base, i));
      }
    }
  }
};

using string_views = basic_string_views<>;

} // namespace v14

// Include or paste the definitions of v1 through v7 from main.cxx here,
// or assume they are included via header. For self-containment, we assume
// the namespace definitions are available.

// Helper to generate a vector of strings of a given size
std::vector<std::string> make_string_vector(std::size_t count) {
  std::vector<std::string> v;
  v.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    v.push_back("item_" + std::to_string(i));
  }
  return v;
}

// -----------------------------------------------------------------------------
// Benchmark Fixture for Parameterized Counts
// -----------------------------------------------------------------------------
class StringViewsBenchmark : public benchmark::Fixture {
public:
  std::vector<std::string> strings;
  std::vector<std::string_view> string_views_storage;
  std::array<std::string, 8> arr_string{"one",  "two", "three", "four",
                                        "five", "six", "seven", "eight"};
  std::array<std::string_view, 8> arr_string_view{
      "one", "two", "three", "four", "five", "six", "seven", "eight"};
  std::span<std::string const> span_string_const;
  std::span<std::string_view const> span_string_view_const;

  void SetUp(const benchmark::State &state) override {
    const auto size = static_cast<std::size_t>(state.range(0));
    strings = make_string_vector(size);
    string_views_storage.reserve(size);
    for (const auto &s : strings) {
      string_views_storage.emplace_back(s);
    }
    span_string_const = std::span<std::string const>(strings);
    span_string_view_const =
        std::span<std::string_view const>(string_views_storage);
  }

  void TearDown(const benchmark::State &) override {
    strings.clear();
    string_views_storage.clear();
  }
};

// -----------------------------------------------------------------------------
// 1. Normal Looping Benchmarks (The 6 requested baselines)
// -----------------------------------------------------------------------------

BENCHMARK_DEFINE_F(StringViewsBenchmark,
                   Baseline_VectorString)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    for (const std::string &s : strings) {
      std::string_view sv{s};
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark,
                   Baseline_VectorStringView)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    for (std::string_view sv : string_views_storage) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark,
                   Baseline_ArrayString)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    for (const std::string &s : arr_string) {
      std::string_view sv{s};
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark,
                   Baseline_ArrayStringView)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    for (std::string_view sv : arr_string_view) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark,
                   Baseline_SpanString)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    std::span<std::string const> sp{strings};
    for (const std::string &s : sp) {
      std::string_view sv{s};
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark,
                   Baseline_SpanStringView)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    std::span<std::string_view const> sp{string_views_storage};
    for (std::string_view sv : sp) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

// -----------------------------------------------------------------------------
// 2. Custom Variants (v1, v2, v3, v5, v6, v7) Benchmarks
// -----------------------------------------------------------------------------

BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V1)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v1::string_views views(strings);
    for (std::string_view sv : views) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

// BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V2)(benchmark::State &state)
// {
//   for (auto _ : state) {
//     std::size_t hash_sum = 0;
//     v2::string_views views(strings);
//     for (std::string_view sv : views) {
//       benchmark::DoNotOptimize(hash_sum += sv.size());
//     }
//     benchmark::ClobberMemory();
//   }
// }

BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V3)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v3::string_views views(strings);
    for (std::string_view sv : views) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V5)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v5::string_views views(span_string_const, v5::as_string_view{});
    for (std::string_view sv : views) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V6)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v6::string_views views(strings);
    for (std::string_view sv : views) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V7)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v7::string_views views(span_string_const);
    for (std::string_view sv : views) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V8)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v8::string_views views(span_string_const);
    for (std::string_view sv : views) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V9)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v9::string_views views(span_string_const);
    for (std::string_view sv : views) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V10)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v10::string_views views(span_string_const);
    for (std::string_view sv : views) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V11)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v11::string_views views(span_string_const);
    for (std::string_view sv : views) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark,
                   Variant_V11_ForEach)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v11::string_views views(span_string_const);
    views.for_each([&](std::string_view sv) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    });
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V12)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v12::string_views views(span_string_const);
    for (std::string_view sv : views) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V13)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v13::string_views views(span_string_const);
    for (std::string_view sv : views) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_DEFINE_F(StringViewsBenchmark, Variant_V14)(benchmark::State &state) {
  for (auto _ : state) {
    std::size_t hash_sum = 0;
    v14::string_views views(span_string_const);
    for (std::string_view sv : views) {
      benchmark::DoNotOptimize(hash_sum += sv.size());
    }
    benchmark::ClobberMemory();
  }
}

// Register benchmarks with argument sizes (e.g., 8, 64, 512 elements)
BENCHMARK_REGISTER_F(StringViewsBenchmark, Baseline_VectorString)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Baseline_VectorStringView)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Baseline_ArrayString)
    ->Arg(8); // fixed size 8
BENCHMARK_REGISTER_F(StringViewsBenchmark, Baseline_ArrayStringView)
    ->Arg(8); // fixed size 8
BENCHMARK_REGISTER_F(StringViewsBenchmark, Baseline_SpanString)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Baseline_SpanStringView)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);

BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V1)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
// BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V2)
//     ->Arg(8)
//     ->Arg(64)
//     ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V3)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V5)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V6)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V7)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V8)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V9)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V10)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V11)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V11_ForEach)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V12)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V13)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
BENCHMARK_REGISTER_F(StringViewsBenchmark, Variant_V14)
    ->Arg(8)
    ->Arg(64)
    ->Arg(512);
