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
  //      [object_ptr<T>]                      [object_ptr<Imm>]
  //      (for heap objects)                   (for immediate values) 
  //              |   ^
  //   conversion |   | value_cast
  //              v   |
  //      [object_ptr<Object>]

  /// internal storage of object_ptr
  struct object_ptr_storage
  {
    enum class pointer_tags : uint32_t
    {
      // clang-format off

      //            32bit
      // h <---------------------> l
      //                        |<>|
      //                        tag (3bit)
      //                        ^^^^^^^^^^
      // |<---------------->|
      //       type flag

      pointer   = 0x00000000, //< plain pointer
      immediate = 0x00000001, //< immediate value
      apply     = 0x00000002, //< apply value (optional)
      exception = 0x00000003, //< exception value (optional)

      extract_mask = 0x00000007, // ...0111
      clear_mask   = 0xFFFFFFF8, // ...1000

      // clang-format on
    };

    /// extract pointer tag flag
    pointer_tags get_pointer_tag() const
    {
      uint32_t flag {};
      std::memcpy(&flag, &m_np.flag, 4);
      return static_cast<pointer_tags>(
        flag & (uint32_t)pointer_tags::extract_mask);
    }

    /// set new pointer tag flag
    void set_pointer_tag(pointer_tags flag)
    {
      uint32_t flg {};
      std::memcpy(&flg, &m_np.flag, 4);
      flg &= (uint32_t)pointer_tags::clear_mask;
      flg |= (uint32_t)flag;
      std::memcpy(&m_np.flag, &flg, 4);
    }

    /// get pointer
    Object* ptr() const
    {
      assert(is_pointer());
      // remove tags
      return (Object*)((uintptr_t)m_ptr & (uint32_t)pointer_tags::clear_mask);
    }

    /// get pointer without masking
    Object*& ptr_nomask()
    {
      assert(get_pointer_tag() == pointer_tags::pointer);
      return m_ptr;
    }

    Object* ptr_nomask() const
    {
      assert(get_pointer_tag() == pointer_tags::pointer);
      return m_ptr;
    }

    bool is_immediate() const
    {
      return get_pointer_tag() == pointer_tags::immediate;
    }

    bool is_pointer() const
    {
      // include apply and exception
      return !is_immediate();
    }

    bool is_apply() const
    {
      return get_pointer_tag() == pointer_tags::apply;
    }

    bool is_exception() const
    {
      return get_pointer_tag() == pointer_tags::exception;
    }

    enum class immediate_type_tags : uint32_t
    {
      // clang-format off

      //            32bit
      // h <---------------------> l
      //                        |<>|
      //                        tag(3bit)
      // |<------------------>|
      //       type flag
      //       ^^^^^^^^^

      u8  = 0x00000010, //< uint8_t
      u16 = 0x00000020, //< uint16_t
      u32 = 0x00000030, //< uint32_t
      i8  = 0x00000040, //< int8_t
      i16 = 0x00000050, //< int16_t
      i32 = 0x00000060, //< int32_t
      f32 = 0x00000070, //< float (IEEE754 32bit)

      extract_mask = 0xFFFFFFF0,
      clear_mask   = 0x0000000F,

      // clang-format on

    };

    /// extract immediate type flag
    /// \requires is_immediate() == true
    immediate_type_tags get_immediate_type_tag() const
    {
      assert(is_immediate());
      uint32_t flag {};
      std::memcpy(&flag, &m_np.flag, 4);
      return static_cast<immediate_type_tags>(
        flag & (uint32_t)immediate_type_tags::extract_mask);
    }

    /// \requires is_immediate() == true
    /// set new immediate type flag
    void set_immediate_type_tag(immediate_type_tags flag)
    {
      assert(is_immediate());
      uint32_t flg {};
      std::memcpy(&flg, &m_np.flag, 4);
      flg &= (uint32_t)immediate_type_tags::clear_mask;
      flg |= (uint32_t)flag;
      std::memcpy(&m_np.flag, &flg, 4);
    }

    /// non pointer data type
    struct non_pointer
    {
      // Only works when:
      // - 64bit
      // - little endian
      // - byte addressable
      //                    64bit
      // h <-------------------------------------> l
      // |        32       |     28     | 1 |   3  |
      // |      value      |    type    | - |  tag |

      /// pointer flags
      uint32_t flag;

      /// immediate type value
      immediate_transfarable_union value;
    };

    /// get immediate
    /// \requires is_immediate() == true
    const immediate_transfarable_union& immediate_union() const
    {
      assert(is_immediate());
      return m_np.value;
    }

    /// get immediate
    /// \requires is_immediate() == true
    immediate_transfarable_union& immediate_union()
    {
      assert(is_immediate());
      return m_np.value;
    }

    /// get pointer type
    auto get_pointer_type() const;

    /// get immediate type
    auto get_immediate_type() const;

    /// set new immediate type
    /// \requires is_immediate() == true
    template <class U>
    void set_immediate_type()
    {
      assert(is_immediate());

      set_pointer_tag(pointer_tags::immediate);

      if constexpr (type_c<U> == type_c<uint8_t>)
        return set_immediate_type_tag(immediate_type_tags::u8);
      if constexpr (type_c<U> == type_c<uint16_t>)
        return set_immediate_type_tag(immediate_type_tags::u16);
      if constexpr (type_c<U> == type_c<uint32_t>)
        return set_immediate_type_tag(immediate_type_tags::u32);
      if constexpr (type_c<U> == type_c<int8_t>)
        return set_immediate_type_tag(immediate_type_tags::i8);
      if constexpr (type_c<U> == type_c<int16_t>)
        return set_immediate_type_tag(immediate_type_tags::i16);
      if constexpr (type_c<U> == type_c<int32_t>)
        return set_immediate_type_tag(immediate_type_tags::i32);
      if constexpr (type_c<U> == type_c<float>)
        return set_immediate_type_tag(immediate_type_tags::f32);

      unreachable();
    }

    /// check immediate type
    /// \requires is_immediate() == true
    bool check_immediate_type(immediate_type_tags flag) const
    {
      assert(is_immediate());
      return get_immediate_type_tag() == flag;
    }

    /// check immediate type
    template <class U>
    bool has_immediate_type() const
    {
      if (!is_immediate())
        return false;

      if constexpr (type_c<U> == type_c<uint8_t>)
        return check_immediate_type(immediate_type_tags::u8);
      if constexpr (type_c<U> == type_c<uint16_t>)
        return check_immediate_type(immediate_type_tags::u16);
      if constexpr (type_c<U> == type_c<uint32_t>)
        return check_immediate_type(immediate_type_tags::u32);
      if constexpr (type_c<U> == type_c<int8_t>)
        return check_immediate_type(immediate_type_tags::i8);
      if constexpr (type_c<U> == type_c<int16_t>)
        return check_immediate_type(immediate_type_tags::i16);
      if constexpr (type_c<U> == type_c<int32_t>)
        return check_immediate_type(immediate_type_tags::i32);
      if constexpr (type_c<U> == type_c<float>)
        return check_immediate_type(immediate_type_tags::f32);

      return false;
    }

    template <class U>
    bool has_pointer_type() const; // define later

    explicit operator bool() const
    {
      if (is_pointer()) {
        return ptr();
      }

      if (is_immediate()) {
        if (get_immediate_type_tag() == immediate_type_tags::u8)
          return get<uint8_t>(m_np.value);
        if (get_immediate_type_tag() == immediate_type_tags::u16)
          return get<uint16_t>(m_np.value);
        if (get_immediate_type_tag() == immediate_type_tags::u32)
          return get<uint32_t>(m_np.value);
        if (get_immediate_type_tag() == immediate_type_tags::i8)
          return get<int8_t>(m_np.value);
        if (get_immediate_type_tag() == immediate_type_tags::i16)
          return get<int16_t>(m_np.value);
        if (get_immediate_type_tag() == immediate_type_tags::i32)
          return get<int32_t>(m_np.value);
        if (get_immediate_type_tag() == immediate_type_tags::f32)
          return get<float>(m_np.value);
      }

      unreachable();
    }

  private:
    // -------------------------------------------
    // Helper functions for constexpr initializer.
    // can be removed in C++20 since memcpy will become constexpr

    constexpr uint32_t get_pointer_tag_imm() const
    {
      return m_np.flag & (uint32_t)pointer_tags::extract_mask;
    }

    constexpr void set_pointer_tag_imm(pointer_tags flag)
    {
      m_np.flag &= (uint32_t)pointer_tags::clear_mask;
      m_np.flag |= (uint32_t)flag;
    }

    constexpr uint32_t get_immediate_type_tag_imm() const
    {
      return m_np.flag & (uint32_t)immediate_type_tags::extract_mask;
    }

    constexpr void set_immediate_type_tag_imm(immediate_type_tags flag)
    {
      m_np.flag &= (uint32_t)immediate_type_tags::clear_mask;
      m_np.flag |= (uint32_t)flag;
    }

  public:
    // -------------------------------------------
    // Ctor

    constexpr object_ptr_storage(Object* p) noexcept
      : m_ptr {p}
    {
      set_pointer_tag(pointer_tags::pointer);
    }
    constexpr object_ptr_storage(const Object* p) noexcept
      : m_ptr {const_cast<Object*>(p)}
    {
      set_pointer_tag(pointer_tags::pointer);
    }

    constexpr object_ptr_storage(nullptr_t p) noexcept
      : m_ptr {p}
    {
      set_pointer_tag(pointer_tags::pointer);
    }

    template <class T, class = std::enable_if_t<!std::is_pointer_v<T>>>
    constexpr object_ptr_storage(T t) noexcept
      : m_np {{}, {t}}
    {
      set_pointer_tag_imm(pointer_tags::immediate);

      if constexpr (type_c<T> == type_c<uint8_t>)
        set_immediate_type_tag_imm(immediate_type_tags::u8);
      if constexpr (type_c<T> == type_c<uint16_t>)
        set_immediate_type_tag_imm(immediate_type_tags::u16);
      if constexpr (type_c<T> == type_c<uint32_t>)
        set_immediate_type_tag_imm(immediate_type_tags::u32);
      if constexpr (type_c<T> == type_c<int8_t>)
        set_immediate_type_tag_imm(immediate_type_tags::i8);
      if constexpr (type_c<T> == type_c<int16_t>)
        set_immediate_type_tag_imm(immediate_type_tags::i16);
      if constexpr (type_c<T> == type_c<int32_t>)
        set_immediate_type_tag_imm(immediate_type_tags::i32);
      if constexpr (type_c<T> == type_c<float>)
        set_immediate_type_tag_imm(immediate_type_tags::f32);
    }

  private:
    union
    {
      /// pointer to object
      Object* m_ptr;
      /// non-pointer transfarable immediate value
      non_pointer m_np;
    };
  };

  static_assert(std::is_standard_layout_v<object_ptr_storage>);

  namespace interface {

    /// Smart pointer to manage heap-allocated objects
    /// Holds a pointer to an object in storage member without tagging lowest
    /// bits.
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
        if (likely(m_storage.ptr_nomask() && !is_static()))
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
        if (likely(m_storage.ptr_nomask() && !is_static()))
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
        return static_cast<pointer>(m_storage.ptr_nomask());
      }

      /// get address of header
      auto* head() const noexcept
      {
        if constexpr (std::is_const_v<element_type>)
          return static_cast<const Object*>(m_storage.ptr_nomask());
        else
          return static_cast<Object*>(m_storage.ptr_nomask());
      }

      /// get address of info table
      /// \requires not null.
      const object_info_table* info_table() const noexcept
      {
        assert(m_storage.ptr_nomask());
        return head()->info_table;
      }

      /// get address of member `value`
      /// \requires not null.
      auto* value() const noexcept
      {
        assert(m_storage.ptr_nomask());
        return &static_cast<pointer>(m_storage.ptr_nomask())->_value;
      }

      /// operator bool
      explicit operator bool() const noexcept
      {
        return m_storage.ptr_nomask() != nullptr;
      }

      /// use_count
      /// \requires not null.
      uint64_t use_count() const noexcept
      {
        assert(m_storage.ptr_nomask());
        return head()->refcount.load();
      }

      /// is_static
      /// \requires not null.
      bool is_static() const noexcept
      {
        assert(m_storage.ptr_nomask());
        return use_count() == 0;
      }

      /// release pointer
      /// \effects get() return nullptr after call
      pointer release() noexcept
      {
        auto tmp = static_cast<pointer>(m_storage.ptr_nomask());
        m_storage.ptr_nomask() = nullptr;
        return tmp;
      }

      /// swap data
      void swap(object_ptr<element_type>& obj) noexcept
      {
        std::swap(obj.m_storage.ptr_nomask(), m_storage.ptr_nomask());
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
      if (likely(m_storage.ptr_nomask() && !is_static())) {
        // delete object if needed
        if (head()->refcount.fetch_sub() == 1) {
          std::atomic_thread_fence(std::memory_order_acquire);
          info_table()->destroy(m_storage.ptr_nomask());
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
      template <class T>
      friend object_ptr<T> value_cast_if(const object_ptr_generic&);
      template <class T>
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
        // take over ownership
        if (m_storage.is_pointer())
          other.m_storage.ptr_nomask() = nullptr;
      }

      /// convert ctor
      template <class T>
      object_ptr_generic(const object_ptr<T>& ptr)
        : m_storage {ptr.m_storage}
      {
        if constexpr (is_transfarable_immediate(type_c<T>)) {
          assert(m_storage.is_immediate());
        } else {
          assert(m_storage.is_pointer());
          // add refcount
          if (m_storage.ptr_nomask() && !is_static())
            m_storage.ptr_nomask()->refcount.fetch_add();
          // TODO: add optional tags
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
          ptr.m_storage.ptr_nomask() = nullptr;
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
          other.m_storage.ptr_nomask() = nullptr;
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
        if (m_storage.is_pointer())
          return m_storage.ptr();
        else {
          return static_cast<bool>(m_storage);
        }
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

    // fwd
    template <class T>
    object_ptr<const Type> object_type();

  } // namespace interface

  template <class U>
  bool object_ptr_storage::has_pointer_type() const
  {
    if (!is_pointer())
      return false;

    return ptr() && same_type(ptr()->info_table->obj_type, object_type<U>());
  }

  auto object_ptr_storage::get_immediate_type() const
  {
    assert(is_immediate());

    if (check_immediate_type(immediate_type_tags::u8))
      return object_type<uint8_t>();
    if (check_immediate_type(immediate_type_tags::u16))
      return object_type<uint16_t>();
    if (check_immediate_type(immediate_type_tags::u32))
      return object_type<uint32_t>();
    if (check_immediate_type(immediate_type_tags::i8))
      return object_type<int8_t>();
    if (check_immediate_type(immediate_type_tags::i16))
      return object_type<int16_t>();
    if (check_immediate_type(immediate_type_tags::i32))
      return object_type<int32_t>();
    if (check_immediate_type(immediate_type_tags::f32))
      return object_type<float>();

    unreachable();
  }

  auto object_ptr_storage::get_pointer_type() const
  {
    assert(is_pointer());

    auto p = ptr();
    // null
    if (!p)
      return object_ptr<const Type>(nullptr);
    // get type
    return p->info_table->obj_type;
  }

} // namespace TORI_NS::detail