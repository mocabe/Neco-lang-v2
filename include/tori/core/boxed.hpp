#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file Box

#include "object_ptr.hpp"
#include "type_value.hpp" // clang requires definition of TypeValue to compile.

#include <cassert>
#include <type_traits>

/// \file BoxedHeapObject.hpp

namespace TORI_NS::detail {

  /// tag type to initialize object with 0 reference count
  struct static_construct_t
  {
    explicit static_construct_t() = default;
  };

  namespace interface {

    /// static_construct
    inline constexpr static_construct_t static_construct = static_construct_t();

    // forward decl
    template <class T>
    object_ptr<const Type> object_type();

  } // namespace interface

  /// \brief vtable function to delete object
  ///
  /// vtable function to delete heap allocated object.
  template <class T>
  void vtbl_destroy_func(const HeapObject *obj) noexcept
  {
    static_assert(
      std::is_nothrow_destructible_v<T>,
      "Boxed object should have nothrow destructor");
    auto *p = static_cast<const T *>(obj);
    delete p;
  }

  /// \brief vtable function to clone object.
  ///
  /// vtable function to clone heap-allocated object.
  /// \param T value type
  /// \returns pointer to generated object.
  /// \notes return nullptr when allocation/initialization failed.
  template <class T>
  HeapObject *vtbl_clone_func(const HeapObject *obj) noexcept
  {
    try {
      auto p = static_cast<const T *>(obj);
      return new T {*p};
    } catch (...) {
      // TODO: return Exception object
      return nullptr;
    }
  }

  template <class T>
  constexpr auto get_value_object_term()
  {
    if constexpr (has_term<T>())
      return T::term;
    else
      return type_c<tm_value<BoxedHeapObject<T>>>;
  }

  namespace interface {

    /// \brief Heap-allocated object generator.
    /// \param T value type
    template <class T>
    struct BoxedHeapObject : HeapObject
    {

      /// value type
      using value_type = T;
      /// term
      static constexpr auto term = get_value_object_term<T>();

      /// info table initializer
      struct info_table_initializer
      {
        /// static object info table
        static const object_info_table info_table;
      };

      /// Ctor
      template <
        class U,
        class... Args,
        class = std::enable_if_t<
          !std::is_same_v<std::decay_t<U>, BoxedHeapObject> &&
          !std::is_same_v<std::decay_t<U>, static_construct_t>>>
      constexpr BoxedHeapObject(U &&u, Args &&... args)
        : HeapObject {1u, &info_table_initializer::info_table}
        , _value {std::forward<U>(u), std::forward<Args>(args)...}
      {
      }

      /// Ctor (static initialization)
      template <class... Args>
      constexpr BoxedHeapObject(static_construct_t, Args &&... args)
        : BoxedHeapObject(std::forward<Args>(args)...)
      {
        // set refcount ZERO to avoid deletion
        refcount = 0u;
      }

      /// Ctor
      constexpr BoxedHeapObject()
        : HeapObject {1u, &info_table_initializer::info_table}
        , _value {}
      {
      }

      /// Copy ctor
      constexpr BoxedHeapObject(const BoxedHeapObject &obj)
        : HeapObject {1u, &info_table_initializer::info_table}
        , _value {obj._value}
      {
      }

      /// Move ctor
      constexpr BoxedHeapObject(BoxedHeapObject &&obj)
        : HeapObject {1u, &info_table_initializer::info_table}
        , _value {std::move(obj._value)}
      {
      }

      /// operator=
      BoxedHeapObject &operator=(const BoxedHeapObject &obj)
      {
        _value = obj._value;
      }

      /// operator=
      BoxedHeapObject &operator=(BoxedHeapObject &&obj)
      {
        _value = std::move(obj._value);
      }

      /// value
      value_type _value;
    };

    // Initialize object header
    template <class T>
    const object_info_table
      BoxedHeapObject<T>::info_table_initializer::info_table = {
        object_type<BoxedHeapObject>(),     //
        sizeof(BoxedHeapObject),            //
        object_header_extend_bytes,         //
        vtbl_destroy_func<BoxedHeapObject>, //
        vtbl_clone_func<BoxedHeapObject>};  //

  } // namespace interface

} // namespace TORI_NS::detail