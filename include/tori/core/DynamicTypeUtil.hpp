// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "TypeGen.hpp"

namespace TORI_NS::detail {

  namespace interface {
    /// \brief get **RAW** type of the object
    /// \notes NO null check.
    /// \notes use type_of() to get actual type of terms.
    ObjectPtr<const Type> get_type(const ObjectPtr<>& obj) {
      assert(!obj);
      return obj.info_table()->obj_type;
    }

    /// is_value_type
    bool is_value_type(const ObjectPtr<const Type>& tp) {
      if (std::get_if<ValueType>(tp.value()))
        return true;
      else
        return false;
    }
    /// is_arrow_type
    bool is_arrow_type(const ObjectPtr<const Type>& tp) {
      if (std::get_if<ArrowType>(tp.value()))
        return true;
      else
        return false;
    }
    /// is_vartype
    bool is_vartype(const ObjectPtr<const Type>& tp) {
      if (std::get_if<VarType>(tp.value()))
        return true;
      else
        return false;
    }
    /// has_value_type
    bool has_value_type(const ObjectPtr<>& obj) {
      return is_value_type(get_type(obj));
    }
    /// has_arrow_type
    bool has_arrow_type(const ObjectPtr<>& obj) {
      return is_arrow_type(get_type(obj));
    }
    /// has_vartype
    bool has_vartype(const ObjectPtr<>& obj) {
      return is_vartype(get_type(obj));
    }
  } // namespace interface

  ObjectPtr<const Type> copy_type_impl(const ObjectPtr<const Type>& ptp) {
    if (std::get_if<ValueType>(ptp.value())) { return ptp; }
    if (std::get_if<VarType>(ptp.value())) { return ptp; }
    if (auto arrow = std::get_if<ArrowType>(ptp.value())) {
      auto ret = new Type{ArrowType{copy_type_impl(arrow->captured),
                                    copy_type_impl(arrow->returns)}};
      return ret;
    }

    assert(false);
    return ptp;
  }

  namespace interface {
    /// Deep copy type object
    ObjectPtr<const Type> copy_type(const ObjectPtr<const Type>& tp) {
      return copy_type_impl(tp);
    }
  } // namespace interface

  bool same_type_impl(
    const ObjectPtr<const Type>& lhs, const ObjectPtr<const Type>& rhs) {
    const auto& left = *lhs;
    const auto& right = *rhs;
    if (auto lvar = std::get_if<ValueType>(&left)) {
      if (auto rvar = std::get_if<ValueType>(&right))
        return ValueType::compare(*lvar, *rvar);
    }
    if (auto larr = std::get_if<ArrowType>(&left)) {
      if (auto rarr = std::get_if<ArrowType>(&right))
        return same_type_impl(larr->captured, rarr->captured) &&
               same_type_impl(larr->returns, rarr->returns);
    }
    if (auto lany = std::get_if<VarType>(&left)) {
      if (auto rany = std::get_if<VarType>(&right)) return lany->id == rany->id;
    }
    assert(false);
    return false;
  }

  bool same_type(
    const ObjectPtr<const Type>& lhs, const ObjectPtr<const Type>& rhs) {
    if (lhs.head() == rhs.head()) return true;
    return same_type_impl(lhs, rhs);
  }

  /// check subtype relation T1 :< T2
  bool subtype(
    const ObjectPtr<const Type>& t1, const ObjectPtr<const Type>& t2) {
    if (same_type(t1, t2)) return true;
    if (same_type(object_type<HeapObject>::get(), t2)) return true;
    if (is_arrow_type(t1), is_arrow_type(t2)) {
      auto& t1c = std::get_if<ArrowType>(t1.value())->captured;
      auto& t1r = std::get_if<ArrowType>(t1.value())->returns;
      auto& t2c = std::get_if<ArrowType>(t2.value())->captured;
      auto& t2r = std::get_if<ArrowType>(t2.value())->returns;
      return subtype(t2c, t1c) && subtype(t1r, t2r);
    }
    // TODO support dynamic subtype declaration
    return false;
  }

