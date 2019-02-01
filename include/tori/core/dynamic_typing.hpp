// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

#include "type_gen.hpp"
#include "apply.hpp"
#include "exception.hpp"
#include "type_error.hpp"
#include "utility.hpp"

#include <vector>

namespace TORI_NS::detail {

  namespace interface {

    // fwd
    template <class T, class U>
    [[nodiscard]] object_ptr<T> value_cast_if(const object_ptr<U>& obj) noexcept;
    template <class T, class U>
    [[nodiscard]] object_ptr<T> value_cast_if(object_ptr<U>&& obj) noexcept;

    // ------------------------------------------
    // Utils

    /// \brief get **RAW** type of the object
    /// \notes NO null check.
    /// \notes use type_of() to get actual type of terms.
    template <class T>
    [[nodiscard]] inline object_ptr<const Type>
      get_type(const object_ptr<T>& obj)
    {
      return obj.info_table()->obj_type;
    }

    /// is_value_type
    [[nodiscard]] inline bool is_value_type(const object_ptr<const Type>& tp)
    {
      if (get_if<ValueType>(tp.value()))
        return true;
      else
        return false;
    }

    /// is_arrow_type
    [[nodiscard]] inline bool is_arrow_type(const object_ptr<const Type>& tp)
    {
      if (get_if<ArrowType>(tp.value()))
        return true;
      else
        return false;
    }

    /// is_vartype
    [[nodiscard]] inline bool is_vartype(const object_ptr<const Type>& tp)
    {
      if (get_if<VarType>(tp.value()))
        return true;
      else
        return false;
    }

    /// has_value_type
    [[nodiscard]] inline bool has_value_type(const object_ptr<>& obj)
    {
      return is_value_type(get_type(obj));
    }

    /// has_arrow_type
    [[nodiscard]] inline bool has_arrow_type(const object_ptr<>& obj)
    {
      return is_arrow_type(get_type(obj));
    }

    /// has_vartype
    [[nodiscard]] inline bool has_vartype(const object_ptr<>& obj)
    {
      return is_vartype(get_type(obj));
    }

  } // namespace interface

  [[nodiscard]] inline object_ptr<const Type>
    copy_type_impl(const object_ptr<const Type>& ptp)
  {
    if (get_if<ValueType>(ptp.value()))
      return ptp;
    if (get_if<VarType>(ptp.value()))
      return ptp;
    if (auto arrow = get_if<ArrowType>(ptp.value())) {
      auto ret = new Type(ArrowType {copy_type_impl(arrow->captured),
                                     copy_type_impl(arrow->returns)});
      return ret;
    }

    unreachable();
  }

  namespace interface {

    /// Deep copy type object
    [[nodiscard]] inline object_ptr<const Type>
      copy_type(const object_ptr<const Type>& tp)
    {
      return copy_type_impl(tp);
    }

  } // namespace interface

  [[nodiscard]] inline bool same_type_impl(
    const object_ptr<const Type>& lhs,
    const object_ptr<const Type>& rhs)
  {
    if (lhs.get() == rhs.get())
      return true;

    const auto& left = *lhs;
    const auto& right = *rhs;

    if (auto lvar = get_if<ValueType>(&left)) {
      if (auto rvar = get_if<ValueType>(&right))
        return ValueType::compare(*lvar, *rvar);
      else
        return false;
    }
    if (auto larr = get_if<ArrowType>(&left)) {
      if (auto rarr = get_if<ArrowType>(&right))
        return same_type_impl(larr->captured, rarr->captured) &&
               same_type_impl(larr->returns, rarr->returns);
      else
        return false;
    }
    if (auto lany = get_if<VarType>(&left)) {
      if (auto rany = get_if<VarType>(&right))
        return lany->id == rany->id;
      else
        return false;
    }

    unreachable();
  }

  namespace interface {

