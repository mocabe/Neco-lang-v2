// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file Unit

#include "../core.hpp"

namespace TORI_NS::detail {

  /// UnitValue
  struct UnitValue
  {
  };

  namespace interface {

    /// Unit
    using Unit = BoxedHeapObject<UnitValue>;

  } // namespace interface

} // namespace TORI_NS::detail

namespace TORI_NS {
  TORI_DECL_TYPE(Unit)
} // namespace TORI_NS