// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file Identity

#include "../core.hpp"

namespace TORI_NS::detail {
  namespace interface {
    struct Identity
      : Function<Identity, Auto<class Identity_X>, Auto<class Identity_X>> {
      ReturnType code() {
        return arg<0>();
      }
    };
  } // namespace interface
} // namespace TORI_NS::detail