    /// check type equality
    [[nodiscard]] inline bool same_type(
      const object_ptr<const Type>& lhs,
      const object_ptr<const Type>& rhs)
    {
      return same_type_impl(lhs, rhs);
    }

    /// has_type
    template <class T, class U>
    [[nodiscard]] bool has_type(const object_ptr<U>& obj)
    {
      if (same_type(get_type(obj), object_type<T>()))
        return true;
      return false;
    }

  } // namespace interface

  struct TyArrow
  {
    object_ptr<const Type> from;
    object_ptr<const Type> to;
  };

  [[nodiscard]] inline object_ptr<const Type>
    subst_type_impl(const TyArrow& ta, const object_ptr<const Type>& in)
  {
    auto& from = ta.from;
    auto& to = ta.to;
    if (get_if<ValueType>(in.value())) {
      if (same_type(in, from))
        return to;
      return in;
    }
    if (get_if<VarType>(in.value())) {
      if (same_type(in, from))
        return to;
      return in;
    }
    if (auto arrow = get_if<ArrowType>(in.value())) {
      if (same_type(in, from))
        return to;
      return new Type(ArrowType {subst_type_impl(ta, arrow->captured),
                                 subst_type_impl(ta, arrow->returns)});
    }

    unreachable();
  }

  /// emulate type-substitution
  [[nodiscard]] inline object_ptr<const Type>
    subst_type(const TyArrow& ta, const object_ptr<const Type>& in)
  {
    return subst_type_impl(ta, in);
  }

  [[nodiscard]] inline object_ptr<const Type> subst_type_all(
    const std::vector<TyArrow>& tas,
    const object_ptr<const Type>& ty)
  {
    auto t = ty;
    for (auto tyArrow : tas) {
      t = subst_type(tyArrow, t);
    }
    return t;
  }

  // ------------------------------------------
  // Constr

  /// Type constraint
  struct Constr
  {
    object_ptr<const Type> t1;
    object_ptr<const Type> t2;
  };

  /// subst_constr
  [[nodiscard]] inline Constr
    subst_constr(const TyArrow& ta, const Constr& constr)
  {
    return {subst_type(ta, constr.t1), subst_type(ta, constr.t2)};
  }

  /// subst_constr_all
  [[nodiscard]] inline std::vector<Constr>
    subst_constr_all(const TyArrow& ta, const std::vector<Constr>& cs)
  {
    auto ret = std::vector<Constr> {};
    ret.reserve(cs.size());
    for (auto&& c : cs) ret.push_back(subst_constr(ta, c));
    return ret;
  }

  // ------------------------------------------
  // Occurs

  /// occurs
  [[nodiscard]] inline bool
    occurs(const object_ptr<const Type>& x, const object_ptr<const Type>& t)
  {
    if (get_if<ValueType>(t.value()))
      return false;
    if (get_if<VarType>(t.value()))
      return same_type(x, t);
    if (auto arrow = get_if<ArrowType>(t.value()))
      return occurs(x, arrow->captured) || occurs(x, arrow->returns);

    unreachable();
  }

  inline void unify_func_impl(
    std::vector<Constr>& cs,
    std::vector<TyArrow>& ta,
    const object_ptr<>& src)
  {
    while (!cs.empty()) {
      auto c = cs.back();
      cs.pop_back();
      if (same_type(c.t1, c.t2))
        continue;
      if (is_vartype(c.t2)) {
        if (likely(!occurs(c.t2, c.t1))) {
          auto arr = TyArrow {c.t2, c.t1};
          cs = subst_constr_all(arr, cs);
          ta.push_back(arr);
          continue;
        }
        throw type_error::circular_constraint(src, c.t1);
      }
      if (is_vartype(c.t1)) {
        if (likely(!occurs(c.t1, c.t2))) {
          auto arr = TyArrow {c.t1, c.t2};
          cs = subst_constr_all(arr, cs);
          ta.push_back(arr);
          continue;
        }
        throw type_error::circular_constraint(src, c.t1);
      }
      if (auto arrow1 = get_if<ArrowType>(c.t1.value())) {
        if (auto arrow2 = get_if<ArrowType>(c.t2.value())) {
          cs.push_back({arrow1->captured, arrow2->captured});
          cs.push_back({arrow1->returns, arrow2->returns});
          continue;
        }
      }
      throw type_error::type_missmatch(src, c.t1, c.t2);
    }
  }

