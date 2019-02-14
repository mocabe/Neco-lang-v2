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

    public:
      constexpr immediate()
        : m_storage {T {}}
      {
      }

      constexpr immediate(T v)
        : m_storage {v}
      {
      }

      constexpr immediate(const immediate& other)
        : m_storage {other.m_storage}
      {
      }

      constexpr immediate& operator=(const immediate& other)
      {
        m_storage = other.m_storage;
        return *this;
      }

      const auto& value() const
      {
        assert(m_storage.is_immediate());
        return get<T>(m_storage.immediate_union());
      }

      auto& value()
      {
        assert(m_storage.is_immediate());
        return get<T>(m_storage.immediate_union());
      }

      const auto& operator*() const
      {
        return value();
      }

      auto& operator*()
      {
        return value();
      }

      explicit operator bool() const
      {
        return static_cast<bool>(value());
      }

    private:
      object_ptr_storage m_storage;
    };

  } // namespace interface

}