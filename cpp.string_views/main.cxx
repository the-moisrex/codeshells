import standard;

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
        explicit string_views(U&& range)
            : view_(base_view{std::forward<U>(range)}, &to_view) {}

        auto begin() const { return view_.begin(); }
        auto end() const { return view_.end(); }

        auto size() const
            requires std::ranges::sized_range<base_view>
        {
            return view_.size();
        }
    };

    template <class R>
    string_views(R&&) -> string_views<std::views::all_t<R>>;

} // namespace v1

namespace v2 {
    struct as_string_view {
        template <class S>
            requires std::convertible_to<S, std::string_view>
        std::string_view operator()(S&& s) const noexcept {
            return std::string_view{std::forward<S>(s)};
        }
    };

    template <class R>
        requires std::ranges::input_range<R> &&
                 std::convertible_to<std::ranges::range_reference_t<R>,
                                     std::string_view>
    class string_views
        : public std::ranges::transform_view<std::views::all_t<R>,
                                             as_string_view> {
        using base_view = std::views::all_t<R>;
        using view_type =
            std::ranges::transform_view<base_view, as_string_view>;

      public:
        template <class U>
            requires std::constructible_from<base_view, U>
        explicit string_views(U&& range)
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
            using pointer = const std::string_view*;
            using reference = std::string_view;

            struct operations {
                bool (*next)(void*, value_type&);
                bool (*done)(const void*);
            };

            void* impl_ptr = nullptr;
            const operations* ops_ = nullptr;
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

            iterator& operator++() {
                advance();
                return *this;
            }

            void operator++(int) { advance(); }

            friend bool operator==(const iterator& it,
                                   std::default_sentinel_t) noexcept {
                if (!it.has_value_)
                    return true;
                if (it.ops_ && it.ops_->done(it.impl_ptr))
                    return true;
                return false;
            }

            friend bool operator==(std::default_sentinel_t,
                                   const iterator& it) noexcept {
                return it == std::default_sentinel_t{};
            }
        };

      private:
        static constexpr std::size_t storage_size = 64;
        static constexpr std::size_t storage_align = alignof(std::max_align_t);

        struct range_operations {
            bool (*next)(void*, value_type&);
            bool (*done)(const void*);
            void (*destroy)(void*) noexcept;
            void (*move)(void* from, void* to) noexcept;
        };

        alignas(storage_align) std::byte storage_[storage_size]{};
        const range_operations* ops_ = nullptr;

        template <class I, class S>
        struct cursor {
            I current;
            S end;

            bool next(value_type& out) {
                if (current == end)
                    return false;
                out = value_type{*current};
                ++current;
                return true;
            }

            bool done() const { return current == end; }
        };

