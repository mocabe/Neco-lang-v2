// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

#include <type_traits>

namespace TORI_NS::detail {

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

} // namespace TORI_NS::detail