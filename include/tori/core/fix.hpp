// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#if !defined(TORI_NO_LOCAL_INCLUDE)
#  include "../config/config.hpp"
#  include "box.hpp"
#  include "type_gen.hpp"
#  include "function.hpp"
#  include "eval.hpp"
#endif

namespace TORI_NS::detail {

  class Fix_X;

  namespace interface {

    // fix: (a -> a) -> a
    // fix f = let x = f x in x
    struct Fix : Function<Fix, closure<Fix_X, Fix_X>, Fix_X>
    {
      return_type code() const
      {
        // reduce argument closure
        object_ptr<const Object> f = eval_arg<0>();

        // check arity for safety
        auto c = static_cast<const Closure<>*>(_get_storage(f).get());
        if (TORI_UNLIKELY(c->arity() == 0)) {
          throw eval_error::bad_fix();
        }

        auto ret = [&] {
          // create return value
          auto pap = clone(f);
          auto cc = static_cast<const Closure<>*>(_get_storage(pap).get());

          // build self-referencing closure
          auto arity = --cc->arity();
          cc->arg(arity) = pap;

          // avoid memory leak
          cc->refcount.fetch_sub();

          // eval
          if (TORI_UNLIKELY(arity == 0))
            return eval_impl(cc->code());

          return pap;
        }();

        // return
        return static_object_cast<const VarValueProxy<Fix_X>>(std::move(ret));
      }
    };

  } // namespace interface

} // namespace TORI_NS::detail