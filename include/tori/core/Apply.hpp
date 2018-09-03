// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once 

/// \file Apply

#include "DynamicTypeUtil.hpp"
#include "Fix.hpp"

namespace TORI_NS::detail {

  /// ApplyValue for runtime
  struct ApplyRValue {
    template <class App, class Arg>
    ApplyRValue(App&& app, Arg&& arg)
      : m_app{std::forward<App>(app)}, m_arg{std::forward<Arg>(arg)} {}

    const ObjectPtr<>& app() const {
      assert(m_app != nullptr);
      return m_app;
    }
    const ObjectPtr<>& arg() const {
      assert(m_app != nullptr);
      return m_arg;
    }

    bool evaluated() const {
      return m_app == nullptr;
    }

    const ObjectPtr<>& get_cache() const {
      assert(evaluated());
      return m_arg;
    }

    void set_cache(const ObjectPtr<>& obj) {
      assert(!evaluated());
      m_app = nullptr;
      m_arg = obj;
    }

  private:
    /// closure
    /// when evaluated: nullptr
    ObjectPtr<> m_app;
    /// argument
    /// when evaluated: result
    ObjectPtr<> m_arg; 
  };

  namespace interface {
    /// runtime apply object
    using ApplyR = BoxedHeapObject<ApplyRValue>;
  } // namespace interface

  namespace interface {
    /// compile time apply object
    template <class App, class Arg>
    struct Apply : ApplyR {
      /// base
      using base = ApplyR;
      /// term
      using term = TmApply<typename App::term, typename Arg::term>;

      Apply(const ObjectPtr<App>& ap, const ObjectPtr<Arg>& ar)
        : base{ObjectPtr<>{ap}, ObjectPtr<>{ar}} {}
      Apply(ObjectPtr<App>&& ap, const ObjectPtr<Arg>& ar)
        : base{ObjectPtr<>{std::move(ap)}, ObjectPtr<>{ar}} {}
      Apply(const ObjectPtr<App>& ap, ObjectPtr<Arg>&& ar)
        : base{ObjectPtr<>{ap}, ObjectPtr<>{std::move(ar)}} {}
      Apply(ObjectPtr<App>&& ap, ObjectPtr<Arg>&& ar)
        : base{ObjectPtr<>{std::move(ap)}, ObjectPtr<>{std::move(ar)}} {}

      // NOTE: msvc 15.8 Preview 3 can't compile CTAD with delegate constructors

      Apply(const ObjectPtr<App>& ap, Arg* ar) //
        : base{ObjectPtr<>{ap}, ObjectPtr<>{ar}} {}
      Apply(App* ap, const ObjectPtr<Arg>& ar) //
        : base{ObjectPtr<>{ap}, ObjectPtr<>{ar}} {}

      Apply(App* ap, ObjectPtr<Arg>&& ar)
        : base{ObjectPtr<>{ap}, ObjectPtr<>{std::move(ar)}} {}
      Apply(ObjectPtr<App>&& ap, Arg* ar)
        : base{ObjectPtr<>{std::move(ap)}, ObjectPtr<>{ar}} {}

      Apply(App* ap, Arg* ar) //
        : base{ObjectPtr<>{ap}, ObjectPtr<>{ar}} {}
    };
  } // namespace interface

  template <class T>
  struct is_valid_app_arg : std::false_type {};
  template <class T>
  struct is_valid_app_arg<T*> {
    static constexpr bool value = std::is_base_of_v<HeapObject, T>;
  };
  template <class T>
  struct is_valid_app_arg<ObjectPtr<T>> : std::true_type {};
  template <class T>
  static constexpr bool is_valid_app_arg_v =
    is_valid_app_arg<std::decay_t<T>>::value;

  namespace interface {
    /// static apply operator
    template <
      class T1,
      class T2,
      class =
        std::enable_if_t<is_valid_app_arg_v<T1> && is_valid_app_arg_v<T2>>>
    [[nodiscard]] auto operator<<(T1&& lhs, T2&& rhs) {
      return ObjectPtr{new Apply{std::forward<T1>(lhs), std::forward<T2>(rhs)}};
    }
    /// dynamic apply operator
    template <
      class T1,
      class T2,
      class =
        std::enable_if_t<is_valid_app_arg_v<T1> && is_valid_app_arg_v<T2>>>
    [[nodiscard]] ObjectPtr<> operator>>(T1&& lhs, T2&& rhs) {
      return new ApplyR{ObjectPtr<>(std::forward<T1>(lhs)),
                        ObjectPtr<>(std::forward<T2>(rhs))};
    }
  } // namespace interface

} // namespace TORI_NS::detail

namespace TORI_NS {
  TORI_DECL_TYPE(ApplyR)
} // namespace TORI_NS