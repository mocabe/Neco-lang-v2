// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../core.hpp"

#include <vector>
#include <string>
#include <variant>
#include <algorithm>

namespace TORI_NS::detail {

  /// get string represents type
  template <size_t MaxDepth>
  [[nodiscard]] std::string
    to_string_impl(const object_ptr<const Type>& type, size_t depth)
  {
    if (depth > MaxDepth)
      return "[...]";

    if (is_value_type(type)) {
      return get<value_type>(*type).c_str();
    }

    if (is_arrow_type(type)) {
      return "(" + //
             to_string_impl<MaxDepth>(
               get<arrow_type>(*type).captured, depth + 1) + //
             " -> " +                                        //
             to_string_impl<MaxDepth>(
               get<arrow_type>(*type).returns, depth + 1) + //
             ")";                                           //
    }

    if (is_var_type(type)) {
      return "Var[" +                                             //
             std::to_string(get_if<var_type>(type.value())->id) + //
             "]";                                                 //
    }

    unreachable();
  }

  namespace interface {

    /// convert type to string
    template <size_t MaxDepth = 48>
    [[nodiscard]] std::string to_string(const object_ptr<const Type>& type)
    {
      return to_string_impl<MaxDepth>(type, 1);
    }

  } // namespace interface

} // namespace TORI_NS::detail