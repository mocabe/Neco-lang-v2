// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "exception.hpp"

namespace TORI_NS::detail {

  // ------------------------------------------
  // Eval errors

  /// eval_error_type
  enum class eval_error_type : uint64_t
  {
    unknown = 0,
    bad_fix = 1,
    bad_apply = 2,
    too_may_arguments = 3,
  };

  /// EvlaErrorValue
  struct EvalErrorValue
  {
    /// error type
    eval_error_type error_type;
  };

  namespace interface {

    /// EvalError
    using EvalError = Box<EvalErrorValue>;

  } // namespace interface

  namespace interface {

    namespace eval_error {

      /// evaluation error
      class eval_error : public std::runtime_error
      {
      public:
        /// Ctor
        template <class T>
        explicit eval_error(const char* msg = "eval_error")
          : std::runtime_error(msg)
        {
        }
      };

      /// bad apply for fix
      class bad_fix : public eval_error
      {
      public:
        bad_fix()
          : eval_error("Invalid use of fix operator")
        {
        }
      };

      /// bad apply (apply for value)
      class bad_apply : public eval_error
      {
      public:
        bad_apply()
          : eval_error("Invalid application")
        {
        }
      };

      /// too many arguments
      class too_many_arguments : public eval_error
      {
      public:
        too_many_arguments()
          : eval_error("Invalid number of arguments")
        {
        }
      };

    } // namespace eval_error

  } // namespace interface

  // ------------------------------------------
  // conversion

  object_ptr<Exception> to_Exception(const eval_error::eval_error& e)
  {
    return make_object<Exception>(
      e.what(), make_object<EvalError>(eval_error_type::unknown));
  }

  object_ptr<Exception> to_Exception(const eval_error::bad_fix& e)
  {
    return make_object<Exception>(
      e.what(), make_object<EvalError>(eval_error_type::bad_fix));
  }

  object_ptr<Exception> to_Exception(const eval_error::bad_apply& e)
  {
    return make_object<Exception>(
      e.what(), make_object<EvalError>(eval_error_type::bad_apply));
  }

  object_ptr<Exception> to_Exception(const eval_error::too_many_arguments& e)
  {
    return make_object<Exception>(
      e.what(), make_object<EvalError>(eval_error_type::too_may_arguments));
  }

} // namespace TORI_NS::detail

// EvalError
TORI_DECL_TYPE(EvalError)