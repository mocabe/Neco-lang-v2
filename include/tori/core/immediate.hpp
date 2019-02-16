// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "object_ptr_storage_fwd.hpp"

namespace TORI_NS::detail {

  namespace interface {

    /// specialized for immediate types
    template <class T>
    class immediate
    {
      friend class object_ptr_generic;

      static_assert(is_transfarable_immediate(type_c<T>));

    public:
      /// Ctor
      constexpr immediate()
        : m_storage {T {}}
      {
      }

      /// Ctor
      constexpr immediate(T v)
        : m_storage {v}
      {
      }

      /// Ctor
      constexpr immediate(const immediate& other)
        : m_storage {other.m_storage}
      {
      }

      /// operator=
      constexpr immediate& operator=(const immediate& other)
      {
        m_storage = other.m_storage;
        return *this;
      }

      /// access value
      const auto& value() const
      {
        assert(m_storage.is_immediate());
        return get<T>(m_storage.immediate_union());
      }

      /// access value
      auto& value()
      {
        assert(m_storage.is_immediate());
        return get<T>(m_storage.immediate_union());
      }

      /// operator*
      const auto& operator*() const
      {
        return value();
      }

      /// operator*
      auto& operator*()
      {
        return value();
      }

    private:
      object_ptr_storage m_storage;
    };

  } // namespace interface

}