// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

#include "meta_type.hpp"

namespace TORI_NS::detail {

  /// meta_tuple
  template <class... Ts>
  struct meta_tuple
  {
    constexpr size_t size() const
    {
      return sizeof...(Ts);
    }
  };

  /// meta_tuple constant
  template <class... Ts>
  static constexpr meta_tuple<Ts...> tuple_c {};

  template <class... Ts1, class... Ts2>
  constexpr auto equal(meta_tuple<Ts1...> t1, meta_tuple<Ts2...> t2)
  {
    if constexpr (std::is_same_v<decltype(t1), decltype(t2)>)
      return true_c;
    else
      return false_c;
  }

  /// operator==
  template <class... Ts1, class... Ts2>
  constexpr auto operator==(meta_tuple<Ts1...> t1, meta_tuple<Ts2...> t2)
  {
    return equal(t1, t2);
  }

  /// operaotr!=
  template <class... Ts1, class... Ts2>
  constexpr auto operator!=(meta_tuple<Ts1...> t1, meta_tuple<Ts2...> t2)
  {
    return std::bool_constant<!(t1 == t2)> {};
  }

  // ------------------------------------------
  // make

  template <class... Ts>
  constexpr auto make_tuple(meta_type<Ts>...)
  {
    return tuple_c<Ts...>;
  }

  // ------------------------------------------
  // get

  template <size_t It, size_t Idx, class H, class... Ts>
  constexpr auto tuple_get_impl(meta_tuple<H, Ts...>)
  {
    if constexpr (It == Idx)
      return type_c<H>;
    else
      return tuple_get_impl<It + 1, Idx>(tuple_c<Ts...>);
  }

  template <size_t Idx, class... Ts>
  constexpr auto get(meta_tuple<Ts...> tuple)
  {
    static_assert(Idx < tuple.size(), "Index out of range");
    return tuple_get_impl<0, Idx>(tuple);
  }

  // ------------------------------------------
  // head

  template <class H, class... Ts>
  constexpr auto head(meta_tuple<H, Ts...>)
  {
    return type_c<H>;
  }

  // ------------------------------------------
  // tail

  template <class H, class... Ts>
  constexpr auto tail(meta_tuple<H, Ts...>)
  {
    return tuple_c<Ts...>;
  }

  constexpr auto tail(meta_tuple<>)
  {
    return tuple_c<>;
  }

  // ------------------------------------------
  // last

  template <class... Ts>
  constexpr auto last(meta_tuple<Ts...> t)
  {
    if constexpr (empty(t))
      static_assert(false_v<Ts...>, "Empty tuple");

    if constexpr (t.size() <= 1)
      return head(t);
    else
      return last(tail(t));
  }

  // ------------------------------------------
  // empty

  template <class... Ts>
  constexpr auto empty(meta_tuple<Ts...>)
  {
    if constexpr (sizeof...(Ts) == 0)
      return true_c;
    else
      return false_c;
  }

  // ------------------------------------------
  // append

  template <class E, class... Ts>
  constexpr auto append(meta_type<E>, meta_tuple<Ts...>)
  {
    return tuple_c<Ts..., E>;
  }

  template <class E>
  constexpr auto append(meta_type<E>, meta_tuple<>)
  {
    return tuple_c<E>;
  }

  // ------------------------------------------
  // concat

  template <class... Ts1, class... Ts2>
  constexpr auto concat(meta_tuple<Ts1...>, meta_tuple<Ts2...>)
  {
    return tuple_c<Ts1..., Ts2...>;
  }

  // ------------------------------------------
  // contains

  template <class E, class... Ts>
  constexpr auto contains(meta_type<E> e, meta_tuple<Ts...> tuple)
  {
    (void)e;
    (void)tuple;

    if constexpr (empty(tuple)) {
      return false_c;
    } else if constexpr (e == head(tuple)) {
      return true_c;
    } else {
      return contains(e, tail(tuple));
    }
  }

  // ------------------------------------------
  // remove_last

  template <class... Ts>
  constexpr auto remove_last(meta_tuple<Ts...> tuple)
  {
    if constexpr (tuple.size() <= 1)
      return tuple_c<>;
    else
      return concat(make_tuple(head(tuple)), remove_last(tail(tuple)));
  }

} // namespace TORI_NS::detail