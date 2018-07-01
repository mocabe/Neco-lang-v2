// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file type_of

#include "Function.hpp"
#include "Fix.hpp"
#include "Exception.hpp"
#include "ValueCast.hpp"

namespace TORI_NS::detail {

  void vars_impl(
    const ObjectPtr<const Type>& tp, std::vector<ObjectPtr<const Type>>& vars) {
    if (is_value_type(tp)) return;
    if (is_vartype(tp)) {
      return [&]() {
        for (auto&& v : vars) {
          if (same_type(v, tp)) return;
        }
        vars.push_back(tp);
      }();
    }
    if (is_arrow_type(tp)) {
      vars_impl(std::get_if<ArrowType>(tp.value())->captured, vars);
      vars_impl(std::get_if<ArrowType>(tp.value())->returns, vars);
      return;
    }
    assert(false);
    throw std::invalid_argument("vars_impl(): invalid variant index");
  };

  // get list of type variables
  [[nodiscard]] std::vector<ObjectPtr<const Type>> vars(
    const ObjectPtr<const Type>& tp) {
    std::vector<ObjectPtr<const Type>> vars;
    vars_impl(tp, vars);
    return vars;
  };

  /// create fresh polymorphic closure type
  [[nodiscard]] ObjectPtr<const Type> genpoly(const ObjectPtr<const Type>& tp) {
    if (!is_arrow_type(tp)) return tp;
    auto vs = vars(tp);
    auto t = tp;
    for (auto v : vs) {
      TyArrow a{v, genvar()};
      t = subst_type(a, tp);
    }
    return t;
  };

  [[nodiscard]] const ObjectPtr<const Type> recon_impl(
    const ObjectPtr<>& obj, std::vector<Constr>& constr) {
    // Apply
    if (auto apply = value_cast_if<ApplyR>(obj)) {
      if (auto fix = value_cast_if<Fix>(apply->app)) {
        auto _t1 = recon_impl(apply->arg, constr);
        auto _t = genvar();
        constr.push_back({_t1, new Type(ArrowType{_t, _t})});
        return _t;
      } else {
        auto _t1 = recon_impl(apply->app, constr);
        auto _t2 = recon_impl(apply->arg, constr);
        auto _t = genvar();
        constr.push_back({_t1, new Type(ArrowType{_t2, _t})});
        return _t;
      }
    }
    // value -> value, []
    if (has_value_type(obj)) return get_type(obj);
    // var -> var, []
    if (has_vartype(obj)) return get_type(obj);
    // arrow -> arrow, []
    if (has_arrow_type(obj)) return genpoly(get_type(obj));

    assert(false);
    throw std::invalid_argument("recon_impl(): invalid variant index");
  };

  /// recon
  [[nodiscard]] std::pair<const ObjectPtr<const Type>, std::vector<Constr>>
  recon(const ObjectPtr<>& obj) {
    std::vector<Constr> constr;
    return {recon_impl(obj, constr), constr};
  }

  namespace interface {
    // type_of
    [[nodiscard]] ObjectPtr<const Type> type_of(const ObjectPtr<>& obj) {
      auto [type, constr] = recon(obj);
      return subst_type_all(unify(constr, obj), type);
    }
  } // namespace interface

  namespace interface {
    /// check type
    template <class T>
    void check_type(const ObjectPtr<>& obj) {
      auto t1 = object_type<T>();
      auto t2 = type_of(obj);
      if (!same_type(t1, t2)) throw type_error{"check_type failed", obj};
    }
  } // namespace interface

} // namespace TORI_NS::detail