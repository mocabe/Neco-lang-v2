// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file Thunk

#include "../core.hpp"

namespace TORI_NS::detail {

  namespace interface {
    /// Thunk
    struct Thunk : Function<Thunk, Auto<class Thunk_X>, Auto<class Thunk_X>> {
      ReturnType code() {
        if (m_evaluated) {
          return m_obj;
        } else {
          auto r = eval_arg<0>();
          m_obj = r;
          m_evaluated = true;
          return r;
        }
      }

    private:
      ObjectPtr<Auto<Thunk_X>> m_obj;
      bool_t m_evaluated;
    };
  } // namespace interface
} // namespace TORI_NS::detail