// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "fix.hpp"
#include "value_cast.hpp"
#include "function.hpp"
#include "eval_error.hpp"
#include "result_error.hpp"

namespace TORI_NS::detail {

  namespace interface {

    /// copy apply graph
    [[nodiscard]] inline object_ptr<const Object>
      copy_apply_graph(const object_ptr<const Object>& obj)
    {
      if (auto apply = value_cast_if<ApplyR>(obj)) {
        // return cached value
        if (apply->evaluated()) {
          return apply->get_cache();
        }
        // create new apply
        return new ApplyR(
          copy_apply_graph(apply->app()), copy_apply_graph(apply->arg()));
      }
      return obj;
    }

  } // namespace interface

  /// eval implementation
  [[nodiscard]] inline object_ptr<const Object>
    eval_impl(const object_ptr<const Object>& obj)
  {
    // apply
    if (auto apply = value_cast_if<ApplyR>(obj)) {
      // graph reduction
      if (apply->evaluated()) {
        return apply->get_cache();
      }
      // reduce app
      auto app = eval_impl(apply->app());
      // detect exception
      if (auto exception = value_cast_if<Exception>(app)) {
        throw result_error::exception_result(std::move(exception));
      }
      const auto& arg = apply->arg();
      // Fix
      if (has_type<Fix>(app)) {
        auto f = eval_impl(arg);
        // detect exception
        if (auto exception = value_cast_if<Exception>(f)) {
          throw result_error::exception_result(std::move(exception));
        }
        // check arg
        if (unlikely(has_value_type(f))) {
          throw eval_error::bad_fix();
        }
        // cast to closure
        auto c = static_cast<const Closure<>*>(f.get());
        // check arity
        if (unlikely(c->arity() == 0)) {
          throw eval_error::bad_fix();
        }
        // process
        auto pap = clone(f);
        auto cc = static_cast<const Closure<>*>(pap.get());
        auto arity = --cc->arity();
        cc->arg(arity) = obj;
        if (arity == 0) {
          auto eval_result = eval_impl(cc->code());
          apply->set_cache(eval_result);
          return eval_result;
        } else {
          return eval_impl(pap);
        }
      }
      // check app
      if (unlikely(has_value_type(app))) {
        throw eval_error::bad_apply();
      }
      // too many arguments
      auto c = static_cast<const Closure<>*>(app.get());
      if (unlikely(c->arity() == 0)) {
        throw eval_error::too_many_arguments();
      }
      // create pap
      auto pap = clone(app);
      // process
      auto cc = static_cast<const Closure<>*>(pap.get());
      auto arity = --cc->arity();
      cc->arg(arity) = arg;
      if (arity == 0) {
        auto eval_result = eval_impl(cc->code());
        apply->set_cache(eval_result);
        return eval_result;
      } else {
        apply->set_cache(pap);
        return eval_impl(pap);
      }
    }
    // detect exception
    if (auto exception = value_cast_if<Exception>(obj)) {
      throw result_error::exception_result(std::move(exception));
    }

    return obj;
  }

  namespace interface {

    /// evaluate each apply node and replace with result
    template <class T>
    [[nodiscard]] auto eval(const object_ptr<T>& obj)
    {
      auto result = eval_impl(object_ptr<const Object>(obj));
      assert(result);

      // for gcc 7
      constexpr auto type = type_of(get_term<T>(), false_c);

      // run compile time type check
      if constexpr (!is_error_type(type)) {
        // Currently object_ptr<T> MUST have type T which has compatible memory
        // layout with actual object pointing to.
        // Since it's impossible to decide memory layout of closure types,
        // we convert it to closure<...> which is essentially equal to to
        // Object. Type variables are also undecidable so we just convert
        // them to Object.
        using To =
          std::add_const_t<typename decltype(guess_object_type(type))::type>;
        // cast to resutn type
        return static_object_cast<To>(result);
      } else {
        // fallback to object_ptr<>
        return result;
      }
    }

  } // namespace interface

} // namespace TORI_NS::detail