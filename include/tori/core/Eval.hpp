// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file eval

#include "Function.hpp"
#include "Fix.hpp"
#include "Exception.hpp"
#include "ValueCast.hpp"
#include "Thunk.hpp"

namespace TORI_NS::detail {

  /// eval implementation
  [[nodiscard]] ObjectPtr<> _eval_rec(const ObjectPtr<>& obj) {
    // closure
    if (has_arrow_type(obj)) {
      auto c = static_cast<Closure<>*>(obj.head());
      if (c->arity.atomic == 0) {
        auto eval_result = c->code();
        return _eval_rec(eval_result);
      }
    }
    // thunk
    if (auto thunk = value_cast_if<ThunkR>(obj))
      return _eval_rec(thunk->code());
    // exception
    if (auto exception = value_cast_if<Exception>(obj))
      throw result_error(exception);
    // apply
    if (auto apply = value_cast_if<ApplyR>(obj)) {
      // reduce app
      auto app = _eval_rec(apply->app);
      auto arg = apply->arg;
      // Fix
      if (has_type<Fix>(app)) {
        auto f = _eval_rec(arg);
        if (has_value_type(f)) {
          throw eval_error{"Fix: Expected Closure", obj};
        } else {
          auto c = static_cast<Closure<>*>(f.head());
          if (c->arity.atomic == 0) {
            throw eval_error{"Fix: Expected appliable closure", obj};
          } else if (c->arity.atomic == c->n_args()) {
            auto pap = f.clone();
            auto cc = static_cast<Closure<>*>(pap.head());
            cc->args(--cc->arity.atomic) = obj;
            return _eval_rec(ObjectPtr<>(pap));
          } else {
            c->args(--c->arity.atomic) = obj;
            return _eval_rec(ObjectPtr<>(f));
          }
        }
      }
      // Apply
      if (has_value_type(app)) {
        throw eval_error{"Apply: Cannot apply to value", obj};
      } else {
        // create pap
        auto c = static_cast<Closure<>*>(app.head());
        if (c->arity.atomic == 0) {
          // arguments are full
          throw eval_error{"Apply: Too many arguments", obj};
        } else if (c->arity.atomic == c->n_args()) {
          // first apply
          auto pap = app.clone();
          auto cc = static_cast<Closure<>*>(pap.head());
          cc->args(--cc->arity.atomic) = std::move(arg);
          return _eval_rec(ObjectPtr<>(pap));
        } else {
          // partial apply
          c->args(--c->arity.atomic) = std::move(arg);
          return _eval_rec(ObjectPtr<>(app));
        }
      }
    }
    return obj;
  }

  namespace interface {
    template <class T>
    [[nodiscard]] auto eval(const ObjectPtr<T>& obj)
      -> ObjectPtr<assume_object_type_t<type_of_t<typename T::term>>> {
      using To = assume_object_type_t<type_of_t<typename T::term>>;
      auto result = _eval_rec(ObjectPtr<>(obj));
      ++(result.head()->refcount.atomic);
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
      return ObjectPtr<To>(static_cast<To*>(result.head()));
    }
  } // namespace interface

} // namespace TORI_NS::detail