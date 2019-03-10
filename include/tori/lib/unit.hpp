// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file Unit

#if !defined(TORI_NO_LOCAL_INCLUDE)
#  include "../core.hpp"
#endif

namespace TORI_NS::detail {

  /// UnitValue
  struct UnitValue
  {
  };

  namespace interface {

    /// Unit
    using Unit = Box<UnitValue>;

  } // namespace interface

} // namespace TORI_NS::detail

// Unit
TORI_DECL_TYPE(Unit)