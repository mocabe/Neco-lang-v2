#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file General exception classes

#include "BoxedHeapObject.hpp"
#include "String.hpp"
#include <exception>

namespace TORI_NS::detail {

  /// \brief Exception value
  ///
  /// Exception object holds a pointer to actual exception object.
  /// This is useful to detect exception object with single type comparison but
  /// it requires more computation resources.
  struct ExceptionValue {
    template <class T>
    ExceptionValue(const object_ptr<T>& i) noexcept : info{i} {}
    template <class T>
    ExceptionValue(T* ptr) noexcept : ExceptionValue{object_ptr(ptr)} {}
    /// pointer to exception value
    object_ptr<> info;
  };

  namespace interface {
    /// Exception
    using Exception = BoxedHeapObject<ExceptionValue>;
  } // namespace interface

  // ------------------------------------------
  // Type errors
  // ------------------------------------------

  /// TypeErrorValue
  struct TypeErrorValue {
    /// error message
    object_ptr<String> msg;
    /// source node
    object_ptr<> src;
  };

  namespace interface {
    // TypeError
    using TypeError = BoxedHeapObject<TypeErrorValue>;
  } // namespace interface

  namespace interface {
    /// type_error
    class type_error : public std::logic_error {
    public:
      /// Ctor string
      template <class T>
      explicit type_error(const std::string& what, const object_ptr<T>& src)
        : std::logic_error(what), m_src{object_ptr<>(src)} {}
      template <class T>
      /// Ctor const char*
      explicit type_error(const char* what, const object_ptr<T>& src)
        : std::logic_error(what), m_src{object_ptr<>(src)} {}
      /// get source node
      object_ptr<> src() {
        return m_src;
      }
      /// format function
      virtual std::string format() {
        return {what()};
      }

    private:
      /// source node
      object_ptr<> m_src;
    };
  } // namespace interface

  // ------------------------------------------
  // Eval errors
  // ------------------------------------------

  /// EvlaErrorValue
  struct EvalErrorValue {
    /// error message
    object_ptr<String> msg;
    /// source node
    object_ptr<> src;
  };

  namespace interface {
    /// EvalError
    using EvalError = BoxedHeapObject<EvalErrorValue>;
  } // namespace interface

  namespace interface {
    class eval_error : public std::logic_error {
    public:
      /// Ctor string
      template <class T>
      explicit eval_error(const std::string& what, const object_ptr<T>& src)
        : std::logic_error(what), m_src{object_ptr<>(src)} {}
      /// Ctor const char*
      template <class T>
      explicit eval_error(const char* what, const object_ptr<T>& src)
        : std::logic_error(what), m_src{object_ptr<>(src)} {}
      /// get source node
      object_ptr<> src() {
        return m_src;
      }
      /// format
      virtual std::string format() {
        return {what()};
      }

    private:
      /// source node
      object_ptr<> m_src;
    };
  } // namespace interface

  // ------------------------------------------
  // Result errors
  // ------------------------------------------

  namespace interface {
    /// result error
    class result_error : public std::runtime_error {
    public:
      result_error(const object_ptr<Exception>& result)
        : runtime_error("result_error: Exception detected while evaluation")
        , m_result{result} {}
      result_error(object_ptr<Exception>&& result)
        : runtime_error("result_error: Exception detected while evaluation")
        , m_result{std::move(result)} {}
      object_ptr<Exception> result() {
        return m_result;
      }

    private:
      object_ptr<Exception> m_result;
    };
  } // namespace interface

} // namespace TORI_NS::detail

namespace TORI_NS {
  TORI_DECL_TYPE(Exception)
  TORI_DECL_TYPE(TypeError)
  TORI_DECL_TYPE(EvalError)
} // namespace TORI_NS