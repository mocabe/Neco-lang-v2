// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "exception.hpp"

namespace TORI_NS::detail {

  // ------------------------------------------
  // Result errors

  namespace interface {

    namespace result_error {

      /// result error
      class result_error : public std::runtime_error
      {
      public:
        result_error(object_ptr<Exception> result)
          : runtime_error("result_error: Exception detected while evaluation")
          , m_result {std::move(result)}
        {
        }

        /// result
        const object_ptr<Exception>& result() const
        {
          return m_result;
        }

      private:
        object_ptr<Exception> m_result;
      };

    } // namespace result_error

  } // namespace interface

}