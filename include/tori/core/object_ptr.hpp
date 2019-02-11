// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

#include "object.hpp"
#include "unboxed.hpp"

#include <cstring>

namespace TORI_NS::detail {

  //                       [object_ptr_generic]
  //                             ^ |  | ^
  //          ____conversion_____| |  | |____conversion_____
  //         |   __________________|  |_________________   |
  //         |   |    value_cast          value_cast   |   |
  //         |   v                                     v   |
  //      [object_ptr<T>]                      [object_ptr<imm<T>>]
  //      (for heap objects)                   (for immediate values) 
  //              |   ^
  //   conversion |   | value_cast
  //              v   |
  //      [object_ptr<Object>]

  /// internal storage of object_ptr
  struct object_ptr_storage
  {
    enum class pointer_tag_flags : uint32_t
    {
      // clang-format off

      //            32bit
      // h <---------------------> l
      // | ---- , ----, ----, ---- | :8bit x4
      //                    |<---->|
      //                      tag
      //                     ^^^^^
      // |<---------------->|
      //       type flag

      pointer   = 0x00, //< plain pointer
      immediate = 0x01, //< immediate value
      apply     = 0x02, //< apply value (optional)
      exception = 0x04, //< exception value (optional)

      extract_mask = 0x000000FF,
      clear_mask   = 0xFFFFFF00,

      // clang-format on
    };

    enum class immediate_type_flags : uint32_t
    {
      // clang-format off

      //            32bit
      // h <---------------------> l
      // | ---- , ----, ----, ---- | :8bit x4
      //                    |<---->|
      //                      tag
      // |<---------------->|
      //       type flag
      //       ^^^^^^^^^

      u8  = 0x00000100, //< uint8_t
      u16 = 0x00000200, //< uint16_t
      u32 = 0x00000400, //< uint32_t
      i8  = 0x00000800, //< int8_t
      i16 = 0x00001000, //< int16_t
      i32 = 0x00002000, //< int32_t
      f32 = 0x00004000, //< float (IEEE754 32bit)

      extract_mask = 0xFFFFFF00,
      clear_mask   = 0x000000FF,

      // clang-format on
    };

    // immediate
    struct non_pointer
    {
      // **LITTLE ENDIAN ONLY**  
      // h <---------------------------------> l
      // | immediate union | [3] [2] [1] | [0] |
      // |      value      |     type    | tag |

      /// pointer flags
      uint32_t flag;

      /// immediate type value
      immediate_transfarable_union value;
    };

    /// extract pointer tag flag
    uint32_t get_pointer_tag_flag() const
    {
      uint32_t flag {};
      std::memcpy(&flag, &imm.flag, 4);
      return flag & (uint32_t)pointer_tag_flags::extract_mask;
    }

    /// set new pointer tag flag
    void set_pointer_tag_flag(pointer_tag_flags flag)
    {
      uint32_t flg {};
      std::memcpy(&flg, &imm.flag, 4);
      flg &= (uint32_t)pointer_tag_flags::clear_mask;
      flg |= (uint32_t)flag;
      std::memcpy(&imm.flag, &flg, 4);
    }

    /// extract immediate type flag
    /// \requires is_immediate() == true
    uint32_t get_immediate_type_flag() const
    {
      assert(is_immediate());
      uint32_t flag {};
      std::memcpy(&flag, &imm.flag, 4);
      return flag & (uint32_t)immediate_type_flags::extract_mask;
    }

    /// \requires is_immediate() == true
    /// set new immediate type flag
    void set_immediate_type_flag(immediate_type_flags flag)
    {
      assert(is_immediate());
      uint32_t flg {};
      std::memcpy(&flg, &imm.flag, 4);
      flg &= (uint32_t)immediate_type_flags::clear_mask;
      flg |= (uint32_t)flag;
      std::memcpy(&imm.flag, &flg, 4);
    }

    /// pointer?
    bool is_pointer() const
    {
      return get_pointer_tag_flag() == (uint32_t)pointer_tag_flags::pointer;
    }

    /// immediate?
    bool is_immediate() const
    {
      return get_pointer_tag_flag() == (uint32_t)pointer_tag_flags::immediate;
    }

    /// get immediate
    /// \requires is_immediate() == true
    const immediate_transfarable_union& immediate_union() const
    {
      assert(is_immediate());
      return imm.value;
    }

