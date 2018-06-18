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
  ObjectPtr<> _eval_rec(const ObjectPtr<>& obj) {
    // closure
    if (has_arrow_type(obj)) {
      auto c = static_cast<Closure<>*>(obj.head());
      if (c->arity.atomic == 0) {
        auto eval_result = c->code();
        return _eval_rec(eval_result);
      }
    }
    // thunk
    if (auto thunk = value_cast_if<Thunk>(obj)) {
      if (thunk->evaluated) {
        return thunk->value;
      } else {
        thunk->value = _eval_rec(thunk->value);
        thunk->evaluated = true;
        return thunk->value;
      }
    }
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
            cc->args(--cc->arity.atomic) = make_object<Thunk>(obj);
            return _eval_rec(ObjectPtr<>(pap));
          } else {
            c->args(--c->arity.atomic) = make_object<Thunk>(obj);
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
          cc->args(--cc->arity.atomic) = make_object<Thunk>(std::move(arg));
          return _eval_rec(ObjectPtr<>(pap));
        } else {
          // partial apply
          c->args(--c->arity.atomic) = make_object<Thunk>(std::move(arg));
          return _eval_rec(ObjectPtr<>(app));
        }
      }
    }
    return obj;
  }

  namespace interface {
    /// evaluation
    template <class TObjectPtr>
    ObjectPtr<> eval(TObjectPtr&& obj) {
      ObjectPtr<> _obj(std::forward<TObjectPtr>(obj));
      return detail::_eval_rec(_obj);
    }

    /// evaluation
    template <class T>
    ObjectPtr<T> eval(const Expected<T>& e) {
      auto r = eval(e.obj);
      ++r->refcount.atomic;
      return static_cast<T*>(r.head());
    }

    /// evaluation
    template <class T>
    ObjectPtr<T> eval(Expected<T>&& e) {
      auto r = eval(std::move(e.obj));
      ++r->refcount.atomic;
      return static_cast<T*>(r.head());
    }
  } // namespace interface

} // namespace TORI_NS::detail