#pragma once 

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file HeapObject

#include <cstdint>
#include <type_traits>
#include <atomic>
#include <cassert>
#include <stdexcept>
#include <new>

// config
#include "../config/config.hpp"
// type
#include "StaticTypeUtil.hpp"

namespace TORI_NS::detail {

#if defined(OBJECT_HEADER_EXTEND_BYTES)
  constexpr uint64_t object_header_extend_bytes = OBJECT_HEADER_EXTEND_BYTES;
#else
  /// size of additional buffer in heap object header
  constexpr uint64_t object_header_extend_bytes = 0;
#endif

  // value type
  struct ValueType;
  // arrow type
  struct ArrowType;
  // type variable
  struct VarType;

  // TypeValue
  class TypeValue;

  // interface
  namespace interface {

    /// reference count
    template <class T>
    class atomic_refcount
    {
    public:
      constexpr atomic_refcount() noexcept
        : atomic {0}
      {
      }

      constexpr atomic_refcount(T v) noexcept
        : atomic {v}
      {
      }

      atomic_refcount(const atomic_refcount& other) noexcept
        : atomic {other.load()}
      {
      }

      atomic_refcount& operator=(const atomic_refcount& other) noexcept
      {
        store(other.load());
        return *this;
      }

      atomic_refcount& operator=(T v) noexcept
      {
        store(v);
        return *this;
      }

      T load() const noexcept
      {
        return atomic.load(std::memory_order_acquire);
      }

      void store(T v) noexcept
      {
        atomic.store(v, std::memory_order_release);
      }

      /// use memory_order_relaxed
      T fetch_add() noexcept
      {
        return atomic.fetch_add(1u, std::memory_order_relaxed);
      }

      /// use memory_order_release
      T fetch_sub() noexcept
      {
        return atomic.fetch_sub(1u, std::memory_order_release);
      }

    private:
      std::atomic<T> atomic;
      static_assert(sizeof(T) == sizeof(std::atomic<T>));
    };

    // heap-allocated object of type T
    template <class T>
    struct BoxedHeapObject;

    // handler for heap-allocated object
    template <class>
    class object_ptr;

    // object info table
    struct object_info_table;

    // heap-allocated runtime type infomation
    using Type = BoxedHeapObject<TypeValue>;

    /// Base class of heap-allocated objects
    struct HeapObject
    {
      /// term
      using term = tm_value<HeapObject>;

      // reference count
      mutable atomic_refcount<uint64_t> refcount;

      /// pointer to info-table
      const object_info_table* info_table;

#if defined(OBJECT_HEADER_EXTEND_BYTES)
      /// additional buffer storage
      std::byte obj_ext_buffer[object_header_extend_bytes] = {};
#endif
    };

    /// stateless allocator for heap objects
    template <class T>
    struct object_allocator_traits
    {
      /// allocate memory
      static void* allocate(size_t n)
      {
        return std::allocator<T>().allocate(n);
      }
      /// deallocate memory
      static void deallocate(void* ptr, size_t n)
      {
        std::allocator<T>().deallocate(static_cast<T*>(ptr), n);
      }
    };

    /// Smart pointer to manage heap-allocated objects
    template <class T = HeapObject>
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
          return static_cast<const HeapObject*>(m_ptr);
        else
          return static_cast<HeapObject*>(m_ptr);
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
      void (*destroy)(HeapObject*);
      /// vtable of clone function
      HeapObject* (*clone)(const HeapObject*);
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
          info_table()->destroy(const_cast<std::remove_cv_t<T>*>(m_ptr));
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

    /// Clone
    /// \effects Call copy constructor of the object from vtable.
    /// \returns `object_ptr<T>` pointing new object.
    /// \throws `std::bad_alloc` when `clone` returned nullptr.
    /// \throws `std::runtime_error` when object is null.
    /// \notes Reference count of new object will be set to 1.
    /// \requires not null.
    template <class T>
    object_ptr<T> clone(const object_ptr<T>& obj)
    {
      assert(obj);
      auto r = static_cast<T*>(obj.info_table()->clone(obj.get()));
      if (unlikely(!r))
        throw std::bad_alloc();
      return r;
    }

    /// make object
    template <class T, class... Args>
    object_ptr<T> make_object(Args&&... args)
    {
      return new T {std::forward<Args>(args)...};
    }

  } // namespace interface

} // namespace TORI_NS::detail