  struct TyArrow {
    ObjectPtr<const Type> from;
    ObjectPtr<const Type> to;
  };

  ObjectPtr<const Type> subst_type_impl(
    const TyArrow& ta, const ObjectPtr<const Type>& in) {
    auto& from = ta.from;
    auto& to = ta.to;
    if (std::get_if<ValueType>(in.value())) {
      if (same_type(in, from)) return to;
    }
    if (std::get_if<ArrowType>(in.value())) {
      if (same_type(in, from)) return to;
      return in;
    }
    if (auto arrow = std::get_if<ArrowType>(in.value())) {
      if (same_type(in, from)) return to;
      return new Type{ArrowType{subst_type_impl(ta, arrow->captured),
                                subst_type_impl(ta, arrow->returns)}};
    }
    assert(false);
    return in;
  }

  /// emulate type-substitution
  ObjectPtr<const Type> subst_type(
    const TyArrow& ta, const ObjectPtr<const Type>& in) {
    return subst_type_impl(ta, in);
  }

  // ------------------------------------------
  // Constr
  // ------------------------------------------

  /// Type constraint
  struct Constr {
    ObjectPtr<const Type> t1;
    ObjectPtr<const Type> t2;
  };

  /// subst_constr
  Constr subst_constr(const TyArrow& ta, const Constr& constr) {
    return {subst_type(ta, constr.t1), subst_type(ta, constr.t2)};
  }

  /// subst_constr_all
  std::vector<Constr> subst_constr_all(
    const TyArrow& ta, const std::vector<Constr>& cs) {
    std::vector<Constr> ret;
    ret.reserve(cs.size());
    for (auto&& c : cs)
      ret.push_back(subst_constr(ta, c));
    return ret;
  }

  // ------------------------------------------
  // Occurs
  // ------------------------------------------

  /// occurs
  bool occurs(const ObjectPtr<const Type>& x, const ObjectPtr<const Type>& t) {
    if (std::get_if<ValueType>(t.value())) return false;
    if (std::get_if<VarType>(t.value())) return same_type(x, t);
    if (auto arrow = std::get_if<ArrowType>(t.value()))
      return occurs(x, arrow->captured) || occurs(x, arrow->returns);
    return false;
  }

  // ------------------------------------------
  // Unify
  // ------------------------------------------

  void unify_impl(std::vector<Constr>& cs, std::vector<TyArrow>& ta) {
    while (!cs.empty()) {
      auto c = cs.back();
      cs.pop_back();
      if (subtype(c.t1, c.t2)) continue;
      if (!occurs(c.t1, c.t2)) {
        auto arr = TyArrow{c.t1, c.t2};
        subst_constr_all(arr, cs);
        ta.push_back(arr);
        continue;
      }
      if (!occurs(c.t2, c.t1)) {
        auto arr = TyArrow{c.t2, c.t1};
        subst_constr_all(arr, cs);
        ta.push_back(arr);
        continue;
      }
      if (auto arrow1 = std::get_if<ArrowType>(c.t1.value())) {
        if (auto arrow2 = std::get_if<ArrowType>(c.t2.value())) {
          cs.push_back({arrow1->captured, arrow2->captured});
          cs.push_back({arrow1->returns, arrow2->returns});
          continue;
        }
      }
      throw std::runtime_error("Unification error: Unsolvalbe constraints");
    }
  }

  /// unify
  std::vector<TyArrow> unify(const std::vector<Constr>& cs) {
    auto _cs = cs;
    std::vector<TyArrow> as;
    unify_impl(_cs, as);
    return as;
  }

  // ------------------------------------------
  // Recon
  // ------------------------------------------

  ObjectPtr<const Type> genvar() {
    auto var = make_object<Type>(VarType{0});
    std::get_if<VarType>(var.value())->id = uintptr_t(var.head());
    return ObjectPtr<const Type>(var);
  }

} // namespace TORI_NS::detail