// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "type_gen.hpp"

namespace TORI_NS::detail {

  namespace interface {

    /// Undefined
    ///
    /// null objects are treated as instances of Undefined.
    struct Undefined;

  } // namespace interface

} // namespace TORI_NS::detail

template <>
struct tori::object_type_traits<tori::Undefined>
{
  static constexpr const char name[] = "_Undefined(NULL)";
};