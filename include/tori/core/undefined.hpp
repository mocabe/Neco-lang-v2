// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "type_gen.hpp"

namespace TORI_NS::detail {

  struct UndefinedValue {};

  namespace interface {
    /// Undefined
    using Undefined = Box<UndefinedValue>;
  } // namespace interface

} // namespace TORI_NS::detail

TORI_DECL_TYPE(Undefined)
