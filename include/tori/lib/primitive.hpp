// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file primitives

#if !defined(TORI_NO_LOCAL_INCLUDE)
#  include "../core.hpp"
#endif

namespace TORI_NS::detail::interface {

  // clang-format off

  // primitive types

  using Int8   = Box<int8_t>;
  using Int16  = Box<int16_t>;
  using Int32  = Box<int32_t>;
  using Int64  = Box<int64_t>;
  using UInt8  = Box<uint8_t>;
  using UInt16 = Box<uint16_t>;
  using UInt32 = Box<uint32_t>;
  using UInt64 = Box<uint64_t>;
  using Float  = Box<float>;
  using Double = Box<double>;

  using Char   = Int8;
  using Short  = Int16;
  using Int    = Int32;
  using Long   = Int64;

  using UChar  = UInt8;
  using UShort = UInt16;
  using UInt   = UInt32;
  using ULong  = UInt64;

  using Bool   = Box<bool>;

  // primitive non-const pointer types

  using Int8Ptr   = Box<int8_t *>;
  using Int16Ptr  = Box<int16_t *>;
  using Int32Ptr  = Box<int32_t *>;
  using Int64Ptr  = Box<int64_t *>;
  using UInt8Ptr  = Box<uint8_t *>;
  using UInt16Ptr = Box<uint16_t *>;
  using UInt32Ptr = Box<uint32_t *>;
  using UInt64Ptr = Box<uint64_t *>;
  using FloatPtr  = Box<float *>;
  using DoublePtr = Box<double *>;

  using CharPtr   = Int8Ptr;
  using ShortPtr  = Int16Ptr;
  using IntPtr    = Int32Ptr;
  using LongPtr   = Int64Ptr;

  using UCharPtr  = UInt8Ptr;
  using UShortPtr = UInt16Ptr;
  using UIntPtr   = UInt32Ptr;
  using ULongPtr  = UInt64Ptr;

  using BoolPtr   = Box<bool *>;

  // void*
  using VoidPtr   = Box<void *>;

  // const char*
  using CStr      = Box<const char *>;

  // clang-format on

} // namespace TORI_NS::detail::interface

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