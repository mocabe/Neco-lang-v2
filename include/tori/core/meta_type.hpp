#pragma once

#include "../config/config.hpp"

#include <type_traits>

namespace TORI_NS::detail {

  /// meta_type
  template <class T>
  struct meta_type
  {
    using type = T;
  };

  /// meta_type constant
  template <class T>
  static constexpr meta_type<T> type_c {};

  /// operator==
  template <class T1, class T2>
  constexpr auto operator==(meta_type<T1>, meta_type<T2>)
  {
    if constexpr (std::is_same_v<T1, T2>)
      return std::true_type {};
    else
      return std::false_type {};
  }

  /// operator!=
  template <class T1, class T2>
  constexpr auto operator!=(meta_type<T1> lhs, meta_type<T2> rhs)
  {
    return !(lhs == rhs);
  }

  /// meta bool

  /// true_c
  static constexpr std::true_type true_c {};
  /// false_c
  static constexpr std::false_type false_c {};
}