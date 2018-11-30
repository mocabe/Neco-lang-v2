// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../core.hpp"
#include "Primitive.hpp"

namespace TORI_NS::detail {

  class If_X;

  namespace interface {
    struct If : Function<If, Bool, forall<If_X>, forall<If_X>, forall<If_X>> {
      ReturnType code() const
      {
        if (*eval_arg<0>())
          return arg<1>();
        else
          return arg<2>();
      }
    };
  } // namespace interface
} // namespace TORI_NS::detail