    /// get immediate
    /// \requires is_immediate() == true
    immediate_transfarable_union& immediate_union()
    {
      assert(is_immediate());
      return imm.value;
    }

    /// get immediate type
    auto get_immediate_type() const;

    /// set new immediate type
    /// \requires is_immediate() == true
    template <class U>
    void set_immediate_type()
    {
      assert(is_immediate());

      set_pointer_tag_flag(pointer_tag_flags::immediate);

      if constexpr (type_c<U> == type_c<uint8_t>)
        return set_immediate_type_flag(immediate_type_flags::u8);
      if constexpr (type_c<U> == type_c<uint16_t>)
        return set_immediate_type_flag(immediate_type_flags::u16);
      if constexpr (type_c<U> == type_c<uint32_t>)
        return set_immediate_type_flag(immediate_type_flags::u32);
      if constexpr (type_c<U> == type_c<int8_t>)
        return set_immediate_type_flag(immediate_type_flags::i8);
      if constexpr (type_c<U> == type_c<int16_t>)
        return set_immediate_type_flag(immediate_type_flags::i16);
      if constexpr (type_c<U> == type_c<int32_t>)
        return set_immediate_type_flag(immediate_type_flags::i32);
      if constexpr (type_c<U> == type_c<float>)
        return set_immediate_type_flag(immediate_type_flags::f32);

      unreachable();
    }

    /// check immediate type
    /// \requires is_immediate() == true
    bool check_immediate_type(immediate_type_flags flag) const
    {
      assert(is_immediate());
      return get_immediate_type_flag() == (uint32_t)flag;
    }

    /// check immediate type
    template <class U>
    bool has_immediate_type() const
    {
      if (!is_immediate())
        return false;

      if constexpr (type_c<U> == type_c<uint8_t>)
        return check_immediate_type(immediate_type_flags::u8);
      if constexpr (type_c<U> == type_c<uint16_t>)
        return check_immediate_type(immediate_type_flags::u16);
      if constexpr (type_c<U> == type_c<uint32_t>)
        return check_immediate_type(immediate_type_flags::u32);
      if constexpr (type_c<U> == type_c<int8_t>)
        return check_immediate_type(immediate_type_flags::i8);
      if constexpr (type_c<U> == type_c<int16_t>)
        return check_immediate_type(immediate_type_flags::i16);
      if constexpr (type_c<U> == type_c<int32_t>)
        return check_immediate_type(immediate_type_flags::i32);
      if constexpr (type_c<U> == type_c<float>)
        return check_immediate_type(immediate_type_flags::f32);

      return false;
    }

    template <class U>
    bool has_pointer_type() const; // define later

  private:
    // -------------------------------------------
    // Helper functions for constexpr initializer.
    // can be removed in C++20 since memcpy will become constexpr

    constexpr uint32_t get_pointer_tag_flag_imm() const
    {
      return imm.flag & (uint32_t)pointer_tag_flags::extract_mask;
    }

    constexpr void set_pointer_tag_flag_imm(pointer_tag_flags flag)
    {
      imm.flag &= (uint32_t)pointer_tag_flags::clear_mask;
      imm.flag |= (uint32_t)flag;
    }

    constexpr uint32_t get_immediate_type_flag_imm() const
    {
      return imm.flag & (uint32_t)immediate_type_flags::extract_mask;
    }

    constexpr void set_immediate_type_flag_imm(immediate_type_flags flag)
    {
      imm.flag &= (uint32_t)immediate_type_flags::clear_mask;
      imm.flag |= (uint32_t)flag;
    }

  public:
    // -------------------------------------------
    // Ctor

    constexpr object_ptr_storage(Object* p) noexcept
      : ptr {p}
    {
      set_pointer_tag_flag(pointer_tag_flags::pointer);
    }
    constexpr object_ptr_storage(const Object* p) noexcept
      : ptr {const_cast<Object*>(p)}
    {
      set_pointer_tag_flag(pointer_tag_flags::pointer);
    }

    constexpr object_ptr_storage(nullptr_t p) noexcept
      : ptr {p}
    {
      set_pointer_tag_flag(pointer_tag_flags::pointer);
    }

