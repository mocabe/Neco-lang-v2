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

  namespace interface {
    /// closure_cast
    ///
    /// dynamically cast object to specified closure type.
    /// \throws bad_value_cast when fail.
    template <class T, class U>
    [[nodiscard]] ObjectPtr<T> closure_cast(const ObjectPtr<U>& obj) {
      static_assert(has_TmClosure_v<T>, "T is not closure type");
      assert(obj);
      auto o = ObjectPtr<>(obj);
      if (has_type<T>(o)) {
        // +1
        if (o.head()) ++(o.head()->refcount.atomic);
        return static_cast<T*>(o.head());
      }
      throw bad_closure_cast{get_type(o), object_type<T>::get()};
    }

    /// closure_cast
    ///
    /// dynamically cast object to specified closure type.
    /// \throws bad_value_cast when fail.
    template <class T, class U>
    [[nodiscard]] ObjectPtr<T> closure_cast(ObjectPtr<U>&& obj) {
      static_assert(has_TmClosure_v<T>, "T is not closure type");
      assert(obj);
      auto o = ObjectPtr<>(std::move(obj));
      if (has_type<T>(o)) {
        // move
        auto r = static_cast<T*>(o.m_ptr);
        o.m_ptr = nullptr;
        return r;
      }
      throw bad_closure_cast{get_type(o), object_type<T>::get()};
    }

    /// closure_cast_if
    ///
    /// dynamically cast object to specified closure type.
    /// \returns nullptr when fail.
    template <class T, class U>
    [[nodiscard]] ObjectPtr<T> closure_cast_if(
      const ObjectPtr<U>& obj) noexcept {
      static_assert(has_TmClosure_v<T>, "T is not closure type");
      assert(obj);
      auto o = ObjectPtr<>(obj);
      if (has_type<T>(o)) {
        // +1
        if (o.head()) ++(o.head()->refcount.atomic);
        return static_cast<T*>(o.head());
      }
      return nullptr;
    }

    /// closure_cast_if
    ///
    /// dynamically cast object to specified closure type.
    /// \returns nullptr when fail.
    template <class T, class U>
    [[nodiscard]] ObjectPtr<T> closure_cast_if(ObjectPtr<U>&& obj) noexcept {
      static_assert(has_TmClosure_v<T>, "T is not closure type");
      assert(obj);
      auto o = ObjectPtr<>(std::move(obj));
      if (has_type<T>(o)) {
        // move
        auto r = static_cast<T*>(o.m_ptr);
        o.m_ptr = nullptr;
        return r;
      }
      return nullptr;
    }
  } // namespace interface
} // namespace TORI_NS::detail

namespace TORI_NS {
  TORI_DECL_TYPE(BadClosureCast)
} // namespace TORI_NS