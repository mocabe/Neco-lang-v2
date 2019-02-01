// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

#include "object.hpp"

namespace TORI_NS::detail {

  namespace interface {

    /// Smart pointer to manage heap-allocated objects
    template <class T = Object>
    class object_ptr
    {
    public:
      // value type
      using element_type = T;
      // pointer
      using pointer = T*;

      /// Constructor
      constexpr object_ptr() noexcept
        : m_ptr {nullptr}
      {
      }

      /// Constructor
      constexpr object_ptr(nullptr_t) noexcept
        : m_ptr {nullptr}
      {
      }

      /// Pointer constructor
      constexpr object_ptr(pointer p) noexcept
        : m_ptr {p}
      {
      }

      /// Copy constructor
      /// \effects increases reference count.
      object_ptr(const object_ptr<element_type>& other) noexcept
        : m_ptr {other.get()}
      {
        if (likely(m_ptr && !is_static()))
          head()->refcount.fetch_add();
      }

      /// Move constructor
      object_ptr(object_ptr<element_type>&& other) noexcept
        : m_ptr {other.release()}
      {
      }

      /// Copy convert constructor
      /// \effects increases reference count.
      template <class U>
      object_ptr(const object_ptr<U>& other) noexcept
        : m_ptr {other.get()}
      {
        if (likely(m_ptr && !is_static()))
          head()->refcount.fetch_add();
      }

      /// Move convert constructor
      template <class U>
      object_ptr(object_ptr<U>&& other) noexcept
        : m_ptr {other.release()}
      {
      }

      /// get address of object
      pointer get() const noexcept
      {
        return m_ptr;
      }

      /// get address of header
      auto* head() const noexcept
      {
        if constexpr (std::is_const_v<element_type>)
          return static_cast<const Object*>(m_ptr);
        else
          return static_cast<Object*>(m_ptr);
      }

      /// get address of info table
      /// \requires not null.
      const object_info_table* info_table() const noexcept
      {
        assert(m_ptr);
        return head()->info_table;
      }

      /// get address of member `value`
      /// \requires not null.
      auto* value() const noexcept
      {
        assert(m_ptr);
        return &m_ptr->_value;
      }

      /// operator bool
      explicit operator bool() const noexcept
      {
        return m_ptr != nullptr;
      }

      /// use_count
      /// \requires not null.
      uint64_t use_count() const noexcept
      {
        assert(m_ptr);
        return head()->refcount.load();
      }

      /// is_static
      /// \requires not null.
      bool is_static() const noexcept
      {
        assert(m_ptr);
        return use_count() == 0;
      }

      /// release pointer
      /// \effects get() return nullptr after call
      pointer release() noexcept
      {
        auto tmp = m_ptr;
        m_ptr = nullptr;
        return tmp;
      }

      /// swap data
      void swap(object_ptr<element_type>& obj) noexcept
      {
        std::swap(obj.m_ptr, m_ptr);
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
      /// pointer to object
      pointer m_ptr;
    };

    /// Object info table
    struct object_info_table
    {
      /// pointer to type object
      object_ptr<const Type> obj_type;
      /// total size of object
      uint64_t obj_size;
      /// size of additional storage
      uint64_t obj_ext_bytes;
      /// vtable of delete function
      void (*destroy)(const Object*);
      /// vtable of clone function
      Object* (*clone)(const Object*);
    };

    /// Destructor
    /// \effects Destroy object with vtable function when reference count become
    /// 0
    template <class T>
    object_ptr<T>::~object_ptr() noexcept
    {
      if (likely(m_ptr && !is_static())) {
        // delete object if needed
        if (head()->refcount.fetch_sub() == 1) {
          std::atomic_thread_fence(std::memory_order_acquire);
          info_table()->destroy(m_ptr);
        }
      }
    }

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