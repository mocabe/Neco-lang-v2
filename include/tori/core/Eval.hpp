#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file eval

#include "Function.hpp"
#include "Fix.hpp"
#include "Exception.hpp"
#include "ValueCast.hpp"

namespace TORI_NS::detail {

  namespace interface {

    namespace eval_error {

      /// bad appli for fix
      class bad_fix : public eval_error
      {
      public:
        bad_fix(const char* msg, const object_ptr<>& obj)
          : eval_error(msg, obj)
        {
        }
      };

      /// bad apply (apply for value)
      class bad_apply : public eval_error
      {
      public:
        bad_apply(const char* msg, const object_ptr<>& obj)
          : eval_error(msg, obj)
        {
        }
      };

      /// too many arguments
      class too_many_arguments : public eval_error
      {
      public:
        too_many_arguments(const char* msg, const object_ptr<>& obj)
          : eval_error(msg, obj)
        {
        }
      };

    } // namespace eval_error

    /// copy apply graph
    [[nodiscard]] TORI_INLINE object_ptr<>
      copy_apply_graph(const object_ptr<>& obj)
    {
      if (auto apply = value_cast_if<ApplyR>(obj)) {
        // return cached value
        if (apply->evaluated()) {
          return apply->get_cache();
        }
        // create new apply
        return new ApplyR {copy_apply_graph(apply->app()),
                           copy_apply_graph(apply->arg())};
      }
      return obj;
    }

  } // namespace interface

  /// eval implementation
  [[nodiscard]] TORI_INLINE object_ptr<> eval_impl(const object_ptr<>& obj)
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
        throw result_error::result_error(exception);
      }
      const auto& arg = apply->arg();
      // Fix
      if (has_type<Fix>(app)) {
        auto f = eval_impl(arg);
        // detect exception
        if (auto exception = value_cast_if<Exception>(f)) {
          throw result_error::result_error(exception);
        }
        // check arg
        if (unlikely(has_value_type(f))) {
          throw eval_error::bad_fix(
            "eval_error: Expected closure after Fix", obj);
        }
        // cast to closure
        auto c = static_cast<Closure<>*>(f.get());
        // check arity
        if (unlikely(c->arity.load() == 0)) {
          throw eval_error::bad_fix(
            "eval_error: Expected appliable closure after Fix", obj);
        }
        // process
        auto pap = f.clone();
        auto cc = static_cast<Closure<>*>(pap.get());
        auto arity = cc->arity.fetch_sub() - 1;
        cc->args(arity) = obj;
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
        throw eval_error::bad_apply("eval_error: Apply to value type", obj);
      }
      // too many arguments
      auto c = static_cast<Closure<>*>(app.get());
      if (unlikely(c->arity.load() == 0)) {
        throw eval_error::too_many_arguments(
          "eval_error: Too many arguments", obj);
      }
      // create pap
      auto pap = app.clone();
      // process
      auto cc = static_cast<Closure<>*>(pap.get());
      auto arity = cc->arity.fetch_sub() - 1;
      cc->args(arity) = arg;
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
      throw result_error::result_error(exception);
    }

    return obj;
  }

  namespace interface {

    /// evaluate each apply node and replace with result
    template <class T>
    [[nodiscard]] auto eval(const object_ptr<T>& obj)
    {
      auto result = eval_impl(object_ptr<>(obj));
      if constexpr (!is_error_type_v<type_of_t<typename T::term, false>>) {
        // Currently object_ptr<T> MUST have type T which has compatible memory
        // layout with actual object pointing to.
        // Since it's impossible to decide memory layout of closure types,
        // we convert it to closure<...> which is essentially equal to to
        // HeapObject. Type variables are also undecidable so we just convert
        // them to HeapObject.
        using To = assume_object_type_t<type_of_t<typename T::term>>;
        result.head()->refcount.fetch_add(); // +1
        return object_ptr<To>(static_cast<To*>(result.get()));
      } else {
        return result;
      }
    }

  } // namespace interface

} // namespace TORI_NS::detail