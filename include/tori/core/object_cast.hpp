// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

#include "object_ptr.hpp"

namespace TORI_NS::detail {

  /// static object cast
  template <class T, class U>
  [[nodiscard]] object_ptr<T> static_object_cast(const object_ptr<U>& obj)
  {
    if (likely(!obj.is_static()))
      obj.head()->refcount.fetch_add();

    if constexpr (std::is_base_of_v<T, U>)
      return static_cast<T*>(obj.get());
    else if constexpr (std::is_base_of_v<U, T>)
      // downcast to proxy types (empty derived class from Object) using
      // static_cast IS undefined behaviour. reinterpret_cast is not UB when T
      // and U are layout compatible.
      return reinterpret_cast<T*>(obj.get());
    else
      static_assert(false_v<T, U>);
  }

  /// static object cast
  template <class T, class U>
  [[nodiscard]] object_ptr<T> static_object_cast(object_ptr<U>&& obj)
  {
    if constexpr (std::is_base_of_v<T, U>)
      return static_cast<T*>(obj.release());
    else if constexpr (std::is_base_of_v<U, T>)
      return reinterpret_cast<T*>(obj.release());
    else
      static_assert(false_v<T, U>);
  }

} // namespace TORI_NS::detail
