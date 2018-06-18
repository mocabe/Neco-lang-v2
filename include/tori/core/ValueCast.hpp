// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file value cast function

#include "DynamicTypeUtil.hpp"
#include <exception>

namespace TORI_NS::detail {

  /// bad_value_cast exception
  class bad_value_cast : public std::logic_error {
  public:
    explicit bad_value_cast(
      ObjectPtr<const Type> from, ObjectPtr<const Type> to)
      : std::logic_error("bad_value_cast")
      , m_from{std::move(from)}
      , m_to{std::move(to)} {}
    /// get from
    ObjectPtr<const Type> from() const {
      return m_from;
    }
    /// get to
    ObjectPtr<const Type> to() const {
      return m_to;
    }

  private:
    /// cast from
    ObjectPtr<const Type> m_from;
    /// cast to
    ObjectPtr<const Type> m_to;
  };

  struct BadValueCastValue {
    ObjectPtr<const Type> from;
    ObjectPtr<const Type> to;
  };

  namespace interface {
    /// Exception object for bad_value_cast exception
    using BadValueCast = BoxedHeapObject<detail::BadValueCastValue>;
  } // namespace interface

  namespace interface {
    /// value_cast
    ///
    /// dynamically cast object to specified value type.
    /// \throws bad_value_cast when fail.
    template <class T>
    [[nodiscard]] ObjectPtr<T> value_cast(const ObjectPtr<>& obj) {
      static_assert(has_TmValue_v<T>, "T is not value type");
      assert(obj);
      if (has_type<T>(obj)) {
        // +1
        if (obj.head()) ++(obj.head()->refcount.atomic);
        return static_cast<T*>(obj.head());
      } else {
        throw bad_value_cast{object_type<T>::get(), get_type(obj)};
      }
    }

    /// value_cast
    ///
    /// dynamically cast object to specified value type.
    /// \throws bad_value_cast when fail.
    template <class T>
    [[nodiscard]] ObjectPtr<T> value_cast(ObjectPtr<>&& obj) {
      static_assert(has_TmValue_v<T>, "T is not value type");
      assert(obj);
      if (has_type<T>(obj)) {
        // move
        auto r = static_cast<T*>(obj.m_ptr);
        obj.m_ptr = nullptr;
        return r;
      } else {
        throw bad_value_cast{object_type<T>::get(), get_type(obj)};
      }
    }

    /// value_cast_if
    ///
    /// dynamically cast object to specified value type.
    /// \returns nullptr when fail.
    template <class T>
    [[nodiscard]] ObjectPtr<T> value_cast_if(const ObjectPtr<>& obj) noexcept {
      static_assert(has_TmValue_v<T>, "T is not value type");
      assert(obj);
      if (has_type<T>(obj)) {
        // +1
        if (obj.head()) ++(obj.head()->refcount.atomic);
        return static_cast<T*>(obj.head());
      } else {
        return nullptr;
      }
    }

    /// value_cast_if
    ///
    /// dynamically cast object to specified value type.
    /// \returns nullptr when fail.
    template <class T>
    [[nodiscard]] ObjectPtr<T> value_cast_if(ObjectPtr<>&& obj) noexcept {
      static_assert(has_TmValue_v<T>, "T is not value type");
      assert(obj);
      if (has_type<T>(obj)) {
        // move
        auto r = static_cast<T*>(obj.m_ptr);
        obj.m_ptr = nullptr;
        return r;
      } else {
        return nullptr;
      }
    }
  } // namespace interface
} // namespace TORI_NS::detail

namespace TORI_NS {
  TORI_DECL_TYPE(BadValueCast)
} // namespace TORI_NS