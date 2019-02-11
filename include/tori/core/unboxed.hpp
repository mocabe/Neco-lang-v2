// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "meta_tuple.hpp"
#include "recursive_union.hpp"

namespace TORI_NS {

  // fwd
  template <class T>
  struct object_type_traits;

} // namespace TORI_NS

#define TORI_DECL_IMMEDIATE_TYPE(TYPE, NAME)      \
  template <>                                     \
  struct TORI_NS::object_type_traits<TYPE>        \
  {                                               \
    static constexpr char name[] = "_" #NAME "#"; \
  };

TORI_DECL_IMMEDIATE_TYPE(uint8_t, UInt8)
TORI_DECL_IMMEDIATE_TYPE(uint16_t, UInt16)
TORI_DECL_IMMEDIATE_TYPE(uint32_t, UInt32)
TORI_DECL_IMMEDIATE_TYPE(uint64_t, UInt64)
TORI_DECL_IMMEDIATE_TYPE(int8_t, Int8)
TORI_DECL_IMMEDIATE_TYPE(int16_t, Int16)
TORI_DECL_IMMEDIATE_TYPE(int32_t, Int32)
TORI_DECL_IMMEDIATE_TYPE(int64_t, Int64)
TORI_DECL_IMMEDIATE_TYPE(float, Float)
TORI_DECL_IMMEDIATE_TYPE(double, Double)

namespace TORI_NS::detail {

  /// transfarable immediate types
  static constexpr auto immediate_transfarable = tuple_c<
    int8_t,   // signed 8bit
    int16_t,  // signed 16bit
    int32_t,  // signed 32bit
    uint8_t,  // unsigned 8bit
    uint16_t, // unsigned 16bit
    uint32_t, // unsigned 32bit
    float>;   // 32bit float

  /// transfarable immediates + signed/unsigned 64bit integers
  static constexpr auto immediate_appliable =
    concat(immediate_transfarable, tuple_c<int64_t, uint64_t, double>);

  /// get recursive union type
  template <class... Ts>
  constexpr auto get_recursive_union_type(meta_tuple<Ts...>)
  {
    return type_c<recursive_union<Ts...>>;
  }

  /// union of transfarable immediate types
  using immediate_transfarable_union =
    decltype(get_recursive_union_type(immediate_transfarable))::type;

  /// union of appliable immediate types
  using immediate_appliable_union =
    decltype(get_recursive_union_type(immediate_appliable))::type;

  static_assert(sizeof(immediate_transfarable_union) == 4);
  static_assert(sizeof(immediate_appliable_union) == 8);

  /// is_transfarable_immediate
  template <class T>
  constexpr auto is_transfarable_immediate(meta_type<T>)
  {
    return contains(type_c<T>, immediate_transfarable);
  }

  /// is_appliable_immediate
  template <class T>
  constexpr auto is_appliable_immediate(meta_type<T>)
  {
    return contains(type_c<T>, immediate_appliable);
  }

  /// is_immediate
  template <class T>
  constexpr auto is_immediate(meta_type<T> type)
  {
    return is_transfarable_immediate(type) || is_appliable_immediate(type);
  }

} // namespace TORI_NS::detail