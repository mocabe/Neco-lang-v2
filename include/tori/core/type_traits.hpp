// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

#include <type_traits>

namespace TORI_NS::detail {

  // ------------------------------------------
  // false_v

  template <class...>
  struct make_false
  {
    static constexpr bool value = false;
  };

  template <class... Ts>
  static constexpr bool false_v = make_false<Ts...>::value;

  // ------------------------------------------
  // is_complete

  template <class T>
  constexpr char is_complete_impl_func(int (*)[sizeof(T)]);

  template <class>
  constexpr long is_complete_impl_func(...);

  template <class T>
  struct is_complete_impl
  {
    static constexpr bool value = sizeof(is_complete_impl_func<T>(0)) == 1;
  };

  template <class T>
  constexpr bool is_complete_v = is_complete_impl<T>::value;

  // ------------------------------------------
  // propagate_const

  template <class T, class U>
  struct propagate_const
  {
    using type = T;
  };

  template <class T, class U>
  struct propagate_const<T, const U>
  {
    using type = std::add_const_t<T>;
  };

  /// conditionally add const to T depending on constness of U
  template <class T, class U>
  using propagate_const_t = typename propagate_const<T, U>::type;

} // namespace TORI_NS::detail