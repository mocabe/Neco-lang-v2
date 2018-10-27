// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../core.hpp"

#include <vector>
#include <string>
#include <variant>
#include <algorithm>

namespace TORI_NS::detail {

  /// get string represents type
  [[nodiscard]] TORI_INLINE std::string to_string_impl(
    const object_ptr<const Type>& type,
    std::vector<object_ptr<const Type>>& stack) {
    if (is_value_type(type)) return get<ValueType>(*type).c_str();
    if (is_arrow_type(type))
      return "(" +                                                   //
             to_string_impl(get<ArrowType>(*type).captured, stack) + //
             " -> " +                                                //
             to_string_impl(get<ArrowType>(*type).returns, stack) +  //
             ")";                                                    //
    if (is_vartype(type)) {
      return "Var[" +                                            //
             std::to_string(get_if<VarType>(type.value())->id) + //
             "]";                                                //
    }

    assert(false);
    unreachable();
  }

  namespace interface {
    /// convert type to string
    [[nodiscard]] TORI_INLINE std::string to_string(
      const object_ptr<const Type>& type) {
      std::vector<object_ptr<const Type>> stack;
      return to_string_impl(type, stack);
    }
  } // namespace interface
} // namespace TORI_NS::detail