#pragma once 

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file HeapObject

// config
#include "../config/config.hpp"
// refcount
#include "atomic_refcount.hpp"
// term
#include "terms.hpp"

#include <cstdint>
#include <type_traits>
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
    struct BoxedHeapObject;

    // handler for heap-allocated object
    template <class>
    class object_ptr;

    // object info table
    struct object_info_table;

    // heap-allocated runtime type infomation
    using Type = BoxedHeapObject<TypeValue>;

    /// Base class of heap-allocated objects
    struct HeapObject
    {
      /// term
      static constexpr auto term = type_c<tm_value<HeapObject>>;

      // reference count
      mutable atomic_refcount<uint64_t> refcount;

      /// pointer to info-table
      const object_info_table* info_table;

#if defined(OBJECT_HEADER_EXTEND_BYTES)
      /// additional buffer storage
      std::byte obj_ext_buffer[object_header_extend_bytes] = {};
#endif
    };

  } // namespace interface

} // namespace TORI_NS::detail