  /// unify
  /// \param cs Type constraints
  /// \param src Source node (for error handling)
  [[nodiscard]] inline std::vector<TyArrow>
    unify(const std::vector<Constr>& cs, const object_ptr<>& src)
  {
    auto _cs = cs;
    auto as = std::vector<TyArrow> {};
    unify_func_impl(_cs, as, src);
    return as;
  }

  // ------------------------------------------
  // Recon

  [[nodiscard]] inline object_ptr<const Type> genvar()
  {
    auto var = make_object<Type>(VarType {});
    get_if<VarType>(var.value())->id = uintptr_t(var.get());
    return object_ptr<const Type>(var);
  }

  inline void vars_impl(
    const object_ptr<const Type>& tp,
    std::vector<object_ptr<const Type>>& vars)
  {
    if (is_value_type(tp))
      return;
    if (is_vartype(tp)) {
      return [&]() {
        for (auto&& v : vars) {
          if (same_type(v, tp))
            return;
        }
        vars.push_back(tp);
      }();
    }
    if (is_arrow_type(tp)) {
      vars_impl(get_if<ArrowType>(tp.value())->captured, vars);
      vars_impl(get_if<ArrowType>(tp.value())->returns, vars);
      return;
    }

    assert(false);
    unreachable();
  }

  // get list of type variables
  [[nodiscard]] inline std::vector<object_ptr<const Type>>
    vars(const object_ptr<const Type>& tp)
  {
    auto vars = std::vector<object_ptr<const Type>> {};
    vars_impl(tp, vars);
    return vars;
  }

  /// create fresh polymorphic closure type
  [[nodiscard]] inline object_ptr<const Type>
    genpoly(const object_ptr<const Type>& tp)
  {
    if (!is_arrow_type(tp))
      return tp;
    auto vs = vars(tp);
    auto t = tp;
    for (auto v : vs) {
      auto a = TyArrow {v, genvar()};
      t = subst_type(a, tp);
    }
    return t;
  }

  // typing
  [[nodiscard]] inline const object_ptr<const Type>
    type_of_func_impl(const object_ptr<>& obj)
  {
    // Apply
    if (auto apply = value_cast_if<ApplyR>(obj)) {
      if (auto fix = value_cast_if<Fix>(apply->app())) {
        auto _t1 = type_of_func_impl(apply->arg());
        auto _t = genvar();
        auto c = std::vector {Constr {_t1, new Type(ArrowType {_t, _t})}};
        auto s = unify(std::move(c), obj);
        return subst_type_all(s, _t); // FIXME should use genvar() instead of _t?
      } else {
        auto _t1 = type_of_func_impl(apply->app());
        auto _t2 = type_of_func_impl(apply->arg());
        auto _t = genvar();
        auto c = std::vector {Constr {_t1, new Type(ArrowType {_t2, _t})}};
        auto s = unify(std::move(c), obj);
        return subst_type_all(s, _t);
      }
    }
    // value -> value
    if (has_value_type(obj))
      return get_type(obj);
    // var -> var
    if (has_vartype(obj))
      return get_type(obj);
    // arrow -> genpoly arrow
    if (has_arrow_type(obj))
      return genpoly(get_type(obj));

    assert(false);
    unreachable();
  }

  namespace interface {

    // type_of
    [[nodiscard]] inline object_ptr<const Type> type_of(const object_ptr<>& obj)
    {
      return type_of_func_impl(obj);
    }

  } // namespace interface

} // namespace TORI_NS::detail