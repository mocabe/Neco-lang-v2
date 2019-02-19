// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "object_ptr.hpp"
#include "immediate.hpp"
#include "object_ptr_generic.hpp"
#include "object_ptr_storage.hpp"
#include "object_cast.hpp"
#include "dynamic_typing.hpp"
#include "undefined.hpp"

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
    [[nodiscard]] auto value_cast(const object_ptr_generic& obj)
    {
      const auto& storage = _get_storage(obj);
      if constexpr (is_transfarable_immediate(type_c<T>)) {
        // not immediate
        if (!obj.is_immediate())
          throw bad_value_cast(
            storage.ptr() ? storage.get_pointer_type()
                          : object_type<Undefined>(),
            object_type<T>());
        // return immediate type
        if (storage.has_immediate_type<T>())
          return immediate(get<T>(storage.immediate_union()));
        else
          throw bad_value_cast(storage.get_immediate_type(), object_type<T>());
      } else {
        // not pointer
        if (!obj.is_pointer())
          throw bad_value_cast(storage.get_immediate_type(), object_type<T>());
        // pointer
        if (storage.has_pointer_type<T>()) {
          auto ptr = storage.ptr();
          ptr->refcount.fetch_add(); // add refcount
          return object_ptr(static_cast<T*>(ptr));
        } else
          throw bad_value_cast(
            storage.ptr() ? storage.get_pointer_type()
                          : object_type<Undefined>(),
            object_type<T>());
      }
    }

    /// value_cast
    ///
    /// dynamically cast object to specified value type.
    /// \throws bad_value_cast when fail.
    template <class T>
    [[nodiscard]] auto value_cast(object_ptr_generic&& obj)
    {
      auto& storage = _get_storage(obj);
      if constexpr (is_transfarable_immediate(type_c<T>)) {
        // delegate to lvalue version
        return value_cast<T>(obj);
      } else {
        // not pointer
        if (!obj.is_pointer())
          throw bad_value_cast(storage.get_immediate_type(), object_type<T>());
        // pointer
        if (storage.has_pointer_type<T>())
          return object_ptr(static_cast<T*>(storage.ptr()));
        else
          throw bad_value_cast(storage.get_pointer_type(), object_type<T>());
      }
    }

    /// value_cast_if
    ///
    /// dynamically cast object to specified value type.
    /// \returns nullptr or zero initialized immediate value when fail.
    template <
      class T,
      // disabled because immediate<T> cannot propagate error conditions.
      class = std::enable_if_t<!is_transfarable_immediate(type_c<T>)>>
    [[nodiscard]] object_ptr<T> value_cast_if(const object_ptr_generic& obj) noexcept
    {
      const auto& storage = _get_storage(obj);
      if (!obj.is_pointer())
        return nullptr;
      if (storage.has_pointer_type<T>()) {
        auto ptr = storage.ptr();
        ptr->refcount.fetch_add(); // add refcount
        return static_cast<T*>(ptr);
      } else
        return nullptr;
    }

    /// value_cast_if
    ///
    /// dynamically cast object to specified value type.
    /// \returns nullptr or zero initialized immediate value when fail.
    template <
      class T,
      class = std::enable_if_t<!is_transfarable_immediate(type_c<T>)>>
    [[nodiscard]] object_ptr<T> value_cast_if(object_ptr_generic&& obj) noexcept
    {
      auto& storage = _get_storage(obj);
      if (!storage.is_pointer())
        return nullptr;
      // pointer
      if (storage.has_pointer_type<T>())
        return static_cast<T*>(storage.ptr());
      else
        return nullptr;
    }

  } // namespace interface

} // namespace TORI_NS::detail

// BadValueCast
TORI_DECL_TYPE(BadValueCast)