// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

namespace TORI_NS::detail {

  /// constexpr offsetof
  template <typename T1, typename T2>
  struct offset_of_member_impl
  {
    union U
    {
      U()
        : c {}
      {
      }
      ~U()
      {
      }
      char c[sizeof(T2)];
      T2 o;
    };
    static U u;
    static constexpr uint64_t get(T1 T2::*member)
    {
      uint64_t i = 0;
      for (; i < sizeof(T2); ++i)
        if (((void*)&(u.c[i])) == &(u.o.*member))
          break;

      // g++ bug 67371 workaround
      if (i >= sizeof(T2))
        throw;
      else
        return i;
    }
  };
  template <class T1, class T2>
  typename offset_of_member_impl<T1, T2>::U offset_of_member_impl<T1, T2>::u {};

  /// get offset of member
  template <class T1, class T2>
  [[nodiscard]] constexpr uint64_t offset_of_member(T1 T2::*member)
  {
    return offset_of_member_impl<T1, T2>::get(member);
  }

} // namespace TORI_NS::detail
