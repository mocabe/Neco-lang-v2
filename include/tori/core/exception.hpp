// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "box.hpp"
#include "string.hpp"

#include <exception>

namespace TORI_NS::detail {

  /// \brief Exception value
  ///
  /// Exception object holds a pointer to arbitary error value.
  /// This is useful to detect exception object with single type comparison.
  struct ExceptionValue
  {
    template <class T>
    ExceptionValue(object_ptr<const String> msg, object_ptr<T> err)
      : message {std::move(msg)}
      , error_value {std::move(err)}
    {
    }

    template <class T>
    ExceptionValue(const char* msg, object_ptr<T> err)
      : ExceptionValue(make_object<String>(msg), std::move(err))
    {
    }

    /// message
    object_ptr<const String> message;
    /// pointer to error value
    object_ptr<const Object> error_value;
  };

  namespace interface {

    /// Exception
    using Exception = Box<ExceptionValue>;

  } // namespace interface

} // namespace TORI_NS::detail

// Exception
TORI_DECL_TYPE(Exception)