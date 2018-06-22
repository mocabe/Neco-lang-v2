// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file Thunk

#include "DynamicTypeUtil.hpp"

namespace TORI_NS::detail {

  namespace interface {
    template <class T>
    [[nodiscard]] auto eval(const ObjectPtr<T>& obj)
      -> ObjectPtr<assume_object_type_t<type_of_t<typename T::term>>>;
  } // namespace interface

  class ThunkValue {
  public:
    // custom term
    using term = TmThunk<HeapObject>;
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

      template <
        class U,
        class = std::enable_if_t<
          !std::is_same_v<T, U> && //
          std::is_same_v<
            type_of_t<typename T::term>,
            type_of_t<typename U::term>>>>
      Thunk(const ObjectPtr<U>& obj) : ThunkR(obj) {}
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