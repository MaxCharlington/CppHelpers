#pragma once

#include <concepts>
#include <utility>
#include <type_traits>

// A type that can be implicitly converted to *anything*
struct Anything {
    template <typename T>
    operator T() const; // We don't need to define this function
};

namespace detail {
  template <typename T, typename Is, typename=void>
  struct is_aggregate_constructible_from_n_impl
    : std::false_type{};

  template <typename T, std::size_t...Is>
  struct is_aggregate_constructible_from_n_impl<
    T,
    std::index_sequence<Is...>,
    std::void_t<decltype(T{(void(Is),Anything{})...})>
  > : std::true_type{};
} // namespace detail

template <typename T, std::size_t N>
using is_aggregate_constructible_from_n = detail::is_aggregate_constructible_from_n_impl<T,std::make_index_sequence<N>>;

namespace detail {
  template <std::size_t Min, std::size_t Range, template <std::size_t N> class target>
  struct maximize
    : std::conditional_t<
        maximize<Min, Range/2, target>{} == (Min+Range/2)-1,
        maximize<Min+Range/2, (Range+1)/2, target>,
        maximize<Min, Range/2, target>
      >{};
  template <std::size_t Min, template <std::size_t N> class target>
  struct maximize<Min, 1, target>
    : std::conditional_t<
        target<Min>{},
        std::integral_constant<std::size_t,Min>,
        std::integral_constant<std::size_t,Min-1>
      >{};
  template <std::size_t Min, template <std::size_t N> class target>
  struct maximize<Min, 0, target>
    : std::integral_constant<std::size_t,Min-1>
  {};

  template <typename T>
  struct construct_searcher {
    template<std::size_t N>
    using result = is_aggregate_constructible_from_n<T, N>;
  };
}

template <typename T, std::size_t Cap=32>
using constructor_arity = detail::maximize< 0, Cap, detail::construct_searcher<T>::template result >;

namespace detail {
  template <typename T, typename Fn>
  auto for_each_impl(T&& agg, Fn&& fn, std::integral_constant<std::size_t,0>) -> void
  {
    // do nothing (0 members)
  }

  template <typename T, typename Fn>
  auto for_each_impl(T& agg, Fn&& fn, std::integral_constant<std::size_t, 1>) -> void
  {
    auto& [m0] = agg;

    fn(m0);
  }

  template <typename T, typename Fn>
  auto for_each_impl(T& agg, Fn&& fn, std::integral_constant<std::size_t, 2>) -> void
  {
    auto& [m0, m1] = agg;

    fn(m0); fn(m1);
  }

  template <typename T, typename Fn>
  auto for_each_impl(T& agg, Fn&& fn, std::integral_constant<std::size_t, 3>) -> void
  {
    auto& [m0, m1, m2] = agg;

    fn(m0); fn(m1); fn(m2);
  }
  template <typename T, typename Fn>
  auto for_each_impl(T& agg, Fn&& fn, std::integral_constant<std::size_t, 4>) -> void
  {
    auto& [m0, m1, m2, m3] = agg;

    fn(m0); fn(m1); fn(m2); fn(m3);
  }

  template <typename T, typename Fn>
  auto for_each_impl(T& agg, Fn&& fn, std::integral_constant<std::size_t, 5>) -> void
  {
    auto& [m0, m1, m2, m3, m4] = agg;

    fn(m0); fn(m1); fn(m2); fn(m3); fn(m4);
  }

  template <typename T, typename Fn>
  auto for_each_impl(T& agg, Fn&& fn, std::integral_constant<std::size_t, 6>) -> void
  {
    auto& [m0, m1, m2, m3, m4, m5] = agg;

    fn(m0); fn(m1); fn(m2); fn(m3); fn(m4); fn(m5);
  }

} // namespace detail

template <typename T, typename Fn>
void for_each_member(T& agg, Fn&& fn)
{
  detail::for_each_impl(agg, std::forward<Fn>(fn), constructor_arity<T>{});
}
