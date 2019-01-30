#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file Fix

#include "../config/config.hpp"

#include "boxed.hpp"
#include "type_gen.hpp"

#include <type_traits>

namespace TORI_NS::detail {

  struct FixValue;

  namespace interface {

    /// Fix object
    using Fix = BoxedHeapObject<FixValue>;

  } // namespace interface

  struct FixValue
  {
    static constexpr auto term = type_c<tm_fix<Fix>>;
  };

  static_assert(has_term<FixValue>());

} // namespace TORI_NS::detail

// Fix
TORI_DECL_TYPE(Fix)