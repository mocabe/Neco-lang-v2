// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file Identity

#if !defined(TORI_NO_LOCAL_INCLUDE)
#  include "../core.hpp"
#endif

namespace TORI_NS::detail {

  class Identity_X;

  namespace interface {

    struct Identity : Function<Identity, forall<Identity_X>, forall<Identity_X>>
    {
      return_type code() const
      {
        return arg<0>();
      }
    };

  } // namespace interface
} // namespace TORI_NS::detail