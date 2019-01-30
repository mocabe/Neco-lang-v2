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
    return static_cast<T*>(obj.get());
  }

  /// static object cast
  template <class T, class U>
  [[nodiscard]] object_ptr<T> static_object_cast(object_ptr<U>&& obj)
  {
    return static_cast<T*>(obj.release());
  }

} // namespace TORI_NS::detail
