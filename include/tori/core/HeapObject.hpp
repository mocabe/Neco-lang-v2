#pragma once 

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file HeapObject

#include <cstdint>
#include <type_traits>
#include <atomic>
#include <cassert>
#include <variant>
#include <stdexcept>

// config
#include "../config/config.hpp"
// type
#include "StaticTypeUtil.hpp"

/// size of additional buffer in heap object header
#if defined(OBJECT_HEADER_EXTEND_BYTES)
namespace TORI_NS::detail {
  constexpr size_t object_header_extend_bytes = OBJECT_HEADER_EXTEND_BYTES;
}
#else
namespace TORI_NS::detail {
  constexpr size_t object_header_extend_bytes = 0;
}
#endif

namespace TORI_NS::detail {

  // value type
  struct ValueType;
  // arrow type
  struct ArrowType;
  // type variable
  struct VarType;

  // TypeValue
  using TypeValue = std::variant<ValueType, ArrowType, VarType>;

  // interface
  namespace interface {

    // heap-allocated object of type T
    template <
      class T,
      template <class> class AllocatorTemplate = std::allocator>
    struct BoxedHeapObject;
    // handler for heap-allocated object
    template <class>
    class object_ptr;
    // object info table
    struct object_info_table;
    // heap-allocated runtime type infomation
    using Type = BoxedHeapObject<TypeValue>;

    /// Base class of heap-allocated objects
    struct HeapObject {
      /// term
      using term = tm_value<HeapObject>;
      /// reference counter
      /// \notes static objects have zero for reference count.
      struct _refcount {
        constexpr _refcount() noexcept : atomic{0} {}
        constexpr _refcount(size_t v) noexcept : atomic{v} {}
        constexpr _refcount(const _refcount& refcnt) noexcept
          : atomic{refcnt.raw} {}
        union {
          std::atomic<size_t> atomic;
          size_t raw;
        };
      } mutable refcount;

      /// pointer to info-table
      const object_info_table* info_table;

#if defined(OBJECT_HEADER_EXTEND_BYTES)
      /// additional buffer storage
      std::byte obj_ext_buffer[object_header_extend_bytes] = {};
#endif
    };

    /// Smart pointer to manage heap-allocated objects
    template <class T = HeapObject>
    class object_ptr {
      friend class object_ptr<HeapObject>;

      /// move cast function
      template <class U, class S>
      friend object_ptr<U> value_cast(object_ptr<S>&&);
      /// move cast function
      template <class U, class S>
      friend object_ptr<U> value_cast_if(object_ptr<S>&&) noexcept;
      /// move cast function
      template <class U, class S>
      friend object_ptr<U> closure_cast(object_ptr<S>&&);
      /// move cast function
      template <class U, class S>
      friend object_ptr<U> closure_cast_if(object_ptr<S>&&) noexcept;

    public:
      using value_type = T;
      /// Constructor
      constexpr object_ptr() noexcept : m_ptr{nullptr} {}
      /// Constructor
      constexpr object_ptr(nullptr_t) noexcept : m_ptr{nullptr} {}
      /// Pointer constructor
      constexpr object_ptr(value_type* p) noexcept : m_ptr{p} {}
      /// Copy constructor
      /// \effects increases reference count.
      object_ptr(const object_ptr<value_type>& obj) noexcept
        : m_ptr{obj.m_ptr} {
        // when not static object
        if (m_ptr && head()->refcount.atomic != 0) {
          head()->refcount.atomic.fetch_add(1u, std::memory_order_relaxed);
        }
      }
      /// Move constructor
      object_ptr(object_ptr<value_type>&& obj) noexcept : m_ptr{obj.m_ptr} {
        obj.m_ptr = nullptr;
      }
      /// Copy convert constructor
      /// \effects increases reference count.
      template <class U>
      object_ptr(const object_ptr<U>& obj) noexcept : m_ptr{obj.get()} {
        // when not static object
        if (m_ptr && head()->refcount.atomic != 0) {
          head()->refcount.atomic.fetch_add(1u, std::memory_order_relaxed);
        }
      }
      /// Move convert constructor
      template <class U>
      object_ptr(object_ptr<U>&& obj) noexcept : m_ptr{obj.get()} {
        obj.m_ptr = nullptr;
      }
      /// get address of object
      value_type* get() const noexcept {
        return m_ptr;
      }
      /// get address of header
      auto head() const noexcept {
        if constexpr (std::is_const_v<value_type>)
          return static_cast<const HeapObject*>(m_ptr);
        else
          return static_cast<HeapObject*>(m_ptr);
      }
      /// get address of info table
      /// \requires Object is not null.
      const object_info_table* info_table() const noexcept {
        return head()->info_table;
      };
      /// get address of member `value`
      /// \requires Object is not null.
      auto* value() const noexcept {
        return &m_ptr->value;
      }
      /// operator bool
      explicit operator bool() const noexcept {
        return m_ptr != nullptr;
      }
      /// use_count
      /// \requires Object is not null.
      size_t use_count() const noexcept {
        return head()->refcount.atomic;
      }
      /// swap data
      void swap(object_ptr<value_type>& obj) noexcept {
        std::swap(obj.m_ptr, m_ptr);
      }
      /// operator=
      object_ptr<value_type>& operator=(
        const object_ptr<value_type>& obj) noexcept {
        object_ptr(obj).swap(*this);
        return *this;
      }
      /// operator=
      template <class U>
      object_ptr<value_type>& operator=(const object_ptr<U>& obj) noexcept {
        object_ptr(obj).swap(*this);
        return *this;
      }
      /// operator=
      object_ptr<value_type>& operator=(object_ptr<value_type>&& obj) noexcept {
        object_ptr(std::move(obj)).swap(*this);
        return *this;
      }
      /// operator=
      template <class U>
      object_ptr<value_type>& operator=(object_ptr<U>&& obj) noexcept {
        object_ptr(std::move(obj)).swap(*this);
        return *this;
      }

