#pragma once

#include "exception.hpp"

namespace TORI_NS::detail {

  namespace interface {

    // ------------------------------------------
    // Exceptions

    namespace type_error {

      /// unification error(circular constraint)
      class circular_constraint : public type_error
      {
      public:
        circular_constraint(object_ptr<> src, object_ptr<const Type> var)
          : type_error("Circular constraints", std::move(src))
          , m_var {std::move(var)}
        {
        }

        /// var
        const object_ptr<const Type>& var() const
        {
          return m_var;
        }

      private:
        object_ptr<const Type> m_var;
      };

      /// unification error(missmatch)
      class type_missmatch : public type_error
      {
      public:
        type_missmatch(
          object_ptr<> src,
          object_ptr<const Type> t1,
          object_ptr<const Type> t2)
          : type_error("Type missmatch", std::move(src))
          , m_t1 {std::move(t1)}
          , m_t2 {std::move(t2)}
        {
        }

        /// t1
        const object_ptr<const Type>& t1() const
        {
          return m_t1;
        }

        /// t2
        const object_ptr<const Type>& t2() const
        {
          return m_t2;
        }

      private:
        /// t1
        object_ptr<const Type> m_t1;
        /// t2
        object_ptr<const Type> m_t2;
      };

      /// bad type check
      class bad_type_check : public type_error
      {
      public:
        bad_type_check(
          object_ptr<const Type> expected,
          object_ptr<const Type> result,
          object_ptr<> obj)
          : type_error(
              "type_error: check_type failed. Result type is invalid",
              std::move(obj))
          , m_expected {std::move(expected)}
          , m_result {std::move(result)}
        {
        }

        /// expected
        const object_ptr<const Type>& expected() const
        {
          return m_expected;
        }

        /// result
        const object_ptr<const Type>& result() const
        {
          return m_result;
        }

      private:
        /// expected type
        object_ptr<const Type> m_expected;
        /// result type
        object_ptr<const Type> m_result;
      };

    } // namespace type_error

  } // namespace interface

} // namespace TORI_NS::detail
