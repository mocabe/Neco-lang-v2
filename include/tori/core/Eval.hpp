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
    /// copy apply graph
    [[nodiscard]] TORI_INLINE object_ptr<> copy_apply_graph(
      const object_ptr<>& obj) {
      if (auto apply = value_cast_if<ApplyR>(obj)) {
        // return cached value
        if (apply->evaluated()) return apply->get_cache();
        // create new apply
        return new ApplyR{copy_apply_graph(apply->arg()),
                          copy_apply_graph(apply->app())};
      }
      return obj;
    }
  } // namespace interface

  namespace interface {
    struct eval_error::invalid_fix : eval_error {
      invalid_fix(const char* msg, const object_ptr<>& obj)
        : eval_error(msg, obj) {}
    };
    struct eval_error::invalid_apply : eval_error {
      invalid_apply(const char* msg, const object_ptr<>& obj)
        : eval_error(msg, obj) {}
    };
    struct eval_error::too_many_arguments : eval_error {
      too_many_arguments(const char* msg, const object_ptr<>& obj)
        : eval_error(msg, obj) {}
    };
  } // namespace interface

  /// eval implementation
  [[nodiscard]] TORI_INLINE object_ptr<> eval_impl(const object_ptr<>& obj) {
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
        if (unlikely(has_value_type(f))) {
          throw eval_error::invalid_fix(
            "eval_error: Expected closure after Fix", obj);
        } else {
          auto c = static_cast<Closure<>*>(f.get());
          if (unlikely(c->arity.atomic == 0)) {
            throw eval_error::invalid_fix(
              "eval_error: Expected appliable closure after Fix", obj);
          } else {
            auto pap = f.clone();
            auto cc = static_cast<Closure<>*>(pap.get());
            cc->args(--cc->arity.atomic) = obj;
            return eval_impl(object_ptr<>(pap));
          }
        }
      }
      // Apply
      if (unlikely(has_value_type(app))) {
        throw eval_error::invalid_apply("eval_error: Apply to value type", obj);
      } else {
        // create pap
        auto c = static_cast<Closure<>*>(app.get());
        if (unlikely(c->arity.atomic == 0)) {
          throw eval_error::too_many_arguments(
            "eval_error: Too many arguments", obj);
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
            return eval_impl(object_ptr<>(pap));
          }
        }
      }
    }
    return obj;
  }

  namespace interface {

    /// evaluate each apply node and replace with result
    template <class T>
    [[nodiscard]] auto eval(const object_ptr<T>& obj)
      ->object_ptr<assume_object_type_t<type_of_t<typename T::term>>> {
      using To = assume_object_type_t<type_of_t<typename T::term>>;
      auto result = eval_impl(object_ptr<>(obj));

      ++(result.head()->refcount.atomic);

      // This conversion is not obvious.
      // Currently object_ptr<T> MUST have type T which has compatible memory
      // layout with ACTUAL object pointing to.
      // Since it's impossible to decide memory layout of closure types (because
      // of runtime currying), we convert it to closure<...> which is
      // essentially equal to to HeapObject.
      // Type variables are also undecidable so we just convert them to
      // HeapObject.
      // Finally, we get type To from compile time checker. So when compile time
      // type system is broken or bugged, this conversion will crash the
      // program without throwing any error.
      return object_ptr<To>(static_cast<To*>(result.get()));
    }

  } // namespace interface

} // namespace TORI_NS::detail