// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "object_ptr.hpp"
#include "object_cast.hpp"
#include "dynamic_typing.hpp"
#include "bad_value_cast.hpp"

#include <exception>

namespace TORI_NS::detail {

  namespace interface {

    /// value_cast
    ///
    /// dynamically cast object to specified value type.
    /// \throws bad_value_cast when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<propagate_const_t<T, U>>
      value_cast(const object_ptr<U>& obj)
    {
      if (likely(obj && has_type<T>(obj))) {
        using To = typename decltype(
          get_object_type(normalize_specifier(type_c<T>)))::type;
        return static_object_cast<propagate_const_t<To, U>>(obj);
      }
      throw bad_value_cast(obj ? get_type(obj) : nullptr, object_type<T>());
    }

    /// value_cast
    ///
    /// dynamically cast object to specified value type.
    /// \throws bad_value_cast when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<propagate_const_t<T, U>>
      value_cast(object_ptr<U>&& obj)
    {
      if (likely(obj && has_type<T>(obj))) {
        using To = typename decltype(
          get_object_type(normalize_specifier(type_c<T>)))::type;
        return static_object_cast<propagate_const_t<To, U>>(std::move(obj));
      }
      throw bad_value_cast(obj ? get_type(obj) : nullptr, object_type<T>());
    }

    /// value_cast_if
    ///
    /// dynamically cast object to specified value type.
    /// \returns nullptr when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<propagate_const_t<T, U>>
      value_cast_if(const object_ptr<U>& obj) noexcept
    {
      if (likely(obj && has_type<T>(obj))) {
        using To = typename decltype(
          get_object_type(normalize_specifier(type_c<T>)))::type;
        return static_object_cast<propagate_const_t<To, U>>(obj);
      }
      return nullptr;
    }

    /// value_cast_if
    ///
    /// dynamically cast object to specified value type.
    /// \returns nullptr when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<propagate_const_t<T, U>>
      value_cast_if(object_ptr<U>&& obj) noexcept
    {
      if (likely(obj && has_type<T>(obj))) {
        using To = typename decltype(
          get_object_type(normalize_specifier(type_c<T>)))::type;
        return static_object_cast<propagate_const_t<To, U>>(std::move(obj));
      }
      return nullptr;
    }

  } // namespace interface

} // namespace TORI_NS::detail

// BadValueCast
TORI_DECL_TYPE(BadValueCast)