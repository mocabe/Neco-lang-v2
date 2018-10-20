#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file Dynamic typing utility

#include "TypeGen.hpp"
#include "Exception.hpp"

namespace TORI_NS::detail {

  namespace interface {
    /// \brief get **RAW** type of the object
    /// \notes NO null check.
    /// \notes use type_of() to get actual type of terms.
    [[nodiscard]] ObjectPtr<const Type> get_type(const ObjectPtr<>& obj) {
      return obj.info_table()->obj_type;
    }

      /// is_value_type
      [[nodiscard]] bool is_value_type(const ObjectPtr<const Type>& tp) {
      if (std::get_if<ValueType>(tp.value()))
        return true;
      else
        return false;
    }
    /// is_arrow_type
    [[nodiscard]] bool is_arrow_type(const ObjectPtr<const Type>& tp) {
      if (std::get_if<ArrowType>(tp.value()))
        return true;
      else
        return false;
    }
      /// is_vartype
      [[nodiscard]] bool is_vartype(const ObjectPtr<const Type>& tp) {
      if (std::get_if<VarType>(tp.value()))
        return true;
      else
        return false;
    }
    /// has_value_type
    [[nodiscard]] bool has_value_type(const ObjectPtr<>& obj) {
      return is_value_type(get_type(obj));
    }
      /// has_arrow_type
      [[nodiscard]] bool has_arrow_type(const ObjectPtr<>& obj) {
      return is_arrow_type(get_type(obj));
    }
    /// has_vartype
    [[nodiscard]] bool has_vartype(const ObjectPtr<>& obj) {
      return is_vartype(get_type(obj));
    }
  } // namespace interface

  [[nodiscard]] ObjectPtr<const Type> copy_type_impl(
    const ObjectPtr<const Type>& ptp) {
    if (std::get_if<ValueType>(ptp.value())) return ptp;
    if (std::get_if<VarType>(ptp.value())) return ptp;
    if (auto arrow = std::get_if<ArrowType>(ptp.value())) {
      auto ret = new Type{ArrowType{copy_type_impl(arrow->captured),
                                    copy_type_impl(arrow->returns)}};
      return ret;
    }

    assert(false);
    return {};
  }

  namespace interface{

    /// Deep copy type object
    [[nodiscard]] ObjectPtr<const Type> copy_type(
      const ObjectPtr<const Type>& tp) { return copy_type_impl(tp); }}

    [[nodiscard]] bool same_type_impl(
      const ObjectPtr<const Type>& lhs, const ObjectPtr<const Type>& rhs) {
    const auto& left = *lhs;
    const auto& right = *rhs;
    if (auto lvar = std::get_if<ValueType>(&left)) {
      if (auto rvar = std::get_if<ValueType>(&right))
        return ValueType::compare(*lvar, *rvar);
      else
        return false;
    }
    if (auto larr = std::get_if<ArrowType>(&left)) {
      if (auto rarr = std::get_if<ArrowType>(&right))
        return same_type_impl(larr->captured, rarr->captured) &&
               same_type_impl(larr->returns, rarr->returns);
      else
        return false;
    }
    if (auto lany = std::get_if<VarType>(&left)) {
      if (auto rany = std::get_if<VarType>(&right))
        return lany->id == rany->id;
      else
        return false;
    }
    assert(false);
    return {};
  }

  namespace interface {
    /// check type equality
    [[nodiscard]] bool same_type(
      const ObjectPtr<const Type>& lhs, const ObjectPtr<const Type>& rhs) {
      if (lhs.get() == rhs.get()) return true;
      return same_type_impl(lhs, rhs);
    }
  } // namespace interface

  struct TyArrow {
    ObjectPtr<const Type> from;
    ObjectPtr<const Type> to;
  };

  [[nodiscard]] ObjectPtr<const Type> subst_type_impl(
    const TyArrow& ta, const ObjectPtr<const Type>& in) {
    auto& from = ta.from;
    auto& to = ta.to;
    if (std::get_if<ValueType>(in.value())) {
      if (same_type(in, from)) return to;
      return in;
    }
    if (std::get_if<VarType>(in.value())) {
      if (same_type(in, from)) return to;
      return in;
    }
    if (auto arrow = std::get_if<ArrowType>(in.value())) {
      if (same_type(in, from)) return to;
      return new Type{ArrowType{subst_type_impl(ta, arrow->captured),
                                subst_type_impl(ta, arrow->returns)}};
    }
    assert(false);
    return {};
  }

    /// emulate type-substitution
    [[nodiscard]] ObjectPtr<const Type> subst_type(
      const TyArrow& ta, const ObjectPtr<const Type>& in) {
    return subst_type_impl(ta, in);
  }

