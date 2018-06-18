// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once 

/// \file Apply

#include "DynamicTypeUtil.hpp"
#include "Primitive.hpp"
#include "Fix.hpp"

namespace TORI_NS::detail {

  /// Expected
  template <class T>
  struct Expected {
    const ObjectPtr<> obj;
  };

  /// ApplyValue for runtime
  struct ApplyRValue {
    ObjectPtr<> app;
    ObjectPtr<> arg;
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

      Apply(const Expected<App>& ap, const Expected<Arg>& ar)
        : base{ap.obj, ar.obj} {}
      Apply(Expected<App>&& ap, const Expected<Arg>& ar)
        : base{std::move(ap.obj), ar.obj} {}
      Apply(const Expected<App>& ap, Expected<Arg>&& ar)
        : base{ap.obj, std::move(ar.obj)} {}
      Apply(Expected<App>&& ap, Expected<Arg>&& ar)
        : base{std::move(ap.obj), std::move(ar.obj)} {}

      Apply(const ObjectPtr<App>& ap, const Expected<Arg>& ar)
        : base{ObjectPtr<>{ap}, ar.obj} {}
      Apply(const ObjectPtr<App>& ap, Expected<Arg>&& ar)
        : base{ObjectPtr<>{ap}, std::move(ar.obj)} {}
      Apply(ObjectPtr<App>&& ap, const Expected<Arg>& ar)
        : base{ObjectPtr<>{std::move(ap)}, ar.obj} {}
      Apply(ObjectPtr<App>&& ap, Expected<Arg>&& ar)
        : base{ObjectPtr<>{std::move(ap)}, std::move(ar.obj)} {}

      Apply(const Expected<App>& ap, const ObjectPtr<Arg>& ar)
        : base{ap.obj, ObjectPtr<>{ar}} {}
      Apply(const Expected<App>& ap, ObjectPtr<Arg>& ar)
        : base{ap.obj, ObjectPtr<>{std::move(ar)}} {}
      Apply(Expected<App>&& ap, const ObjectPtr<Arg>& ar)
        : base{std::move(ap.obj), ObjectPtr<>{ar}} {}
      Apply(Expected<App>&& ap, ObjectPtr<Arg>& ar)
        : base{std::move(ap.obj), ObjectPtr<>{std::move(ar)}} {}

      template <template <class> class T>
      Apply(App* ap, const T<Arg>& ar) : Apply{ObjectPtr(ap), ar} {}
      template <template <class> class T>
      Apply(App* ap, T<Arg>&& ar) : Apply{ObjectPtr(ap), std::move(ar)} {}

      template <template <class> class T>
      Apply(const T<App>& ap, Arg* ar) : Apply{ap, ObjectPtr(ar)} {}
      template <template <class> class T>
      Apply(T<App>&& ap, Arg* ar) : Apply{std::move(ap), ObjectPtr(ar)} {}

      Apply(App* ap, Arg* ar) : Apply{ObjectPtr(ap), ObjectPtr(ar)} {}
    };
  } // namespace interface

  template <class T>
  struct is_valid_app_arg : std::false_type {};
  template <class T>
  struct is_valid_app_arg<T*> {
    static constexpr bool value = std::is_base_of_v<HeapObject, T>;
  };
  template <class T>
  struct is_valid_app_arg<Expected<T>> : std::true_type {};
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