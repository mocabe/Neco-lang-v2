// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file Fix

#include "DynamicTypeUtil.hpp"
#include <type_traits>

namespace TORI_NS::detail {

  struct FixValue {};

  namespace interface {
    /// Fix object
    using Fix = BoxedHeapObject<FixValue>;
  } // namespace interface
} // namespace TORI_NS::detail

namespace TORI_NS {
  TORI_DECL_TYPE(Fix)
}