      // destroy
      ~object_ptr() noexcept;
      // clone
      object_ptr<value_type> clone() const;

    private:
      /// pointer to object
      value_type* m_ptr;

    public:
      /// operator*
      auto& operator*() const noexcept {
        return *value();
      }
      /// operator->
      auto operator-> () const noexcept {
        return value();
      }
    };

    /// Object info table
    struct object_info_table {
      /// pointer to type object
      object_ptr<const Type> obj_type;
      /// total size of object
      size_t obj_size;
      /// size of additional storage
      size_t obj_ext_bytes;
      /// vtable of delete function
      void (*destroy)(HeapObject*);
      /// vtable of clone function
      HeapObject* (*clone)(const HeapObject*);
    };

    /// Clone
    /// \effects Call copy constructor of the object from vtable.
    /// \returns `object_ptr<T>` pointing new object.
    /// \throws `std::bad_alloc` when `clone` returned nullptr.
    /// \throws `std::runtime_error` when object is null.
    /// \notes Reference count of new object will be set to 1.
    template <class T>
    object_ptr<T> object_ptr<T>::clone() const {
      if (!m_ptr) throw std::runtime_error("clone() to null object");
      auto r = static_cast<value_type*>(info_table()->clone(m_ptr));
      if (unlikely(!r)) throw std::bad_alloc();
      r->refcount.atomic = 1;
      return r;
    }

    /// Destructor
    /// \effects Destroy object with vtable function when reference count become
    /// 0
    template <class T>
    object_ptr<T>::~object_ptr() noexcept {
      // when not static object
      if (m_ptr && head()->refcount.atomic != 0) {
        // delete object if needed
        if (
          head()->refcount.atomic.fetch_sub(1u, std::memory_order_release) ==
          1) {
          std::atomic_thread_fence(std::memory_order_acquire);
          info_table()->destroy(const_cast<std::remove_cv_t<T>*>(m_ptr));
        }
      }
    }

    /// operator==
    template <class T, class U>
    bool operator==(
      const object_ptr<T>& lhs, const object_ptr<U>& rhs) noexcept {
      return lhs.get() == rhs.get();
    }

    /// operator==
    template <class T>
    bool operator==(nullptr_t, const object_ptr<T>& p) noexcept {
      return !p;
    }

    /// operator==
    template <class T>
    bool operator==(const object_ptr<T>& p, nullptr_t) noexcept {
      return !p;
    }

    /// operator!=
    template <class T, class U>
    bool operator!=(
      const object_ptr<T>& lhs, const object_ptr<U>& rhs) noexcept {
      return lhs.get() != rhs.get();
    }

    /// operator!=
    template <class T>
    bool operator!=(nullptr_t, const object_ptr<T>& p) noexcept {
      return static_cast<bool>(p);
    }

    /// operator!=
    template <class T>
    bool operator!=(const object_ptr<T>& p, nullptr_t) noexcept {
      return static_cast<bool>(p);
    }

    /// make object
    template <class T, class... Args>
    object_ptr<T> make_object(Args&&... args) {
      return new T{std::forward<Args>(args)...};
    }
  } // namespace interface
} // namespace TORI_NS::detail