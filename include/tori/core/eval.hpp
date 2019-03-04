// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

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
      if (auto apply = value_cast_if<Apply>(obj)) {
        auto& apply_storage = _get_storage(*apply);
        // return cached value
        if (apply_storage.evaluated()) {
          return apply_storage.get_cache(apply.get()->spinlock);
        }
        // create new apply
        return make_object<Apply>(
          copy_apply_graph(apply_storage.app()),
          copy_apply_graph(apply_storage.arg()));
      }
      return obj;
    }

  } // namespace interface

  /// eval implementation
  [[nodiscard]] inline object_ptr<const Object>
    eval_impl(const object_ptr<const Object>& obj)
  {
    // apply
    if (auto apply = value_cast_if<Apply>(obj)) {
      // internal storage
      auto& apply_storage = _get_storage(*apply);
      // graph reduction
      if (apply_storage.evaluated()) {
        return apply_storage.get_cache(_get_storage(apply).get()->spinlock);
      }
      // whnf
      auto app = eval_impl(apply_storage.app());
      // detect exception
      if (has_exception_tag(app))
        throw result_error::exception_result(clear_pointer_tag(std::move(app)));
      // arg
      const auto& arg = apply_storage.arg();
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
      auto cc = static_cast<const Closure<>*>(pap.get());
      // push argument
      auto arity = --cc->arity();
      cc->arg(arity) = arg;
      // call code()
      if (arity == 0)
        pap = eval_impl(cc->code());
      // set cache
      apply_storage.set_cache(pap, apply.get()->spinlock);
      return pap;
    }
    // detect exception
    if (has_exception_tag(obj))
      throw result_error::exception_result(clear_pointer_tag(std::move(obj)));

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