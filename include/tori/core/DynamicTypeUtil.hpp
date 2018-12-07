#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file Dynamic typing utility

#include "TypeGen.hpp"
#include "Exception.hpp"

namespace TORI_NS::detail {

  namespace interface {

    // ------------------------------------------
    // Exceptions

    namespace type_error {

      /// unification error(circular constraint)
      class circular_constraint : public type_error
      {
      public:
        circular_constraint(
          const object_ptr<>& src,
          const object_ptr<const Type>& var)
          : type_error("Circular constraints", src)
          , m_var {var}
        {
        }

        /// var
        object_ptr<const Type> var() const
        {
          return m_var;
        }

      private:
        object_ptr<const Type> m_var;
      };

      /// unification error(missmatch)
      class type_missmatch : public type_error
      {
      public:
        type_missmatch(
          const object_ptr<>& src,
          const object_ptr<const Type>& t1,
          const object_ptr<const Type>& t2)
          : type_error("Type missmatch", src)
          , m_t1 {t1}
          , m_t2 {t2}
        {
        }

        /// t1
        object_ptr<const Type> t1() const
        {
          return m_t1;
        }

        /// t2
        object_ptr<const Type> t2() const
        {
          return m_t2;
        }

      private:
        /// t1
        object_ptr<const Type> m_t1;
        /// t2
        object_ptr<const Type> m_t2;
      };
    } // namespace type_error

    // ------------------------------------------
    // Utils

    /// \brief get **RAW** type of the object
    /// \notes NO null check.
    /// \notes use type_of() to get actual type of terms.
    [[nodiscard]] TORI_INLINE object_ptr<const Type>
      get_type(const object_ptr<>& obj)
    {
      return obj.info_table()->obj_type;
    };

    /// is_value_type
    [[nodiscard]] TORI_INLINE bool
      is_value_type(const object_ptr<const Type>& tp)
    {
      if (get_if<ValueType>(tp.value()))
        return true;
      else
        return false;
    };

    /// is_arrow_type
    [[nodiscard]] TORI_INLINE bool
      is_arrow_type(const object_ptr<const Type>& tp)
    {
      if (get_if<ArrowType>(tp.value()))
        return true;
      else
        return false;
    };

    /// is_vartype
    [[nodiscard]] TORI_INLINE bool is_vartype(const object_ptr<const Type>& tp)
    {
      if (get_if<VarType>(tp.value()))
        return true;
      else
        return false;
    };

    /// has_value_type
    [[nodiscard]] TORI_INLINE bool has_value_type(const object_ptr<>& obj)
    {
      return is_value_type(get_type(obj));
    };

    /// has_arrow_type
    [[nodiscard]] TORI_INLINE bool has_arrow_type(const object_ptr<>& obj)
    {
      return is_arrow_type(get_type(obj));
    };

    /// has_vartype
    [[nodiscard]] TORI_INLINE bool has_vartype(const object_ptr<>& obj)
    {
      return is_vartype(get_type(obj));
    };

  } // namespace interface

  [[nodiscard]] TORI_INLINE object_ptr<const Type>
    copy_type_impl(const object_ptr<const Type>& ptp)
  {
    if (get_if<ValueType>(ptp.value()))
      return ptp;
    if (get_if<VarType>(ptp.value()))
      return ptp;
    if (auto arrow = get_if<ArrowType>(ptp.value())) {
      auto ret = new Type {ArrowType {copy_type_impl(arrow->captured),
                                      copy_type_impl(arrow->returns)}};
      return ret;
    }

    assert(false);
    unreachable();
  };

  namespace interface {

    /// Deep copy type object
    [[nodiscard]] TORI_INLINE object_ptr<const Type>
      copy_type(const object_ptr<const Type>& tp)
    {
      return copy_type_impl(tp);
    };
  } // namespace interface

  [[nodiscard]] TORI_INLINE bool same_type_impl(
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

    assert(false);
    unreachable();
  };

  namespace interface {

    /// check type equality
    [[nodiscard]] TORI_INLINE bool same_type(
      const object_ptr<const Type>& lhs,
      const object_ptr<const Type>& rhs)
    {
      return same_type_impl(lhs, rhs);
    }

    /// has_type
    template <class T>
    [[nodiscard]] bool has_type(const object_ptr<>& obj)
    {
      if (same_type(get_type(obj), object_type<T>()))
        return true;
      return false;
    };
  } // namespace interface

  struct TyArrow
  {
    object_ptr<const Type> from;
    object_ptr<const Type> to;
  };

  [[nodiscard]] TORI_INLINE object_ptr<const Type>
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
      return new Type {ArrowType {subst_type_impl(ta, arrow->captured),
                                  subst_type_impl(ta, arrow->returns)}};
    }

    assert(false);
    unreachable();
  };

  /// emulate type-substitution
  [[nodiscard]] TORI_INLINE object_ptr<const Type>
    subst_type(const TyArrow& ta, const object_ptr<const Type>& in)
  {
    return subst_type_impl(ta, in);
  };

  [[nodiscard]] TORI_INLINE object_ptr<const Type> subst_type_all(
    const std::vector<TyArrow>& tas,
    const object_ptr<const Type>& ty)
  {
    auto t = ty;
    for (auto tyArrow : tas) {
      t = subst_type(tyArrow, t);
    }
    return t;
  };

  // ------------------------------------------
  // Constr

  /// Type constraint
  struct Constr
  {
    object_ptr<const Type> t1;
    object_ptr<const Type> t2;
  };

  /// subst_constr
  [[nodiscard]] TORI_INLINE Constr
    subst_constr(const TyArrow& ta, const Constr& constr)
  {
    return {subst_type(ta, constr.t1), subst_type(ta, constr.t2)};
  };

  /// subst_constr_all
  [[nodiscard]] TORI_INLINE std::vector<Constr>
    subst_constr_all(const TyArrow& ta, const std::vector<Constr>& cs)
  {
    std::vector<Constr> ret;
    ret.reserve(cs.size());
    for (auto&& c : cs) ret.push_back(subst_constr(ta, c));
    return ret;
  };

  // ------------------------------------------
  // Occurs

  /// occurs
  [[nodiscard]] TORI_INLINE bool
    occurs(const object_ptr<const Type>& x, const object_ptr<const Type>& t)
  {
    if (get_if<ValueType>(t.value()))
      return false;
    if (get_if<VarType>(t.value()))
      return same_type(x, t);
    if (auto arrow = get_if<ArrowType>(t.value()))
      return occurs(x, arrow->captured) || occurs(x, arrow->returns);

    assert(false);
    unreachable();
  };

  TORI_INLINE void unify_func_impl(
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
  [[nodiscard]] TORI_INLINE std::vector<TyArrow>
    unify(const std::vector<Constr>& cs, const object_ptr<>& src)
  {
    auto _cs = cs;
    std::vector<TyArrow> as;
    unify_func_impl(_cs, as, src);
    return as;
  };

  // ------------------------------------------
  // Recon

  [[nodiscard]] TORI_INLINE object_ptr<const Type> genvar()
  {
    auto var = make_object<Type>(VarType {0});
    get_if<VarType>(var.value())->id = uintptr_t(var.get());
    return object_ptr<const Type>(var);
  };

} // namespace TORI_NS::detail