    template <class T, class = std::enable_if_t<!std::is_pointer_v<T>>>
    constexpr object_ptr_storage(T t) noexcept
      : imm {{}, {t}}
    {
      set_pointer_tag_flag_imm(pointer_tag_flags::immediate);

      if constexpr (type_c<T> == type_c<uint8_t>)
        set_immediate_type_flag_imm(immediate_type_flags::u8);
      if constexpr (type_c<T> == type_c<uint16_t>)
        set_immediate_type_flag_imm(immediate_type_flags::u16);
      if constexpr (type_c<T> == type_c<uint32_t>)
        set_immediate_type_flag_imm(immediate_type_flags::u32);
      if constexpr (type_c<T> == type_c<int8_t>)
        set_immediate_type_flag_imm(immediate_type_flags::i8);
      if constexpr (type_c<T> == type_c<int16_t>)
        set_immediate_type_flag_imm(immediate_type_flags::i16);
      if constexpr (type_c<T> == type_c<int32_t>)
        set_immediate_type_flag_imm(immediate_type_flags::i32);
      if constexpr (type_c<T> == type_c<float>)
        set_immediate_type_flag_imm(immediate_type_flags::f32);
    }

    union
    {
      /// pointer to object
      Object* ptr;
      /// non-pointer transfarable immediate value
      non_pointer imm;
    };
  };

  static_assert(std::is_standard_layout_v<object_ptr_storage>);

  namespace interface {

    /// Smart pointer to manage heap-allocated objects
    template <class T = Object, class = void>
    class object_ptr
    {
      template <class, class>
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
        if (likely(m_storage.ptr && !is_static()))
          head()->refcount.fetch_add();
      }

      /// Move constructor
      object_ptr(object_ptr<element_type>&& other) noexcept
        : m_storage {other.m_storage}
      {
        other.m_storage = nullptr;
      }

      /// Copy convert constructor
      /// \effects increases reference count.
      template <
        class U,
        class = std::enable_if_t<!is_transfarable_immediate(type_c<U>)>>
      object_ptr(const object_ptr<U>& other) noexcept
        : m_storage {other.m_storage}
      {
        if (likely(m_storage.ptr && !is_static()))
          head()->refcount.fetch_add();
      }

      /// Move convert constructor
      template <
        class U,
        class = std::enable_if_t<!is_transfarable_immediate(type_c<U>)>>
      object_ptr(object_ptr<U>&& other) noexcept
        : m_storage {other.m_storage}
      {
        other.m_storage = nullptr;
      }

      /// get address of object
      pointer get() const noexcept
      {
        return static_cast<pointer>(m_storage.ptr);
      }

      /// get address of header
      auto* head() const noexcept
      {
        if constexpr (std::is_const_v<element_type>)
          return static_cast<const Object*>(m_storage.ptr);
        else
          return static_cast<Object*>(m_storage.ptr);
      }

      /// get address of info table
      /// \requires not null.
      const object_info_table* info_table() const noexcept
      {
        assert(m_storage.ptr);
        return head()->info_table;
      }

      /// get address of member `value`
      /// \requires not null.
      auto* value() const noexcept
      {
        assert(m_storage.ptr);
        return &static_cast<pointer>(m_storage.ptr)->_value;
      }

      /// operator bool
      explicit operator bool() const noexcept
      {
        return m_storage.ptr != nullptr;
      }

      /// use_count
      /// \requires not null.
      uint64_t use_count() const noexcept
      {
        assert(m_storage.ptr);
        return head()->refcount.load();
      }

      /// is_static
      /// \requires not null.
      bool is_static() const noexcept
      {
        assert(m_storage.ptr);
        return use_count() == 0;
      }

      /// release pointer
      /// \effects get() return nullptr after call
      pointer release() noexcept
      {
        auto tmp = static_cast<pointer>(m_storage.ptr);
        m_storage.ptr = nullptr;
        return tmp;
      }

