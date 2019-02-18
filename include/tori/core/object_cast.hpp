// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

#include "object_ptr.hpp"

namespace TORI_NS::detail {

  /// cast object_ptr without type check
  template <class T, class U>
  [[nodiscard]] object_ptr<T> static_object_cast(const object_ptr<U>& obj)
  {
    if (likely(obj && !obj.is_static()))
      obj.head()->refcount.fetch_add();
    return static_cast<T*>(obj.get());
  }

  /// cast object_ptr without type check
  template <class T, class U>
  [[nodiscard]] object_ptr<T> static_object_cast(object_ptr<U>&& obj)
  {
    return static_cast<T*>(obj.release());
  }

  /// cast object_ptr_generic without type check
  template <class T>
  [[nodiscard]] object_ptr<T> static_object_cast(const object_ptr_generic& obj)
  {
    assert(_get_storage(obj).has_pointer_type<T>());
    object_ptr<T> ret = nullptr;
    // copy storage
    _get_storage(ret) = _get_storage(obj);
    // add refcount
    if (likely(ret && !ret.is_static()))
      ret.head()->refcount.fetch_add();
    return ret;
  }

  /// cast object_ptr_generic without type check
  template <class T>
  [[nodiscard]] object_ptr<T> static_object_cast(object_ptr_generic&& obj)
  {
    assert(_get_storage(obj).has_pointer_type<T>());
    object_ptr<T> ret = nullptr;
    // copy storage
    _get_storage(ret) = _get_storage(obj);
    // take over ownership
    _get_storage(obj) = {nullptr};
    return ret;
  }

  /// cast object_ptr_generic without type check
  template <class T>
  [[nodiscard]] immediate<T>
    static_immediate_cast(const object_ptr_generic& obj)
  {
    assert(_get_storage(obj).has_immediate_type<T>());
    immediate<T> ret;
    // copy storage
    _get_storage(ret) = _get_storage(obj);
    return ret;
  }

  /// cast object_ptr_generic without type check
  template <class T>
  [[nodiscard]] immediate<T> static_immediate_cast(object_ptr_generic&& obj)
  {
    assert(_get_storage(obj).has_immediate_type<T>());
    immediate<T> ret;
    // copy storage
    _get_storage(ret) = _get_storage(obj);
    return ret;
  }

  /// cast object_ptr_generic without type check
  template <class T>
  [[nodiscard]] auto static_auto_cast(const object_ptr_generic& obj)
  {
    if constexpr (is_transfarable_immediate(type_c<T>))
      return static_immediate_cast<T>(obj);
    else
      return static_object_cast<T>(obj);
  }

  /// cast object_ptr_generic without type check
  template <class T>
  [[nodiscard]] auto static_auto_cast(object_ptr_generic&& obj)
  {
    if constexpr (is_transfarable_immediate(type_c<T>))
      return static_immediate_cast<T>(std::move(obj));
    else
      return static_object_cast<T>(std::move(obj));
  }

} // namespace TORI_NS::detail
