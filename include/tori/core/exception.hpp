#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#include "box.hpp"
#include "string.hpp"

#include <exception>

namespace TORI_NS::detail {

  /// \brief Exception value
  ///
  /// Exception object holds a pointer to actual exception object.
  /// This is useful to detect exception object with single type comparison but
  /// it requires more computation resources.
  struct ExceptionValue
  {
    template <class T>
    ExceptionValue(object_ptr<T> i) noexcept
      : info {std::move(i)}
    {
    }

    template <class T>
    ExceptionValue(T* ptr) noexcept
      : info {ptr}
    {
    }

    /// pointer to exception value
    object_ptr<const Object> info;
  };

  namespace interface {

    /// Exception
    using Exception = Box<ExceptionValue>;

  } // namespace interface

  // ------------------------------------------
  // Type errors

  /// TypeErrorValue
  struct TypeErrorValue
  {
    /// error message
    object_ptr<String> msg;
    /// source node
    object_ptr<const Object> src;
  };

  namespace interface {

    // TypeError
    using TypeError = Box<TypeErrorValue>;

  } // namespace interface

  namespace interface {

    namespace type_error {

      /// type_error
      class type_error : public std::logic_error
      {
      public:
        /// Ctor string
        template <class T>
        explicit type_error(const std::string& what, object_ptr<T> src)
          : std::logic_error(what)
          , m_src {std::move(src)}
        {
        }

        template <class T>
        /// Ctor const char*
        explicit type_error(const char* what, object_ptr<T> src)
          : std::logic_error(what)
          , m_src {std::move(src)}
        {
        }

        /// get source node
        const object_ptr<const Object>& src() const
        {
          return m_src;
        }

      private:
        /// source node
        object_ptr<const Object> m_src;
      };

    } // namespace type_error

  } // namespace interface

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
      class eval_error : public std::logic_error
      {
      public:
        /// Ctor string
        template <class T>
        explicit eval_error(const std::string& what, object_ptr<T> src)
          : std::logic_error(what)
          , m_src {std::move(src)}
        {
        }

        /// Ctor const char*
        template <class T>
        explicit eval_error(const char* what, object_ptr<T> src)
          : std::logic_error(what)
          , m_src {std::move(src)}
        {
        }

        /// get source node
        const object_ptr<const Object>& src() const
        {
          return m_src;
        }

      private:
        /// source node
        object_ptr<const Object> m_src;
      };

    } // namespace eval_error

  } // namespace interface

  // ------------------------------------------
  // Result errors

  namespace interface {

    namespace result_error {

      /// result error
      class result_error : public std::runtime_error
      {
      public:
        result_error(object_ptr<Exception> result)
          : runtime_error("result_error: Exception detected while evaluation")
          , m_result {std::move(result)}
        {
        }

        /// result
        const object_ptr<Exception>& result() const
        {
          return m_result;
        }

      private:
        object_ptr<Exception> m_result;
      };

    } // namespace result_error

  } // namespace interface

} // namespace TORI_NS::detail

// Exception
TORI_DECL_TYPE(Exception)
// TypeError
TORI_DECL_TYPE(TypeError)
// EvalError
TORI_DECL_TYPE(EvalError)