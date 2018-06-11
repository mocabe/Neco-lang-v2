// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file primitives

#include "DynamicTypeUtil.hpp"

namespace TORI_NS::detail::interface {

  // primitive types
  using Int8 = BoxedHeapObject<int8_t>;
  using Int16 = BoxedHeapObject<int16_t>;
  using Int32 = BoxedHeapObject<int32_t>;
  using Int64 = BoxedHeapObject<int64_t>;
  using UInt8 = BoxedHeapObject<uint8_t>;
  using UInt16 = BoxedHeapObject<uint16_t>;
  using UInt32 = BoxedHeapObject<uint32_t>;
  using UInt64 = BoxedHeapObject<uint64_t>;
  using Float = BoxedHeapObject<float>;
  using Double = BoxedHeapObject<double>;

  using Char = Int8;
  using Int = Int32;
  using Long = Int64;

  using UChar = UInt8;
  using UInt = UInt32;
  using ULong = UInt64;

  using Bool = BoxedHeapObject<bool>;

  // primitive non-const pointer types
  using Int8Ptr = BoxedHeapObject<int8_t *>;
  using Int16Ptr = BoxedHeapObject<int16_t *>;
  using Int32Ptr = BoxedHeapObject<int32_t *>;
  using Int64Ptr = BoxedHeapObject<int64_t *>;
  using UInt8Ptr = BoxedHeapObject<uint8_t *>;
  using UInt16Ptr = BoxedHeapObject<uint16_t *>;
  using UInt32Ptr = BoxedHeapObject<uint32_t *>;
  using UInt64Ptr = BoxedHeapObject<uint64_t *>;
  using FloatPtr = BoxedHeapObject<float *>;
  using DoublePtr = BoxedHeapObject<double *>;

  using CharPtr = Int8Ptr;
  using IntPtr = Int32Ptr;
  using LongPtr = Int64Ptr;

  using UCharPtr = UInt8Ptr;
  using UIntPtr = UInt32Ptr;
  using ULongPtr = UInt64Ptr;

  using BoolPtr = BoxedHeapObject<bool *>;

  // void*
  using VoidPtr = BoxedHeapObject<void *>;

  // const char*
  using CStr = BoxedHeapObject<const char *>;
} // namespace TORI_NS::detail::interface

namespace TORI_NS {
  // primitive types
  TORI_DECL_TYPE(Int8)
  TORI_DECL_TYPE(Int16)
  TORI_DECL_TYPE(Int32)
  TORI_DECL_TYPE(Int64)

  TORI_DECL_TYPE(UInt8)
  TORI_DECL_TYPE(UInt16)
  TORI_DECL_TYPE(UInt32)
  TORI_DECL_TYPE(UInt64)

  TORI_DECL_TYPE(Float)
  TORI_DECL_TYPE(Double)

  TORI_DECL_TYPE(Bool)

  TORI_DECL_TYPE(Int8Ptr)
  TORI_DECL_TYPE(Int16Ptr)
  TORI_DECL_TYPE(Int32Ptr)
  TORI_DECL_TYPE(Int64Ptr)

  TORI_DECL_TYPE(UInt8Ptr)
  TORI_DECL_TYPE(UInt16Ptr)
  TORI_DECL_TYPE(UInt32Ptr)
  TORI_DECL_TYPE(UInt64Ptr)

  TORI_DECL_TYPE(FloatPtr)
  TORI_DECL_TYPE(DoublePtr)

  TORI_DECL_TYPE(BoolPtr)

  TORI_DECL_TYPE(VoidPtr)
  TORI_DECL_TYPE(CStr)

} // namespace TORI_NS