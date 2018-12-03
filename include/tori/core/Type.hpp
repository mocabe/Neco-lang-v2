#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file type_of

#include "Function.hpp"
#include "Fix.hpp"
#include "Exception.hpp"
#include "ValueCast.hpp"

namespace TORI_NS::detail {

  TORI_INLINE void vars_impl(
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
  };

  // get list of type variables
  [[nodiscard]] TORI_INLINE std::vector<object_ptr<const Type>>
    vars(const object_ptr<const Type>& tp)
  {
    std::vector<object_ptr<const Type>> vars;
    vars_impl(tp, vars);
    return vars;
  };

  /// create fresh polymorphic closure type
  [[nodiscard]] TORI_INLINE object_ptr<const Type>
    genpoly(const object_ptr<const Type>& tp)
  {
    if (!is_arrow_type(tp))
      return tp;
    auto vs = vars(tp);
    auto t = tp;
    for (auto v : vs) {
      TyArrow a {v, genvar()};
      t = subst_type(a, tp);
    }
    return t;
  };

  // typing
  [[nodiscard]] TORI_INLINE const object_ptr<const Type>
    type_of_func_impl(const object_ptr<>& obj)
  {
    // Apply
    if (auto apply = value_cast_if<ApplyR>(obj)) {
      if (auto fix = value_cast_if<Fix>(apply->app())) {
        auto _t1 = type_of_func_impl(apply->arg());
        auto _t = genvar();
        auto c = std::vector {Constr {_t1, new Type(ArrowType {_t, _t})}};
        auto s = unify(std::move(c), obj);
        return subst_type_all(s, _t);
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
  };

  namespace interface {
    // type_of
    [[nodiscard]] TORI_INLINE object_ptr<const Type>
      type_of(const object_ptr<>& obj)
    {
      return type_of_func_impl(obj);
    }
  } // namespace interface

  namespace interface {
    namespace type_error {
      /// bad type check
      class bad_type_check : public type_error {
      public:
        bad_type_check(
          const object_ptr<const Type>& expected,
          const object_ptr<const Type>& result,
          const object_ptr<>& obj)
          : type_error(
              "type_error: check_type failed. Result type is invalid",
              obj)
          , m_expected {expected}
          , m_result {result}
        {}

        /// expected
        const object_ptr<const Type>& expected() const
        {
          return m_expected;
        }

        /// result
        const object_ptr<const Type>& result() const
        {
          return m_result;
        }

      private:
        object_ptr<const Type> m_expected;
        object_ptr<const Type> m_result;
      };
    } // namespace type_error

    /// check type
    template <class T>
    void check_type(const object_ptr<>& obj)
    {
      auto t1 = object_type<T>();
      auto t2 = type_of(obj);
      if (unlikely(!same_type(t1, t2)))
        throw type_error::bad_type_check(t1, t2, obj);
    }
  } // namespace interface

} // namespace TORI_NS::detail