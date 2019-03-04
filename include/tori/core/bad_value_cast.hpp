// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "exception.hpp"

namespace TORI_NS::detail {

  namespace interface {

    /// bad_value_cast exception
    class bad_value_cast : public std::logic_error
    {
    public:
      explicit bad_value_cast(
        object_ptr<const Type> from,
        object_ptr<const Type> to)
        : std::logic_error("bad_value_cast")
        , m_from {std::move(from)}
        , m_to {std::move(to)}
      {
      }

      /// get from
      const object_ptr<const Type>& from() const
      {
        return m_from;
      }

      /// get to
      const object_ptr<const Type>& to() const
      {
        return m_to;
      }

    private:
      /// cast from
      object_ptr<const Type> m_from;
      /// cast to
      object_ptr<const Type> m_to;
    };

  } // namespace interface

  struct bad_value_object_value
  {
    /// cast from
    object_ptr<const Type> from;
    /// cast to
    object_ptr<const Type> to;
  };

  namespace interface {

    /// Exception object for bad_value_cast exception
    using BadValueCast = Box<bad_value_object_value>;

  } // namespace interface

  // ------------------------------------------
  // conversion

  [[nodiscard]] inline object_ptr<Exception>
    to_Exception(const bad_value_cast& e)
  {
    return make_object<Exception>(
      e.what(), make_object<BadValueCast>(e.from(), e.to()));
  }

} // namespace TORI_NS::detail