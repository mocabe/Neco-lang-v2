// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file eval

#include "Function.hpp"
#include "Fix.hpp"
#include "Exception.hpp"
#include "ValueCast.hpp"

namespace TORI_NS::detail {

  namespace interface {
    /// copy apply graph
    [[nodiscard]] ObjectPtr<> copy_apply_graph(const ObjectPtr<>& obj) {
      if (auto apply = value_cast_if<ApplyR>(obj)) {
        // return cached value
        if (apply->evaluated()) return apply->get_cache();
        // create new apply
        return new ApplyR{copy_apply_graph(apply->arg()),
                          copy_apply_graph(apply->app())};
      }
      return obj;
    }
  }

  /// eval implementation
  [[nodiscard]] ObjectPtr<> eval_impl(const ObjectPtr<>& obj) {
    // exception
    if (auto exception = value_cast_if<Exception>(obj))
      throw result_error(exception);
    // apply
    if (auto apply = value_cast_if<ApplyR>(obj)) {
      // graph reduction
      if (apply->evaluated()) return apply->get_cache();
      // reduce app
      auto app = eval_impl(apply->app());
      auto& arg = apply->arg();
      // Fix
      if (has_type<Fix>(app)) {
        auto f = eval_impl(arg);
        if (has_value_type(f)) {
          throw eval_error{"Fix: Expected Closure", obj};
        } else {
          auto c = static_cast<Closure<>*>(f.get());
          if (c->arity.atomic == 0) {
            throw eval_error{"Fix: Expected appliable closure", obj};
          } else {
            auto pap = f.clone();
            auto cc = static_cast<Closure<>*>(pap.get());
            cc->args(--cc->arity.atomic) = obj;
            return eval_impl(ObjectPtr<>(pap));
          }
        }
      }
      // Apply
      if (has_value_type(app)) {
        throw eval_error{"Apply: Cannot apply to value", obj};
      } else {
        // create pap
        auto c = static_cast<Closure<>*>(app.get());
        if (c->arity.atomic == 0) {
          throw eval_error{"Apply: Too many arguments", obj};
        } else {
          auto pap = app.clone();
          auto cc = static_cast<Closure<>*>(pap.get());
          cc->args(--cc->arity.atomic) = arg;
          if (cc->arity.atomic == 0) {
            auto eval_result = eval_impl(cc->code());
            apply->set_cache(eval_result);
            return eval_result;
          } else {
            apply->set_cache(pap);
            return eval_impl(ObjectPtr<>(pap));
          }
        }
      }
    }
    return obj;
  }

  namespace interface {

    /// evaluate each apply node and replace with result
    template <class T>
    [[nodiscard]] auto eval(const ObjectPtr<T>& obj)
      ->ObjectPtr<assume_object_type_t<type_of_t<typename T::term>>> {
      using To = assume_object_type_t<type_of_t<typename T::term>>;
      auto result = eval_impl(ObjectPtr<>(obj));

      // This conversion is not obvious.
      // Currently ObjectPtr<T> MUST have type T which has compatible memory
      // layout with ACTUAL object pointing to.
      // Since it's impossible to decide memory layout of closure types (because
      // of runtime currying), we convert it to closure<...> which is
      // essentially equal to to HeapObject.
      // Type variables are also undecidable so we just convert them to
      // HeapObject.
      // Finally, we get type To from compile time checker. So when compile time
      // type system is broken or bugged, this conversion will crash the
      // program without throwing any error.
      ++(result.head()->refcount.atomic);
      return ObjectPtr<To>(static_cast<To*>(result.get()));
    }

  } // namespace interface

} // namespace TORI_NS::detail