      /// swap data
      void swap(object_ptr<element_type>& obj) noexcept
      {
        std::swap(obj.m_storage.ptr, m_storage.ptr);
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

    /// specialized for immediate types
    template <class T>
    class object_ptr<
      T,
      std::enable_if_t<is_transfarable_immediate(
        /*MSVC ICE workaround*/ meta_type<T> {})>>
    {
      friend class object_ptr_generic;

    public:
      constexpr object_ptr()
        : m_storage {T {}}
      {
      }

      constexpr object_ptr(T v)
        : m_storage {v}
      {
      }

      constexpr object_ptr(const object_ptr& other)
        : m_storage {other.m_storage}
      {
      }

      constexpr object_ptr& operator=(const object_ptr& other)
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
    template <class T, class U>
    object_ptr<T, U>::~object_ptr() noexcept
    {
      if (likely(m_storage.ptr && !is_static())) {
        // delete object if needed
        if (head()->refcount.fetch_sub() == 1) {
          std::atomic_thread_fence(std::memory_order_acquire);
          info_table()->destroy(m_storage.ptr);
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

    /// deduction guide
    template <class T>
    object_ptr(T*)->object_ptr<T>;

    /// deduction guide
    template <class T>
    object_ptr(T)->object_ptr<T>;

    /// type-erased object_ptr.
    /// contains either a pointer-to-object or immediate(unboxed) value.
    class object_ptr_generic
    {
      template <class T>
      friend object_ptr<T> value_cast(const object_ptr_generic&);
      template <class T>
      friend object_ptr<T> value_cast(object_ptr_generic&&);

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
        // take over ownership
        if (m_storage.is_pointer())
          other.m_storage.ptr = nullptr;
      }

      /// convert ctor (pointer)
      template <class T>
      object_ptr_generic(const object_ptr<T>& ptr)
        : m_storage {ptr.m_storage}
      {
        if constexpr (is_transfarable_immediate(type_c<T>)) {
          assert(m_storage.is_immediate());
        } else {
          assert(m_storage.is_pointer());
          if (m_storage.ptr && !is_static())
            m_storage.ptr->refcount.fetch_add();
        }
      }

      /// convert move ctor (pointer)
      template <class T>
      object_ptr_generic(object_ptr<T>&& ptr)
        : m_storage {ptr.m_storage}
      {
        if constexpr (is_transfarable_immediate(type_c<T>)) {
          assert(m_storage.is_immediate());
        } else {
          assert(m_storage.is_pointer());
          ptr.m_storage.ptr = nullptr;
        }
      }

      object_ptr_generic& operator=(const object_ptr_generic& other)
      {
        m_storage = other.m_storage;
        increment_refcount_when_pointer();
        return *this;
      }

      object_ptr_generic& operator=(object_ptr_generic&& other)
      {
        m_storage = other.m_storage;
        if (m_storage.is_pointer())
          other.m_storage.ptr = nullptr;
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

    private:
      bool is_static() const
      {
        assert(m_storage.is_pointer());
        return m_storage.ptr->refcount.load() == 0;
      }

      /// increment refcount
      void increment_refcount_when_pointer()
      {
        if (m_storage.is_pointer() && m_storage.ptr && !is_static())
          m_storage.ptr->refcount.fetch_add();
      }

      /// decrement refcount and free object when needed
      void decrement_refcount_when_pointer()
      {
        if (m_storage.is_pointer() && m_storage.ptr && !is_static()) {
          if (m_storage.ptr->refcount.fetch_sub() == 1) {
            std::atomic_thread_fence(std::memory_order_acquire);
            m_storage.ptr->info_table->destroy(m_storage.ptr);
          }
        }
      }

    private:
      object_ptr_storage m_storage;
    };

    // fwd
    template <class T>
    object_ptr<const Type> object_type();

  } // namespace interface

  template <class U>
  bool object_ptr_storage::has_pointer_type() const
  {
    if (!is_pointer())
      return false;

    return ptr && same_type(ptr->info_table->obj_type, object_type<U>());
  }

  auto object_ptr_storage::get_immediate_type() const
  {
    assert(is_immediate());

    if (check_immediate_type(immediate_type_flags::u8))
      return object_type<uint8_t>();
    if (check_immediate_type(immediate_type_flags::u16))
      return object_type<uint16_t>();
    if (check_immediate_type(immediate_type_flags::u32))
      return object_type<uint32_t>();
    if (check_immediate_type(immediate_type_flags::i8))
      return object_type<int8_t>();
    if (check_immediate_type(immediate_type_flags::i16))
      return object_type<int16_t>();
    if (check_immediate_type(immediate_type_flags::i32))
      return object_type<int32_t>();
    if (check_immediate_type(immediate_type_flags::f32))
      return object_type<float>();

    unreachable();
  }

} // namespace TORI_NS::detail