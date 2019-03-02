// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once 

// config
#include "../config/config.hpp"
// refcount
#include "atomic.hpp"
// term
#include "terms.hpp"

#include <cstdint>
#include <atomic>
#include <cassert>
#include <stdexcept>
#include <new>

namespace TORI_NS::detail {

  // value type
  struct ValueType;
  // arrow type
  struct ArrowType;
  // type variable
  struct VarType;
  // TypeValue
  class TypeValue;

  // interface
  namespace interface {

    // heap-allocated object of type T
    template <class T>
    struct Box;

    // handler for heap-allocated object
    template <class>
    class object_ptr;

    // object info table
    struct object_info_table;

    // heap-allocated runtime type infomation
    using Type = Box<TypeValue>;

    /// Base class of heap-allocated objects
    struct Object
    {
      /// term
      static constexpr auto term = type_c<tm_value<Object>>;

      /// Ctor
      constexpr Object(const object_info_table* info)
        : info_table {info}
      {
        /* Default initialize refcount and spinlock */
      }

      /// Copy ctor
      constexpr Object(const Object& other)
        : info_table {other.info_table}
      {
        /* Default initialize refcount and spinlock */
      }

      /// 4byte: reference count
      mutable atomic_refcount<uint32_t> refcount = {1u};

      /// 1byte: spinlock
      mutable atomic_spinlock<uint8_t> spinlock = {/*false*/};

      /* 3byte: padding */
      std::byte reserved[3] = {};

      /// 8byte: pointer to info table
      const object_info_table* info_table;
    };

    static_assert(std::is_standard_layout_v<Object>);

  } // namespace interface

} // namespace TORI_NS::detail