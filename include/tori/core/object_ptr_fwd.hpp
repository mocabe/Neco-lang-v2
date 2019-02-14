// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "object_ptr_storage_fwd.hpp"

namespace TORI_NS::detail {

  namespace interface {

    /// Smart pointer to manage heap-allocated objects
    /// Holds a (possibly tagged) pointer to an object in storage member.
    template <class T = Object>
    class object_ptr
    {
      template <class>
      friend class object_ptr;
      friend class object_ptr_generic;

    public:
      // value type
      using element_type = T;
      // pointer
      using pointer = T*;

      /// Constructor
      constexpr object_ptr() noexcept
        : m_storage {nullptr}
      {
      }

      /// Constructor
      constexpr object_ptr(nullptr_t) noexcept
        : m_storage {nullptr}
      {
      }

      /// Pointer constructor
      object_ptr(pointer p) noexcept
        : m_storage {p}
      {
      }

      /// Copy constructor
      /// \effects increases reference count.
      object_ptr(const object_ptr<element_type>& other) noexcept
        : m_storage {other.m_storage}
      {
        if (likely(m_storage.ptr() && !is_static()))
          head()->refcount.fetch_add();
      }

      /// Move constructor
      object_ptr(object_ptr<element_type>&& other) noexcept
        : m_storage {other.m_storage}
      {
        other.m_storage = {nullptr};
      }

      /// Copy convert constructor
      /// \effects increases reference count.
      template <class U>
      object_ptr(const object_ptr<U>& other) noexcept
        : m_storage {other.m_storage}
      {
        if (likely(m_storage.ptr() && !is_static()))
          head()->refcount.fetch_add();
      }

      /// Move convert constructor
      template <class U>
      object_ptr(object_ptr<U>&& other) noexcept
        : m_storage {other.m_storage}
      {
        other.m_storage = {nullptr};
      }

      /// get address of object
      pointer get() const noexcept
      {
        return static_cast<pointer>(m_storage.ptr());
      }

      /// get address of header
      auto* head() const noexcept
      {
        if constexpr (std::is_const_v<element_type>)
          return static_cast<const Object*>(m_storage.ptr());
        else
          return static_cast<Object*>(m_storage.ptr());
      }

      /// get address of info table
      /// \requires not null.
      const object_info_table* info_table() const noexcept
      {
        assert(m_storage.ptr());
        return head()->info_table;
      }

      /// get address of member `value`
      /// \requires not null.
      auto* value() const noexcept
      {
        assert(m_storage.ptr());
        return &static_cast<pointer>(m_storage.ptr())->_value;
      }

      /// operator bool
      explicit operator bool() const noexcept
      {
        return m_storage.ptr() != nullptr;
      }

      /// use_count
      /// \requires not null.
      uint64_t use_count() const noexcept
      {
        assert(m_storage.ptr());
        return head()->refcount.load();
      }

      /// is_static
      /// \requires not null.
      bool is_static() const noexcept
      {
        assert(m_storage.ptr());
        return use_count() == 0;
      }

      /// release pointer
      /// \effects get() return nullptr after call
      pointer release() noexcept
      {
        auto tmp = static_cast<pointer>(m_storage.ptr());
        m_storage = {nullptr};
        return tmp;
      }

      /// swap data
      void swap(object_ptr<element_type>& obj) noexcept
      {
        std::swap(m_storage, obj.m_storage);
      }

      /// operator=
      object_ptr<element_type>&
        operator=(const object_ptr<element_type>& obj) noexcept
      {
        object_ptr(obj).swap(*this);
        return *this;
      }

      /// operator=
      template <class U>
      object_ptr<element_type>& operator=(const object_ptr<U>& obj) noexcept
      {
        object_ptr(obj).swap(*this);
        return *this;
      }

      /// operator=
      object_ptr<element_type>&
        operator=(object_ptr<element_type>&& obj) noexcept
      {
        object_ptr(std::move(obj)).swap(*this);
        return *this;
      }

      /// operator=
      template <class U>
      object_ptr<element_type>& operator=(object_ptr<U>&& obj) noexcept
      {
        object_ptr(std::move(obj)).swap(*this);
        return *this;
      }

      /// operator*
      auto& operator*() const noexcept
      {
        return *value();
      }

      /// operator->
      auto operator-> () const noexcept
      {
        return value();
      }

      // destroy
      ~object_ptr() noexcept;

    private:
      object_ptr_storage m_storage;
    };

    /// operator==
    template <class T, class U>
    bool operator==(const object_ptr<T>& lhs, const object_ptr<U>& rhs) noexcept
    {
      return lhs.get() == rhs.get();
    }

    /// operator==
    template <class T>
    bool operator==(nullptr_t, const object_ptr<T>& p) noexcept
    {
      return !p;
    }

    /// operator==
    template <class T>
    bool operator==(const object_ptr<T>& p, nullptr_t) noexcept
    {
      return !p;
    }

    /// operator!=
    template <class T, class U>
    bool operator!=(const object_ptr<T>& lhs, const object_ptr<U>& rhs) noexcept
    {
      return lhs.get() != rhs.get();
    }

    /// operator!=
    template <class T>
    bool operator!=(nullptr_t, const object_ptr<T>& p) noexcept
    {
      return static_cast<bool>(p);
    }

    /// operator!=
    template <class T>
    bool operator!=(const object_ptr<T>& p, nullptr_t) noexcept
    {
      return static_cast<bool>(p);
    }

  } // namespace interface

} // namespace TORI_NS::detail