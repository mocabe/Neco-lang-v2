#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file value cast function

#include "DynamicTypeUtil.hpp"
#include <exception>

namespace TORI_NS::detail {

  /// bad_value_cast exception
  class bad_value_cast : public std::logic_error {
  public:
    explicit bad_value_cast(
      object_ptr<const Type> from, object_ptr<const Type> to)
      : std::logic_error("bad_value_cast")
      , m_from{std::move(from)}
      , m_to{std::move(to)} {}
    /// get from
    object_ptr<const Type> from() const {
      return m_from;
    }
    /// get to
    object_ptr<const Type> to() const {
      return m_to;
    }

  private:
    /// cast from
    object_ptr<const Type> m_from;
    /// cast to
    object_ptr<const Type> m_to;
  };

  struct BadValueCastValue {
    object_ptr<const Type> from;
    object_ptr<const Type> to;
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
    template <class T, class U>
    [[nodiscard]] object_ptr<T> value_cast(const object_ptr<U>& obj) {
      static_assert(!has_tm_closure_v<T>, "T is not value type");
      assert(obj);
      auto o = object_ptr<>(obj);
      if (likely(has_type<T>(o))) {
        // +1
        if (o.get()) ++(o.head()->refcount.atomic);
        return static_cast<T*>(o.get());
      } else {
        throw bad_value_cast{object_type<T>(), get_type(o)};
      }
    }

    /// value_cast
    ///
    /// dynamically cast object to specified value type.
    /// \throws bad_value_cast when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T> value_cast(object_ptr<U>&& obj) {
      static_assert(!has_tm_closure_v<T>, "T is not value type");
      assert(obj);
      auto o = object_ptr<>(std::move(obj));
      if (likely(has_type<T>(o))) {
        // move
        auto r = static_cast<T*>(o.m_ptr);
        o.m_ptr = nullptr;
        return r;
      } else {
        throw bad_value_cast{object_type<T>(), get_type(o)};
      }
    }

    /// value_cast_if
    ///
    /// dynamically cast object to specified value type.
    /// \returns nullptr when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T> value_cast_if(
      const object_ptr<U>& obj) noexcept {
      static_assert(!has_tm_closure_v<T>, "T is not value type");
      assert(obj);
      auto o = object_ptr<>(obj);
      if (has_type<T>(o)) {
        // +1
        if (o.get()) ++(o.head()->refcount.atomic);
        return static_cast<T*>(o.get());
      } else {
        return nullptr;
      }
    }

    /// value_cast_if
    ///
    /// dynamically cast object to specified value type.
    /// \returns nullptr when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T> value_cast_if(object_ptr<U>&& obj) noexcept {
      static_assert(!has_tm_closure_v<T>, "T is not value type");
      assert(obj);
      auto o = object_ptr<>(std::move(obj));
      if (has_type<T>(o)) {
        // move
        auto r = static_cast<T*>(o.m_ptr);
        o.m_ptr = nullptr;
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