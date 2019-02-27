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

  // ------------------------------------------
  // helper

  object_ptr<Exception> add_exception_tag(object_ptr<Exception>&& e)
  {
    _get_storage(e).set_pointer_tag(
      object_ptr_storage::pointer_tags::exception);
    return std::move(e);
  }

  bool has_exception_tag(const object_ptr<const Object>& obj)
  {
    return _get_storage(obj).is_exception();
  }

  // ------------------------------------------
  // conversion

  object_ptr<Exception> to_Exception(const std::exception& e)
  {
    return make_object<Exception>(e.what(), object_ptr(nullptr));
  }

} // namespace TORI_NS::detail

// Exception
TORI_DECL_TYPE(Exception)