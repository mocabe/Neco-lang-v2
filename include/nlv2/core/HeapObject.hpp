// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file HeapObject.hpp

#pragma once 

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
  // recursive type
  struct RecType;
  // type variable
  struct AutoType;

  // TypeValue
  using TypeValue = std::variant<ValueType, ArrowType, RecType, AutoType>;

  // interface
  namespace interface {

    // heap-allocated object of type T
    template <
      class T,
      template <class> class AllocatorTemplate = std::allocator>
    struct BoxedHeapObject;
    // handler for heap-allocated object
    template <class>
    class ObjectPtr;
    // object info table
    struct ObjectInfoTable;
    // heap-allocated runtime type infomation
    using Type = BoxedHeapObject<TypeValue>;

    /// Base class of heap-allocated objects
    struct HeapObject {
      /// term
      using term = TmValue<HeapObject>;
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
      const ObjectInfoTable* info_table;

#if defined(OBJECT_HEADER_EXTEND_BYTES)
      /// additional buffer storage
      std::byte obj_ext_buffer[object_header_extend_bytes] = {};
#endif
    };

    /// Smart pointer to manage heap-allocated objects
    template <class T = HeapObject>
    class ObjectPtr {
      friend class ObjectPtr<HeapObject>;

      /// move cast function
      template <class U>
      friend ObjectPtr<U> value_cast(ObjectPtr<T>&&);
      /// move cast function
      template <class U>
      friend ObjectPtr<U> value_cast_if(ObjectPtr<T>&&);
      /// move cast function
      template <class U>
      friend ObjectPtr<U> closure_cast(ObjectPtr<T>&&);
      /// move cast function
      template <class U>
      friend ObjectPtr<U> closure_cast_if(ObjectPtr<T>&&);

    public:
      using value_type = T;
      /// Constructor
      constexpr ObjectPtr() noexcept : m_ptr{nullptr} {}
      /// Constructor
      constexpr ObjectPtr(nullptr_t) noexcept : m_ptr{nullptr} {}
      /// Pointer constructor
      constexpr ObjectPtr(value_type* p) noexcept : m_ptr{p} {}
      /// Copy constructor
      /// \effects increases reference count.
      ObjectPtr(const ObjectPtr<value_type>& obj) noexcept : m_ptr{obj.m_ptr} {
        if (m_ptr && m_ptr->refcount.atomic != 0) m_ptr->refcount.atomic++;
      }
      /// Move constructor
      ObjectPtr(ObjectPtr<value_type>&& obj) noexcept : m_ptr{obj.m_ptr} {
        obj.m_ptr = nullptr;
      }
      /// Copy convert constructor
      /// \effects increases reference count.
      template <class U>
      explicit ObjectPtr(const ObjectPtr<U>& obj) noexcept : m_ptr{obj.head()} {
        if (m_ptr && m_ptr->refcount.atomic != 0) m_ptr->refcount.atomic++;
      }
      /// Move convert constructor
      template <class U>
      explicit ObjectPtr(ObjectPtr<U>&& obj) noexcept : m_ptr{obj.head()} {
        obj.m_ptr = nullptr;
      }
      /// get address of head of the object
      value_type* head() const noexcept {
        return m_ptr;
      }
      /// get address of info table
      /// \requires Object is not null.
      const ObjectInfoTable* info_table() const noexcept {
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
        return m_ptr->refcount.atomic;
      }
      /// swap data
      void swap(ObjectPtr<value_type>& obj) noexcept {
        std::swap(obj.m_ptr, m_ptr);
      }
      /// operator=
      ObjectPtr<value_type>& operator=(
        const ObjectPtr<value_type>& obj) noexcept {
        ObjectPtr(obj).swap(*this);
        return *this;
      }
      /// operator=
      ObjectPtr<value_type>& operator=(ObjectPtr<value_type>&& obj) noexcept {
        ObjectPtr(std::move(obj)).swap(*this);
        return *this;
      }

      // destroy
      ~ObjectPtr() noexcept;
      // clone
      ObjectPtr<value_type> clone() const;

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
    struct ObjectInfoTable {
      /// pointer to type object
      ObjectPtr<const Type> obj_type;
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
    /// \returns `ObjectPtr<T>` pointing new object.
    /// \throws `std::bad_alloc` when `clone` returned nullptr.
    /// \throws `std::runtime_error` when object is null.
    /// \notes Reference count of new object will be set to 1.
    template <class T>
    ObjectPtr<T> ObjectPtr<T>::clone() const {
      if (!m_ptr) throw std::runtime_error("clone() to null object");
      auto r = static_cast<value_type*>(info_table()->clone(m_ptr));
      if (!r) throw std::bad_alloc();
      r->refcount.atomic = 1;
      return r;
    }

    /// Destructor
    /// \effects Destroy object with vtable function when reference count become
    /// 0
    template <class T>
    ObjectPtr<T>::~ObjectPtr() noexcept {
      if (
        m_ptr && m_ptr->refcount.atomic != 0 && --(m_ptr->refcount.atomic) == 0)
        info_table()->destroy(const_cast<std::remove_cv_t<T>*>(m_ptr));
    }

    /// operator==
    template <class T, class U>
    bool operator==(const ObjectPtr<T>& lhs, const ObjectPtr<U>& rhs) noexcept {
      return lhs.head() == rhs.head();
    }

    /// operator==
    template <class T>
    bool operator==(nullptr_t, const ObjectPtr<T>& p) noexcept {
      return !p;
    }

    /// operator==
    template <class T>
    bool operator==(const ObjectPtr<T>& p, nullptr_t) noexcept {
      return !p;
    }

    /// operator!=
    template <class T, class U>
    bool operator!=(const ObjectPtr<T>& lhs, const ObjectPtr<U>& rhs) noexcept {
      return lhs.head() != rhs.head();
    }

    /// operator!=
    template <class T>
    bool operator!=(nullptr_t, const ObjectPtr<T>& p) noexcept {
      return static_cast<bool>(p);
    }

    /// operator!=
    template <class T>
    bool operator!=(const ObjectPtr<T>& p, nullptr_t) noexcept {
      return static_cast<bool>(p);
    }

    /// make object
    template <class T, class... Args>
    ObjectPtr<T> make_object(Args&&... args) {
      return new T{std::forward<Args>(args)...};
    }
  } // namespace interface
} // namespace TORI_NS::detail