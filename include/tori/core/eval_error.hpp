// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "exception.hpp"

namespace TORI_NS::detail {

  // ------------------------------------------
  // Eval errors

  /// EvlaErrorValue
  struct EvalErrorValue
  {
    /// error message
    object_ptr<const String> msg;
    /// source node
    object_ptr<const Object> src;
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
        explicit eval_error(const char* what)
          : std::runtime_error(what)
        {
        }
      };

      /// bad apply for fix
      class bad_fix : public eval_error
      {
      public:
        bad_fix(const char* msg)
          : eval_error(msg)
        {
        }
      };

      /// bad apply (apply for value)
      class bad_apply : public eval_error
      {
      public:
        bad_apply(const char* msg)
          : eval_error(msg)
        {
        }
      };

      /// too many arguments
      class too_many_arguments : public eval_error
      {
      public:
        too_many_arguments(const char* msg)
          : eval_error(msg)
        {
        }
      };

    } // namespace eval_error

  } // namespace interface

} // namespace TORI_NS::detail

// EvalError
TORI_DECL_TYPE(EvalError)