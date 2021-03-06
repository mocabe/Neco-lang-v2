// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#if !defined(TORI_NO_LOCAL_INCLUDE)
#  include "../config/config.hpp"
#  include "exception.hpp"
#endif

namespace TORI_NS::detail {

  // ------------------------------------------
  // Eval errors

  namespace interface {

    /// eval_error_type
    enum class eval_error_type : uint64_t
    {
      unknown = 0,
      bad_fix = 1,
      bad_apply = 2,
      too_may_arguments = 3,
    };

  } // namespace interface

  /// eval_error_value
  struct eval_error_object_value
  {
    /// error type
    eval_error_type error_type;
  };

  namespace interface {

    /// EvalError
    using EvalError = Box<eval_error_object_value>;

  } // namespace interface

  namespace interface {

    namespace eval_error {

      /// evaluation error
      class eval_error : public std::runtime_error
      {
      public:
        /// Ctor
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

  [[nodiscard]] inline object_ptr<Exception>
    to_Exception(const eval_error::eval_error& e)
  {
    return make_object<Exception>(
      e.what(), make_object<EvalError>(eval_error_type::unknown));
  }

  [[nodiscard]] inline object_ptr<Exception>
    to_Exception(const eval_error::bad_fix& e)
  {
    return make_object<Exception>(
      e.what(), make_object<EvalError>(eval_error_type::bad_fix));
  }

  [[nodiscard]] inline object_ptr<Exception>
    to_Exception(const eval_error::bad_apply& e)
  {
    return make_object<Exception>(
      e.what(), make_object<EvalError>(eval_error_type::bad_apply));
  }

  [[nodiscard]] inline object_ptr<Exception>
    to_Exception(const eval_error::too_many_arguments& e)
  {
    return make_object<Exception>(
      e.what(), make_object<EvalError>(eval_error_type::too_may_arguments));
  }

} // namespace TORI_NS::detail

// EvalError
TORI_DECL_TYPE(EvalError)