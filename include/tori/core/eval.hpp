// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
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
          return apply_storage.get_cache();
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
    // detect exception
    if (TORI_UNLIKELY(has_exception_tag(obj)))
      throw result_error::exception_result(clear_pointer_tag(obj));

    // apply
    if (auto apply = value_cast_if<Apply>(obj)) {

      // alias: internal storage
      auto& apply_storage = _get_storage(*apply);

      // graph reduction
      if (apply_storage.evaluated()) {
        return apply_storage.get_cache();
      }

      // whnf
      auto app = eval_impl(apply_storage.app());

      // alias: argument
      const auto& arg = apply_storage.arg();

      // check app
      if (TORI_UNLIKELY(has_value_type(app))) {
        throw eval_error::bad_apply();
      }

      // too many arguments
      auto capp = static_cast<const Closure<>*>(app.get());
      if (TORI_UNLIKELY(capp->arity() == 0)) {
        throw eval_error::too_many_arguments();
      }

      // clone closure and apply
      auto ret = [&] {
        auto pap = clone(app);
        auto cpap = static_cast<const Closure<>*>(pap.get());

        // push argument
        auto arity = --cpap->arity();
        cpap->arg(arity) = arg;

        // call code()
        if (TORI_UNLIKELY(arity == 0)) {
          return eval_impl(cpap->code());
        }

        return pap;
      }();

      // set cache
      apply_storage.set_cache(ret);

      return ret;
    }

    return obj;
  }

  namespace interface {

    /// evaluate each apply node and replace with result
    template <class T>
    [[nodiscard]] auto eval(object_ptr<T> obj)
    {
      auto result = eval_impl(std::move(obj));
      TORI_ASSERT(result);

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