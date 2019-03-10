// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#if !defined(TORI_NO_LOCAL_INCLUDE)
#  include "../core.hpp"
#  include "primitive.hpp"
#endif

namespace TORI_NS::detail {

  class If_X;

  namespace interface {
    struct If : Function<If, Bool, forall<If_X>, forall<If_X>, forall<If_X>>
    {
      return_type code() const
      {
        if (*eval_arg<0>())
          return arg<1>();
        else
          return arg<2>();
      }
    };

  } // namespace interface
} // namespace TORI_NS::detail
