// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

///\brief closure cast

#include "DynamicTypeUtil.hpp"
#include <exception>

namespace TORI_NS::detail {

  /// bad_closure_cast exception
  class bad_closure_cast : public std::logic_error {
  public:
    explicit bad_closure_cast(
      ObjectPtr<const Type> from, ObjectPtr<const Type> to)
      : std::logic_error("bad_closure_cast")
      , m_from{std::move(from)}
      , m_to{std::move(to)} {}
    ObjectPtr<const Type> from() const {
      return m_from;
    }
    ObjectPtr<const Type> to() const {
      return m_to;
    }

  private:
    /// cast from
    ObjectPtr<const Type> m_from;
    /// cast to
    ObjectPtr<const Type> m_to;
  };

  /// BadClosureCastValue
  struct BadClosureCastValue {
    ObjectPtr<const Type> from;
    ObjectPtr<const Type> to;
  };

  namespace interface {
    /// Exception object for bad_closure_cast exception
    using BadClosureCast = BoxedHeapObject<detail::BadClosureCastValue>;
  } // namespace interface

  /// closure_cast
  ///
  /// dynamically cast object to specified closure type.
  /// \throws bad_value_cast when fail.
  template <class T>
  ObjectPtr<T> closure_cast(const ObjectPtr<>& obj) {
    static_assert(has_TmClosure_v<T>, "T is not closure type");
    assert(obj);
    if (has_type<T>(obj)) {
      // +1
      if (obj.head()) ++(obj.head()->refcount.atomic);
      return static_cast<T*>(obj.head());
    }
    throw bad_closure_cast{get_type(obj), object_type<T>::get()};
  }

  /// closure_cast
  ///
  /// dynamically cast object to specified closure type.
  /// \throws bad_value_cast when fail.
  template <class T>
  ObjectPtr<T> closure_cast(ObjectPtr<>&& obj) {
    static_assert(has_TmClosure_v<T>, "T is not closure type");
    assert(obj);
    if (has_type<T>(obj)) {
      // move
      auto r = static_cast<T*>(obj.m_ptr);
      obj.m_ptr = nullptr;
      return r;
    }
    throw bad_closure_cast{get_type(obj), object_type<T>::get()};
  }

  /// closure_cast_if
  ///
  /// dynamically cast object to specified closure type.
  /// \returns nullptr when fail.
  template <class T>
  ObjectPtr<T> closure_cast_if(const ObjectPtr<>& obj) noexcept {
    static_assert(has_TmClosure_v<T>, "T is not closure type");
    assert(obj);
    if (has_type<T>(obj)) {
      // +1
      if (obj.head()) ++(obj.head()->refcount.atomic);
      return static_cast<T*>(obj.head());
    }
    return nullptr;
  }

  /// closure_cast_if
  ///
  /// dynamically cast object to specified closure type.
  /// \returns nullptr when fail.
  template <class T>
  ObjectPtr<T> closure_cast_if(ObjectPtr<>&& obj) noexcept {
    static_assert(has_TmClosure_v<T>, "T is not closure type");
    assert(obj);
    if (has_type<T>(obj)) {
      // move
      auto r = static_cast<T*>(obj.m_ptr);
      obj.m_ptr = nullptr;
      return r;
    }
    return nullptr;
  }
} // namespace TORI_NS::detail

namespace TORI_NS {
  TORI_DECL_TYPE(BadClosureCast)
} // namespace TORI_NS