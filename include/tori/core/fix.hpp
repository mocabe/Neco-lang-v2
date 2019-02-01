#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#include "../config/config.hpp"

#include "box.hpp"
#include "type_gen.hpp"

#include <type_traits>

namespace TORI_NS::detail {

  struct FixValue;

  namespace interface {

    /// Fix object
    using Fix = Box<FixValue>;

  } // namespace interface

  struct FixValue
  {
    // overwrite value term
    static constexpr auto term = type_c<tm_fix<Fix>>;
  };

} // namespace TORI_NS::detail

// Fix
TORI_DECL_TYPE(Fix)