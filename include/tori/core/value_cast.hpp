// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "object_ptr.hpp"
#include "object_cast.hpp"
#include "dynamic_typing.hpp"

#include <exception>

namespace TORI_NS::detail {

  namespace interface {
    /// bad_value_cast exception
    class bad_value_cast : public std::logic_error
    {
    public:
      explicit bad_value_cast(
        object_ptr<const Type> from,
        object_ptr<const Type> to)
        : std::logic_error("bad_value_cast")
        , m_from {std::move(from)}
        , m_to {std::move(to)}
      {
      }

      /// get from
      const object_ptr<const Type>& from() const
      {
        return m_from;
      }

      /// get to
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
  } // namespace interface

  struct BadValueCastValue
  {
    object_ptr<const Type> from;
    object_ptr<const Type> to;
  };

  namespace interface {

    /// Exception object for bad_value_cast exception
    using BadValueCast = Box<detail::BadValueCastValue>;

  } // namespace interface

  namespace interface {

    /// value_cast
    ///
    /// dynamically cast object to specified value type.
    /// \throws bad_value_cast when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T> value_cast(const object_ptr<U>& obj)
    {
      static_assert(
        !is_immediate(type_c<T>), "Cannot cast to/from unboxed types");
      static_assert(
        !is_immediate(type_c<U>), "Cannot cast to/from unboxed types");

      static_assert(
        !is_tm_closure(T::term),
        "Casting to closure types might cause slicing.");

      if (likely(obj && has_type<T>(obj))) {
        return static_object_cast<T>(obj);
      }
      throw bad_value_cast(obj ? get_type(obj) : nullptr, object_type<T>());
    }

    /// value_cast
    ///
    /// dynamically cast object to specified value type.
    /// \throws bad_value_cast when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T> value_cast(object_ptr<U>&& obj)
    {
      static_assert(
        !is_immediate(type_c<T>), "Cannot cast to/from unboxed types");
      static_assert(
        !is_immediate(type_c<U>), "Cannot cast to/from unboxed types");

      static_assert(
        !is_tm_closure(T::term),
        "Casting to closure types might cause slicing.");

      if (likely(obj && has_type<T>(obj))) {
        return static_object_cast<T>(std::move(obj));
      }
      throw bad_value_cast(obj ? get_type(obj) : nullptr, object_type<T>());
    }


    /// value_cast_if
    ///
    /// dynamically cast object to specified value type.
    /// \returns nullptr when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T> value_cast_if(const object_ptr<U>& obj) noexcept
    {
      static_assert(
        !is_immediate(type_c<T>), "Cannot cast to/from unboxed types");
      static_assert(
        !is_immediate(type_c<U>), "Cannot cast to/from unboxed types");

      static_assert(
        !is_tm_closure(T::term),
        "Casting to closure types might cause slicing.");

      if (likely(obj && has_type<T>(obj))) {
        return static_object_cast<T>(obj);
      }
      return nullptr;
    }

    /// value_cast_if
    ///
    /// dynamically cast object to specified value type.
    /// \returns nullptr when fail.
    template <class T, class U>
    [[nodiscard]] object_ptr<T> value_cast_if(object_ptr<U>&& obj) noexcept
    {
      static_assert(
        !is_immediate(type_c<T>), "Cannot cast to/from unboxed types");
      static_assert(
        !is_immediate(type_c<U>), "Cannot cast to/from unboxed types");

      static_assert(
        !is_tm_closure(T::term),
        "Casting to closure types might cause slicing.");

      if (likely(obj && has_type<T>(obj))) {
        return static_object_cast<T>(std::move(obj));
      }
      return nullptr;
    }

    /// value_cast
    ///
    /// dynamically cast object to specified value type.
    /// \throws bad_value_cast when fail.
    template <class T>
    [[nodiscard]] object_ptr<T> value_cast(const object_ptr_generic& obj)
    {
      if constexpr (is_transfarable_immediate(type_c<T>)) {
        // not immediate
        if (!obj.is_immediate())
          throw bad_value_cast(
            obj.m_storage.get_pointer_type(), object_type<T>());
        // return immediate type
        if (obj.m_storage.has_immediate_type<T>())
          return get<T>(obj.m_storage.immediate_union());
        else
          throw bad_value_cast(
            obj.m_storage.get_immediate_type(), object_type<T>());
      } else {
        // not pointer
        if (!obj.is_pointer())
          throw bad_value_cast(
            obj.m_storage.get_immediate_type(), object_type<T>());
        // pointer
        if (obj.m_storage.has_pointer_type<T>()) {
          auto ptr = obj.m_storage.ptr();
          auto tmp = object_ptr(ptr);
          return static_object_cast<T>(tmp); // add refcount
        } else
          throw bad_value_cast(
            obj.m_storage.get_pointer_type(), object_type<T>());
      }
    }

    /// value_cast
    ///
    /// dynamically cast object to specified value type.
    /// \throws bad_value_cast when fail.
    template <class T>
    [[nodiscard]] object_ptr<T> value_cast(object_ptr_generic&& obj)
    {
      if constexpr (is_transfarable_immediate(type_c<T>)) {
        // delegate to lvalue version
        return value_cast<T>(obj);
      } else {
        // not pointer
        if (!obj.is_pointer())
          throw bad_value_cast(
            obj.m_storage.get_immediate_type(), object_type<T>());
        // pointer
        if (obj.m_storage.has_pointer_type<T>()) {
          auto ptr = obj.m_storage.ptr();
          // move cast
          return static_object_cast<T>(object_ptr(ptr));
        } else
          throw bad_value_cast(
            obj.m_storage.get_pointer_type(), object_type<T>());
      }
    }

    /// value_cast_if
    ///
    /// dynamically cast object to specified value type.
    /// \returns nullptr or zero initialized immediate value when fail.
    template <class T>
    [[nodiscard]] object_ptr<T> value_cast_if(const object_ptr_generic& obj)
    {
      if constexpr (is_transfarable_immediate(type_c<T>)) {
        if (!obj.is_immediate())
          return T {};
        if (obj.m_storage.has_immediate_type<T>())
          return get<T>(obj.m_storage.immediate_union());
        else
          return T {};
      } else {
        if (!obj.is_pointer())
          return nullptr;
        if (obj.m_storage.has_pointer_type<T>()) {
          auto ptr = obj.m_storage.ptr();
          auto tmp = object_ptr(ptr);
          return static_object_cast<T>(tmp); // add refcount
        } else
          return nullptr;
      }
    }

    /// value_cast_if
    ///
    /// dynamically cast object to specified value type.
    /// \returns nullptr or zero initialized immediate value when fail.
    template <class T>
    [[nodiscard]] object_ptr<T> value_cast_if(object_ptr_generic&& obj)
    {
      if constexpr (is_transfarable_immediate(type_c<T>)) {
        // delegate to lvalue version
        return value_cast_if<T>(obj);
      } else {
        if (!obj.m_storage.is_pointer())
          return nullptr;
        if (obj.m_storage.has_pointer_type<T>()) {
          auto ptr = obj.m_storage.ptr();
          // move cast
          return static_object_cast<T>(object_ptr(ptr));
        } else
          return nullptr;
      }
    }

  } // namespace interface

} // namespace TORI_NS::detail

// BadValueCast
TORI_DECL_TYPE(BadValueCast)