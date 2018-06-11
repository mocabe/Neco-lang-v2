// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file Thunk

#include "DynamicTypeUtil.hpp"

namespace TORI_NS::detail {
  struct ThunkValue {
    ObjectPtr<> value;
    bool_t evaluated = false;
  };
  namespace interface {
    /// Thunk
    using Thunk = BoxedHeapObject<detail::ThunkValue>;
  } // namespace interface
} // namespace TORI_NS::detail

namespace TORI_NS {
  TORI_DECL_TYPE(Thunk)
}