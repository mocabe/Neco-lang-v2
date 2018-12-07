#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file closure cast

#include "DynamicTypeUtil.hpp"
#include <exception>

namespace TORI_NS::detail {

  /// bad_closure_cast exception
  class bad_closure_cast : public std::logic_error
  {
  public:
    explicit bad_closure_cast(
      object_ptr<const Type> from,
      object_ptr<const Type> to)
      : std::logic_error("bad_closure_cast")
      , m_from {std::move(from)}
      , m_to {std::move(to)}
    {
    }

    /// from
    object_ptr<const Type> from() const
    {
      return m_from;
    }

    /// to
    object_ptr<const Type> to() const
    {
      return m_to;
    }

  private:
    /// cast from
    object_ptr<const Type> m_from;
    /// cast to
    object_ptr<const Type> m_to;
  };

  /// BadClosureCastValue
  struct BadClosureCastValue
  {
    object_ptr<const Type> from;
    object_ptr<const Type> to;
  };

  namespace interface {

    /// Exception object for bad_closure_cast exception
    using BadClosureCast = BoxedHeapObject<detail::BadClosureCastValue>;

  } // namespace interface

  namespace interface {

    /// closure_cast
    ///
    /// dynamically cast object to specified closure type.
    /// \throws bad_value_cast when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T> closure_cast(const object_ptr<U>& obj)
    {
      static_assert(has_tm_closure_v<T>, "T is not closure type");
      assert(obj);
      auto o = object_ptr<>(obj);
      if (likely(has_type<T>(o))) {
        // +1
        if (o.get())
          o.head()->refcount.fetch_add();
        return static_cast<T*>(o.get());
      }
      throw bad_closure_cast {get_type(o), object_type<T>()};
    }

    /// closure_cast
    ///
    /// dynamically cast object to specified closure type.
    /// \throws bad_value_cast when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T> closure_cast(object_ptr<U>&& obj)
    {
      static_assert(has_tm_closure_v<T>, "T is not closure type");
      assert(obj);
      auto o = object_ptr<>(std::move(obj));
      if (likely(has_type<T>(o))) {
        // move
        auto r = static_cast<T*>(o.m_ptr);
        o.m_ptr = nullptr;
        return r;
      }
      throw bad_closure_cast {get_type(o), object_type<T>()};
    }

    /// closure_cast_if
    ///
    /// dynamically cast object to specified closure type.
    /// \returns nullptr when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T>
      closure_cast_if(const object_ptr<U>& obj) noexcept
    {
      static_assert(has_tm_closure_v<T>, "T is not closure type");
      assert(obj);
      auto o = object_ptr<>(obj);
      if (has_type<T>(o)) {
        // +1
        if (o.get())
          o.head()->refcount.fetch_add();
        return static_cast<T*>(o.get());
      }
      return nullptr;
    }

    /// closure_cast_if
    ///
    /// dynamically cast object to specified closure type.
    /// \returns nullptr when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T> closure_cast_if(object_ptr<U>&& obj) noexcept
    {
      static_assert(has_tm_closure_v<T>, "T is not closure type");
      assert(obj);
      auto o = object_ptr<>(std::move(obj));
      if (has_type<T>(o)) {
        // move
        auto r = static_cast<T*>(o.m_ptr);
        o.m_ptr = nullptr;
        return r;
      }
      return nullptr;
    }

  } // namespace interface

} // namespace TORI_NS::detail

namespace TORI_NS {
  TORI_DECL_TYPE(BadClosureCast)
} // namespace TORI_NS