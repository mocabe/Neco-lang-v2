// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file Thunk

#include "DynamicTypeUtil.hpp"

namespace TORI_NS::detail {

  namespace interface {
    /// evaluation
    template <class TObjectPtr>
    [[nodiscard]] ObjectPtr<> eval(TObjectPtr&& obj);
  } // namespace interface

  class ThunkValue {
  public:
    ThunkValue(const ThunkValue&) = default;
    ThunkValue(ThunkValue&&) = default;
    ThunkValue(const ObjectPtr<>& value) : m_value{value}, m_evaluated{false} {}
    ThunkValue(ObjectPtr<>&& value) : m_value{value}, m_evaluated{false} {}

    bool evaluated() const {
      return m_evaluated;
    }

    ObjectPtr<> value() const {
      return m_value;
    }

    ObjectPtr<> eval() {
      if (m_evaluated) {
        return m_value;
      } else {
        m_value = interface::eval(m_value);
        m_evaluated = true;
        return m_value;
      }
    }

  protected:
    ObjectPtr<> m_value;
    bool_t m_evaluated = false;
  };

  namespace interface {
    // ThunkR
    using ThunkR = BoxedHeapObject<ThunkValue>;
  } // namespace interface

  namespace interface {
    // Thunk<T>
    template <class T>
    struct Thunk : ThunkR {
      using term = TmThunk<T>;
      Thunk() = default;
      Thunk(const Thunk&) = default;
      Thunk(Thunk&&) = default;
      ~Thunk() = default;

      Thunk(const ObjectPtr<T>& obj) : ThunkR(obj) {}
      Thunk(ObjectPtr<T>&& obj) : ThunkR(std::move(obj)) {}

    };
  } // namespace interface

} // namespace TORI_NS::detail

namespace TORI_NS {
  template <>
  struct object_type_traits<ThunkR> {
    static constexpr const char name[] = "_Thunk";
  };
  template <class T>
  struct object_type_traits<Thunk<T>> {
    static constexpr const char name[] = "_Thunk";
  };
}