#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file closure cast

#include "../config/config.hpp"
#include "object_ptr.hpp"
#include "object_cast.hpp"
#include "type_gen.hpp"
#include "dynamic_typing.hpp"

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
    const object_ptr<const Type>& from() const
    {
      return m_from;
    }

    /// to
    const object_ptr<const Type>& to() const
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
      static_assert(is_tm_closure(T::term), "T is not closure type");

      if (likely(obj && has_type<T>(obj))) {
        return static_object_cast<T>(obj);
      }
      throw bad_closure_cast(obj ? get_type(obj) : nullptr, object_type<T>());
    }

    /// closure_cast
    ///
    /// dynamically cast object to specified closure type.
    /// \throws bad_value_cast when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T> closure_cast(object_ptr<U>&& obj)
    {
      static_assert(is_tm_closure(T::term), "T is not closure type");

      if (likely(obj && has_type<T>(obj))) {
        return static_object_cast<T>(std::move(obj));
      }
      throw bad_closure_cast(obj ? get_type(obj) : nullptr, object_type<T>());
    }

    /// closure_cast_if
    ///
    /// dynamically cast object to specified closure type.
    /// \returns nullptr when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T>
      closure_cast_if(const object_ptr<U>& obj) noexcept
    {
      static_assert(is_tm_closure(T::term), "T is not closure type");

      if (likely(obj && has_type<T>(obj))) {
        return static_object_cast<T>(obj);
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
      static_assert(is_tm_closure(T::term), "T is not closure type");

      if (likely(obj && has_type<T>(obj))) {
        return static_object_cast<T>(std::move(obj));
      }
      return nullptr;
    }

  } // namespace interface

} // namespace TORI_NS::detail

// BadClosureCast
TORI_DECL_TYPE(BadClosureCast)