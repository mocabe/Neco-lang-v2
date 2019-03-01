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
    // add refcount
    if (likely(obj && !obj.is_static()))
      obj.head()->refcount.fetch_add();

    if constexpr (std::is_base_of_v<U, T> && sizeof(T) == sizeof(U))
      // downcast to proxy types (empty derived class from Object) using
      // static_cast IS undefined behaviour. But it seems that using
      // reinterpret_cast is not UB when T and U are layout compatible. Since
      // empty derived classes from standard layout class (i.e. Object) are also
      // standard lyaout (and these are layout compatible since no additional
      // members are introduced), we can use reinterpret_cast here.
      return reinterpret_cast<T*>(obj.get());
    else
      // it's undefined behavior if `obj` is not an actual object of T.
      return static_cast<T*>(obj.get());
  }

  /// static object cast
  template <class T, class U>
  [[nodiscard]] object_ptr<T> static_object_cast(object_ptr<U>&& obj)
  {
    if constexpr (std::is_base_of_v<U, T> && sizeof(T) == sizeof(U))
      return reinterpret_cast<T*>(obj.release());
    else
      return static_cast<T*>(obj.release());
  }

} // namespace TORI_NS::detail
