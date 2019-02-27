// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once 

// config
#include "../config/config.hpp"
// refcount
#include "atomic_refcount.hpp"
// term
#include "terms.hpp"

#include <cstdint>
#include <atomic>
#include <cassert>
#include <stdexcept>
#include <new>

namespace TORI_NS::detail {

#if defined(OBJECT_HEADER_EXTEND_BYTES)
  constexpr uint64_t object_header_extend_bytes = OBJECT_HEADER_EXTEND_BYTES;
#else
  /// size of additional buffer in heap object header
  constexpr uint64_t object_header_extend_bytes = 0;
#endif

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

      // reference count
      mutable atomic_refcount<uint64_t> refcount;

      /// pointer to info-table
      const object_info_table* info_table;

#if defined(OBJECT_HEADER_EXTEND_BYTES)
      /// additional buffer storage
      std::byte obj_ext_buffer[object_header_extend_bytes] = {};
#endif
    };

    static_assert(std::is_standard_layout_v<Object>);

  } // namespace interface

} // namespace TORI_NS::detail