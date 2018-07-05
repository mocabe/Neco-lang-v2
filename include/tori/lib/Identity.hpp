// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file Identity

#include "../core.hpp"

namespace TORI_NS::detail {

  class Identity_X;

  namespace interface {
    struct Identity
      : Function<Identity, forall<Identity_X>, forall<Identity_X>> {
      ReturnType code() const {
        return arg<0>();
      }
    };
  } // namespace interface
} // namespace TORI_NS::detail