  [[nodiscard]] ObjectPtr<const Type> subst_type_all(
    const std::vector<TyArrow>& tas, const ObjectPtr<const Type>& ty) {
    auto t = ty;
    for (auto tyArrow : tas) {
      t = subst_type(tyArrow, t);
    }
    return t;
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
  [[nodiscard]] Constr subst_constr(const TyArrow& ta, const Constr& constr) {
    return {subst_type(ta, constr.t1), subst_type(ta, constr.t2)};
  }

    /// subst_constr_all
    [[nodiscard]] std::vector<Constr> subst_constr_all(
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
  [[nodiscard]] bool occurs(
    const ObjectPtr<const Type>& x, const ObjectPtr<const Type>& t) {
    if (std::get_if<ValueType>(t.value())) return false;
    if (std::get_if<VarType>(t.value())) return same_type(x, t);
    if (auto arrow = std::get_if<ArrowType>(t.value()))
      return occurs(x, arrow->captured) || occurs(x, arrow->returns);
    return false;
  }

  // ------------------------------------------
  // Unify
  // ------------------------------------------

  namespace interface {
    // unification error
    struct unification_error : type_error {
      unification_error(const std::string& msg, const ObjectPtr<>& src)
        : type_error(msg, src) {}
    };
    // unification error(circular constraint)
    struct unification_circular_constraint : unification_error {
      unification_circular_constraint(
        const ObjectPtr<>& src, const ObjectPtr<const Type>& var)
        : unification_error(
            "unification_circular_constraint: Circular constraints", src)
        , m_var{var} {}

      ObjectPtr<const Type> var() const {
        return m_var;
      }
      ObjectPtr<const Type> m_var;
    };
    // unification error(missmatch)
    struct unification_missmatch : unification_error {
      unification_missmatch(
        const ObjectPtr<>& src,
        const ObjectPtr<const Type>& t1,
        const ObjectPtr<const Type>& t2)
        : unification_error("unification_missmatch: Type missmatch", src)
        , m_t1{t1}
        , m_t2{t2} {}

      ObjectPtr<const Type> t1() const {
        return m_t1;
      }
      ObjectPtr<const Type> t2() const {
        return m_t2;
      }

    private:
      ObjectPtr<const Type> m_t1;
      ObjectPtr<const Type> m_t2;
    };
  }

  void unify_impl(
    std::vector<Constr>& cs, std::vector<TyArrow>& ta, const ObjectPtr<>& src) {
    while (!cs.empty()) {
      auto c = cs.back();
      cs.pop_back();
      if (same_type(c.t1, c.t2)) continue;
      if (is_vartype(c.t2)) {
        if (!occurs(c.t2, c.t1)) {
          auto arr = TyArrow{c.t2, c.t1};
          cs = subst_constr_all(arr, cs);
          ta.push_back(arr);
          continue;
        }
        throw unification_circular_constraint(src, c.t1);
      }
      if (is_vartype(c.t1)) {
        if (!occurs(c.t1, c.t2)) {
          auto arr = TyArrow{c.t1, c.t2};
          cs = subst_constr_all(arr, cs);
          ta.push_back(arr);
          continue;
        }
        throw unification_circular_constraint(src, c.t1);
      }
      if (auto arrow1 = std::get_if<ArrowType>(c.t1.value())) {
        if (auto arrow2 = std::get_if<ArrowType>(c.t2.value())) {
          cs.push_back({arrow1->captured, arrow2->captured});
          cs.push_back({arrow1->returns, arrow2->returns});
          continue;
        }
      }
      throw unification_missmatch(src, c.t1, c.t2);
    }
  }

  /// unify
  /// \param cs Type constraints
  /// \param src Source node (for error handling)
  [[nodiscard]] std::vector<TyArrow> unify(
    const std::vector<Constr>& cs, const ObjectPtr<>& src) {
    auto _cs = cs;
    std::vector<TyArrow> as;
    unify_impl(_cs, as, src);
    return as;
  }

    // ------------------------------------------
    // Recon
    // ------------------------------------------

    [[nodiscard]] ObjectPtr<const Type> genvar() {
    auto var = make_object<Type>(VarType{0});
    std::get_if<VarType>(var.value())->id = uintptr_t(var.get());
    return ObjectPtr<const Type>(var);
  }

  // ------------------------------------------
  // Util
  // ------------------------------------------

  namespace interface {
    /// has_type
    template <class T>
    [[nodiscard]] bool has_type(const ObjectPtr<>& obj) {
      if (same_type(get_type(obj), object_type<T>())) return true;
      return false;
    }
  } // namespace interface

} // namespace TORI_NS::detail