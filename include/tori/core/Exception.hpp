// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file General exception classes

#include "BoxedHeapObject.hpp"
#include "CString.hpp"
#include <exception>

namespace TORI_NS::detail {

  /// \brief Exception value
  ///
  /// Exception object holds a pointer to actual exception object.
  /// This is useful to detect exception object with single type comparison but
  /// it requires more computation resources.
  struct ExceptionValue {
    template <class T>
    ExceptionValue(const ObjectPtr<T>& i) noexcept : info{i} {}
    template <class T>
    ExceptionValue(T* ptr) noexcept : ExceptionValue{ObjectPtr(ptr)} {}
    /// pointer to exception value
    ObjectPtr<> info;
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
    ObjectPtr<CString> msg;
    /// source node
    ObjectPtr<> src;
  };

  namespace interface {
    // TypeError
    using TypeError = BoxedHeapObject<TypeErrorValue>;
  }

  /// type_error
  class type_error : public std::logic_error {
  public:
    /// Ctor string
    template <class T>
    explicit type_error(const std::string& what, const ObjectPtr<T>& src)
      : std::logic_error(what), m_src{ObjectPtr<>(src)} {}
    template <class T>
    /// Ctor const char*
    explicit type_error(const char* what, const ObjectPtr<T>& src)
      : std::logic_error(what), m_src{ObjectPtr<>(src)} {}
    /// get source node
    ObjectPtr<> src() {
      return m_src;
    }
    /// format function
    virtual std::string format() {
      return {what()};
    }

  private:
    /// source node
    ObjectPtr<> m_src;
  };

  // ------------------------------------------
  // Eval errors
  // ------------------------------------------

  /// EvlaErrorValue
  struct EvalErrorValue {
    /// error message
    ObjectPtr<CString> msg;
    /// source node
    ObjectPtr<> src;
  };

  namespace interface {
    /// EvalError
    using EvalError = BoxedHeapObject<EvalErrorValue>;
  }

  class eval_error : public std::logic_error {
  public:
    /// Ctor string
    template <class T>
    explicit eval_error(const std::string& what, const ObjectPtr<T>& src)
      : std::logic_error(what), m_src{ObjectPtr<>(src)} {}
    /// Ctor const char*
    template <class T>
    explicit eval_error(const char* what, const ObjectPtr<T>& src)
      : std::logic_error(what), m_src{ObjectPtr<>(src)} {}
    /// get source node
    ObjectPtr<> src() {
      return m_src;
    }
    /// format
    virtual std::string format() {
      return {what()};
    }

  private:
    /// source node
    ObjectPtr<> m_src;
  };

  // ------------------------------------------
  // Result errors
  // ------------------------------------------

  /// result error
  class result_error : public std::runtime_error {
  public:
    result_error(const ObjectPtr<Exception>& result)
      : runtime_error("result_error: Exception detected in eval")
      , m_result{result} {}
    result_error(ObjectPtr<Exception>&& result)
      : runtime_error("result_error: Exception detected in eval")
      , m_result{std::move(result)} {}
    ObjectPtr<Exception> result() {
      return m_result;
    }

  private:
    ObjectPtr<Exception> m_result;
  };

} // namespace TORI_NS::detail

namespace TORI_NS {
  template <>
  struct object_type_traits<Exception> {
    static constexpr char name[] = "_Exception";
  };
  template <>
  struct object_type_traits<TypeError> {
    static constexpr char name[] = "_TypeError";
  };
  template <>
  struct object_type_traits<EvalError> {
    static constexpr char name[] = "_EvalError";
  };
}