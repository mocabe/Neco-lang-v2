// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "Eval.hpp"

namespace TORI_NS::detail {

  // ----------------------------------------
  // HeapObject
  // ----------------------------------------
  // no padding
  static_assert(sizeof(HeapObject) == 16 + object_header_extend_bytes);
  static_assert(sizeof(ObjectInfoTable) == 40);

  // ----------------------------------------
  // ObjectPtr
  // ----------------------------------------
  static_assert(sizeof(ObjectPtr<>) == 8);

  // ----------------------------------------
  // Type
  // ----------------------------------------
  static_assert(sizeof(ValueType) == 8);
  static_assert(sizeof(ArrowType) == 16);
  static_assert(sizeof(VarType) == 8);
  static_assert(sizeof(TypeValue) == 24);
  static_assert(
    std::is_same_v<ValueType, std::variant_alternative_t<0, TypeValue>>);
  static_assert(
    std::is_same_v<ArrowType, std::variant_alternative_t<1, TypeValue>>);
  static_assert(
    std::is_same_v<VarType, std::variant_alternative_t<2, TypeValue>>);

  // ----------------------------------------
  // Closure
  // ----------------------------------------
  // no padding
  static_assert(
    sizeof(ClosureN<4>) ==
    sizeof(HeapObject) + 8 + closure_header_extend_bytes + 4 * 8);
  static_assert(sizeof(ClosureInfoTable) == sizeof(ObjectInfoTable) + 24);

  // ----------------------------------------
  // Object
  // ----------------------------------------
  static_assert(sizeof(expected<Fix>) == sizeof(HeapObject));
  static_assert(sizeof(closure<Fix, Fix>) == sizeof(HeapObject));
  static_assert(sizeof(forall<void>) == sizeof(HeapObject));

  static_assert(sizeof(ExceptionValue) == 8);
  static_assert(
    sizeof(Exception) == sizeof(ExceptionValue) + sizeof(HeapObject));
  
  static_assert(sizeof(TypeErrorValue) == 16);
  static_assert(
    sizeof(TypeError) == sizeof(TypeErrorValue) + sizeof(HeapObject));

  static_assert(sizeof(EvalErrorValue) == 16);
  static_assert(
    sizeof(EvalError) == sizeof(EvalErrorValue) + sizeof(HeapObject));

  // strict size of Fix is not actually required
  static_assert(sizeof(FixValue) == 1);
  static_assert(sizeof(Fix) == 8 + sizeof(HeapObject));

  static_assert(sizeof(ApplyRValue) == 16);
  static_assert(sizeof(ApplyR) == sizeof(HeapObject) + sizeof(ApplyRValue));
  static_assert(sizeof(ApplyR) == sizeof(Apply<Fix, Fix>));
}