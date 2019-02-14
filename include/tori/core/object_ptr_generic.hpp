// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "object_ptr.hpp"
#include "immediate.hpp"

namespace TORI_NS::detail {

  namespace interface {

    /// type-erased object_ptr.
    /// contains either a pointer-to-object or immediate(unboxed) value.
    class object_ptr_generic
    {
      template <class T>
      friend auto value_cast(const object_ptr_generic&);
      template <class T>
      friend auto value_cast(object_ptr_generic&&);
      template <class T, class>
      friend object_ptr<T> value_cast_if(const object_ptr_generic&);
      template <class T, class>
      friend object_ptr<T> value_cast_if(object_ptr_generic&&);

    public:
      /// default ctor
      constexpr object_ptr_generic() noexcept
        : m_storage {nullptr}
      {
      }

      /// nullptr ctor
      constexpr object_ptr_generic(nullptr_t) noexcept
        : m_storage {nullptr}
      {
      }

      /// ctor for immediate
      template <class T>
      constexpr object_ptr_generic(T v) noexcept
        : m_storage {v}
      {
        assert(m_storage.is_immediate());
      }

      /// ctor for pointer
      template <class T>
      constexpr object_ptr_generic(T* ptr) noexcept
        : m_storage {ptr}
      {
        assert(m_storage.is_pointer());
      }

      /// copy ctor
      object_ptr_generic(const object_ptr_generic& other)
        : m_storage {other.m_storage}
      {
        increment_refcount_when_pointer();
      }

      /// move ctor
      object_ptr_generic(object_ptr_generic&& other)
        : m_storage {other.m_storage}
      {
        // take over ownership if pointer
        if (m_storage.is_pointer())
          other.m_storage = {nullptr};
      }

      /// convert ctor (pointer)
      template <class T>
      object_ptr_generic(const object_ptr<T>& ptr)
        : m_storage {ptr.m_storage}
      {
        assert(m_storage.is_pointer());
        if (m_storage.ptr() && !is_static())
          m_storage.ptr()->refcount.fetch_add();
      }

      /// convert ctor (immediate)
      template <class T>
      object_ptr_generic(const immediate<T>& imm)
        : m_storage {imm.m_storage}
      {
        assert(m_storage.is_immediate());
      }

      /// convert move ctor (pointer)
      template <class T>
      object_ptr_generic(object_ptr<T>&& ptr)
        : m_storage {ptr.m_storage}
      {
        assert(m_storage.is_pointer());
        ptr.m_storage = {nullptr};
      }

      /// convert move ctor (immediate)
      template <class T>
      object_ptr_generic(immediate<T>&& imm)
        : m_storage {imm.m_storage}
      {
        assert(m_storage.is_immediate());
        *imm = {};
      }

      object_ptr_generic& operator=(const object_ptr_generic& other)
      {
        object_ptr_generic(other).swap(*this);
        return *this;
      }

      object_ptr_generic& operator=(object_ptr_generic&& other)
      {
        object_ptr_generic(std::move(other)).swap(*this);
        return *this;
      }

      ~object_ptr_generic() noexcept
      {
        decrement_refcount_when_pointer();
      }

      bool is_pointer() const noexcept
      {
        return m_storage.is_pointer();
      }

      bool is_immediate() const noexcept
      {
        return m_storage.is_immediate();
      }

      explicit operator bool() const
      {
        return static_cast<bool>(m_storage);
      }

      void swap(object_ptr_generic& other) noexcept
      {
        std::swap(m_storage, other.m_storage);
      }

    private:
      bool is_static() const
      {
        assert(m_storage.is_pointer());
        return m_storage.ptr()->refcount.load() == 0;
      }

      /// increment refcount
      void increment_refcount_when_pointer()
      {
        if (m_storage.is_pointer() && m_storage.ptr() && !is_static())
          m_storage.ptr()->refcount.fetch_add();
      }

      /// decrement refcount and free object when needed
      void decrement_refcount_when_pointer()
      {
        if (m_storage.is_pointer() && m_storage.ptr() && !is_static()) {
          if (m_storage.ptr()->refcount.fetch_sub() == 1) {
            std::atomic_thread_fence(std::memory_order_acquire);
            m_storage.ptr()->info_table->destroy(m_storage.ptr());
          }
        }
      }

    private:
      object_ptr_storage m_storage;
    };

  } // namespace interface

} // namespace TORI_NS::detail