        template <class C>
        static const range_operations& ops_for() {
            static constexpr range_operations table{
                [](void* p, value_type& out) -> bool {
                    return static_cast<C*>(p)->next(out);
                },
                [](const void* p) -> bool {
                    return static_cast<const C*>(p)->done();
                },
                [](void* p) noexcept { std::destroy_at(static_cast<C*>(p)); },
                [](void* from, void* to) noexcept {
                    auto* src = static_cast<C*>(from);
                    std::construct_at(static_cast<C*>(to), std::move(*src));
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
            requires std::ranges::input_range<R&> &&
                     std::convertible_to<std::ranges::range_reference_t<R&>,
                                         value_type>
        explicit string_views(R& range) {
            using I = std::ranges::iterator_t<R&>;
            using S = std::ranges::sentinel_t<R&>;
            using C = cursor<I, S>;

            static_assert(
                sizeof(C) <= storage_size,
                "string_views: iterator state exceeds inline storage");
            static_assert(alignof(C) <= storage_align,
                          "string_views: iterator state is over-aligned");
            static_assert(
                std::is_nothrow_move_constructible_v<C>,
                "string_views: iterator and sentinel must be nothrow movable");

            std::construct_at(
                reinterpret_cast<C*>(storage_),
                C{std::ranges::begin(range), std::ranges::end(range)});

            ops_ = &ops_for<C>();
        }

        string_views(const string_views&) = delete;
        string_views& operator=(const string_views&) = delete;

        string_views(string_views&& other) noexcept
            : ops_(std::exchange(other.ops_, nullptr)) {
            if (ops_)
                ops_->move(other.storage_, storage_);
        }

        string_views& operator=(string_views&& other) noexcept {
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
            it.ops_ = reinterpret_cast<const iterator::operations*>(ops_);
            it.advance();
            return it;
        }

        std::default_sentinel_t end() const noexcept {
            return std::default_sentinel;
        }
    };

} // namespace v3

namespace v4 {

    class string_views {
      public:
        using value_type = std::string_view;

        struct context {
            void* state = nullptr;
            bool (*next)(void* state, value_type& out) noexcept = nullptr;
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

            iterator& operator++() noexcept {
                advance();
                return *this;
            }

            void operator++(int) noexcept { advance(); }

            friend bool operator==(const iterator& it,
                                   std::default_sentinel_t) noexcept {
                return !it.has_value_;
            }

            friend bool operator==(std::default_sentinel_t,
                                   const iterator& it) noexcept {
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
            requires std::ranges::input_range<R&> &&
                     std::convertible_to<std::ranges::range_reference_t<R&>,
                                         value_type>
        explicit string_views(R& range) {
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
        [[nodiscard]] constexpr std::string_view
        operator()(S&& s) const noexcept {
            return std::string_view{std::forward<S>(s)};
        }
    };
    using string_views =
        std::ranges::transform_view<std::span<std::string const>,
                                    as_string_view>;
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

            constexpr iterator(const string_views* owner,
                               std::size_t index) noexcept
                : owner_(owner), index_(index) {}

            constexpr std::string_view operator*() const noexcept {
                return owner_->get_at(index_);
            }

            constexpr iterator& operator++() noexcept {
                ++index_;
                return *this;
            }

            constexpr iterator operator++(int) noexcept {
                iterator tmp = *this;
                ++index_;
                return tmp;
            }

            friend constexpr bool operator==(const iterator& a,
                                             const iterator& b) noexcept {
                return a.index_ == b.index_;
            }

            friend constexpr bool operator!=(const iterator& a,
                                             const iterator& b) noexcept {
                return a.index_ != b.index_;
            }

          private:
            const string_views* owner_;
            std::size_t index_;
        };

        // Constructor accepting any range whose elements are convertible to
        // std::string_view
        template <class Range>
            requires(!std::is_same_v<std::decay_t<Range>, string_views>) &&
                        std::ranges::range<Range> &&
                        std::convertible_to<
                            std::ranges::range_reference_t<Range>,
                            std::string_view>
        constexpr explicit string_views(Range& r) noexcept
            : range_ptr_(const_cast<void*>(static_cast<const void*>(&r))),
              size_fn([](const void* ptr) noexcept -> std::size_t {
                  auto& ref = *static_cast<Range*>(const_cast<void*>(ptr));
                  return static_cast<std::size_t>(std::ranges::distance(ref));
              }),
              get_fn([](const void* ptr,
                        std::size_t idx) noexcept -> std::string_view {
                  auto& ref = *static_cast<Range*>(const_cast<void*>(ptr));
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

        constexpr iterator end() const noexcept {
            return iterator{this, size()};
        }

      private:
        void* range_ptr_ = nullptr;
        std::size_t (*size_fn)(const void*) noexcept = nullptr;
        std::string_view (*get_fn)(const void*, std::size_t) noexcept = nullptr;

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
        [[nodiscard]] constexpr string_view_type
        operator()(S&& s) const noexcept {
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
        constexpr basic_string_views(R&& beg) noexcept : base_type{beg, {}} {}
    };

    using string_views = basic_string_views<>;
} // namespace v7

namespace v11 {

    // Const random-access iterator over a type-erased source of string_views.
    template <typename SV>
    class erased_sv_iterator {
      public:
        using value_type = SV;
        using difference_type = std::ptrdiff_t;
        using iterator_concept = std::random_access_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;

        // "deref(i) -> SV" where i is a byte offset from the source's begin().
        using deref_fn = SV (*)(const void* base, std::ptrdiff_t i) noexcept;

        constexpr erased_sv_iterator() = default;
        constexpr erased_sv_iterator(const void* base, std::ptrdiff_t i,
                                     deref_fn f) noexcept
            : base_{base}, i_{i}, f_{f} {}

        SV operator*() const noexcept { return f_(base_, i_); }
        SV operator[](difference_type n) const noexcept {
            return f_(base_, i_ + n);
        }

        erased_sv_iterator& operator++() noexcept {
            ++i_;
            return *this;
        }
        erased_sv_iterator operator++(int) noexcept {
            auto t = *this;
            ++i_;
            return t;
        }
        erased_sv_iterator& operator--() noexcept {
            --i_;
            return *this;
        }
        erased_sv_iterator operator--(int) noexcept {
            auto t = *this;
            --i_;
            return t;
        }
        erased_sv_iterator& operator+=(difference_type n) noexcept {
            i_ += n;
            return *this;
        }
        erased_sv_iterator& operator-=(difference_type n) noexcept {
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

        friend difference_type operator-(const erased_sv_iterator& a,
                                         const erased_sv_iterator& b) noexcept {
            return a.i_ - b.i_;
        }
        friend bool operator==(const erased_sv_iterator& a,
                               const erased_sv_iterator& b) noexcept {
            return a.i_ == b.i_;
        }
        friend auto operator<=>(const erased_sv_iterator& a,
                                const erased_sv_iterator& b) noexcept {
            return a.i_ <=> b.i_;
        }

      private:
        const void* base_ = nullptr;
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
        static string_view_type read(const void* base, index_type i) noexcept {
            return string_view_type{static_cast<const T*>(base)[i]};
        }

        const void* base_ = nullptr;
        index_type size_ = 0;
        deref_fn deref_ = &read<string_view_type>;

      public:
        constexpr basic_string_views() noexcept = default;

        // Lvalues only: don't silently retain a pointer into a temporary
        // vector. Precondition: size fits in ptrdiff_t and data addresses size
        // elements.
        template <class R, class P = decltype(std::data(std::declval<R&>())),
                  class T = std::remove_cv_t<std::remove_pointer_t<P>>>
            requires std::is_pointer_v<P> && std::convertible_to<P, const T*> &&
                         std::convertible_to<const T&, string_view_type> &&
                         std::is_nothrow_constructible_v<string_view_type,
                                                         const T&> &&
                         requires(R& r) { std::size(r); }
        constexpr basic_string_views(R& r) noexcept(
            noexcept(std::data(r)) &&
            noexcept(static_cast<index_type>(std::size(r))))
            : base_(std::data(r)), size_(static_cast<index_type>(std::size(r))),
              deref_(&read<T>) {}

        constexpr iterator begin() const noexcept { return {base_, 0, deref_}; }
        constexpr iterator end() const noexcept {
            return {base_, size_, deref_};
        }
        constexpr index_type size() const noexcept { return size_; }
        constexpr bool empty() const noexcept { return size_ == 0; }

        string_view_type operator[](index_type i) const noexcept {
            return deref_(base_, i);
        }

        template <class F>
        void for_each(F&& f) const {
            const auto n = size_;
            const auto b = base_;
            const auto read_one = deref_;

            if (read_one == &read<string_view_type>) {
                const auto* p = static_cast<const string_view_type*>(b);
                for (index_type i = 0; i < n; ++i)
                    f(string_view_type{p[i]});
            } else if (read_one == &read<string_type>) {
                const auto* p = static_cast<const string_type*>(b);
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

int main() {
    using namespace v11;

    std::vector<std::string> strings{"one", "two", "three", "four"};
    std::array<std::string_view, 4> statics{"one", "two", "three", "four"};
    string_views views{strings}; // stores a ref_view; no allocation
    string_views views2{statics};

    for (auto const str : views) {
        println("str: {}", str);
    }

    for (auto const str : views2) {
        println("statics: {}", str);
    }

    return 0;
}
