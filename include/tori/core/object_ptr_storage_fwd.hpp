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
  //    [object_ptr<T>]                           [immediate<T>]
  //    (for heap objects)                     (for immediate values)
  //              |   ^
  //   conversion |   | value_cast
  //              v   |
  //      [object_ptr<Object>]

  /// internal storage of object_ptr
  struct object_ptr_storage
  {
    /// pointer tag values. stored in lowest 3 bits of 64bit data.
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
      apply     = 0x00000002, //< pointer to apply (optional)
      exception = 0x00000003, //< pointer to exception (optional)

      extract_mask = 0x00000007, // ...0111
      clear_mask   = 0xFFFFFFF8, // ...1000

      // clang-format on
    };

    /// extract pointer tag flag
    pointer_tags get_pointer_tag() const noexcept
    {
      uint32_t flag {};
      std::memcpy(&flag, &m_np.flag, 4);
      return static_cast<pointer_tags>(
        flag & (uint32_t)pointer_tags::extract_mask);
    }

    /// set new pointer tag flag
    void set_pointer_tag(pointer_tags flag) noexcept
    {
      uint32_t flg {};
      std::memcpy(&flg, &m_np.flag, 4);
      flg &= (uint32_t)pointer_tags::clear_mask;
      flg |= (uint32_t)flag;
      std::memcpy(&m_np.flag, &flg, 4);
    }

    /// get pointer
    Object* ptr() const noexcept
    {
      assert(is_pointer());
      // Ideally, single AND instruction which is very cheap.
      return (Object*)((uintptr_t)m_ptr & (uint32_t)pointer_tags::clear_mask);
    }

    /// immediate?
    bool is_immediate() const noexcept
    {
      return get_pointer_tag() == pointer_tags::immediate;
    }

    /// pointer?
    bool is_pointer() const noexcept
    {
      // include apply and exception
      return !is_immediate();
    }

    /// apply? (optional)
    bool is_apply() const noexcept
    {
      return get_pointer_tag() == pointer_tags::apply;
    }

    /// exception? (optional)
    bool is_exception() const noexcept
    {
      return get_pointer_tag() == pointer_tags::exception;
    }

    /// immediate type tags. stored in higher 28bits of lower 32bits of total
    /// 64bit data.
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
    immediate_type_tags get_immediate_type_tag() const noexcept
    {
      assert(is_immediate());
      uint32_t flag {};
      std::memcpy(&flag, &m_np.flag, 4);
      return static_cast<immediate_type_tags>(
        flag & (uint32_t)immediate_type_tags::extract_mask);
    }

    /// \requires is_immediate() == true
    /// set new immediate type flag
    void set_immediate_type_tag(immediate_type_tags flag) noexcept
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
    const immediate_transfarable_union& immediate_union() const noexcept
    {
      assert(is_immediate());
      return m_np.value;
    }

    /// get immediate
    /// \requires is_immediate() == true
    immediate_transfarable_union& immediate_union() noexcept
    {
      assert(is_immediate());
      return m_np.value;
    }

    /// get pointer type
    auto get_pointer_type() const noexcept;

    /// get immediate type
    auto get_immediate_type() const noexcept;

    /// set new immediate type
    /// \requires is_immediate() == true
    template <class U>
    void set_immediate_type() noexcept
    {
      assert(is_immediate());

      set_pointer_tag(pointer_tags::immediate);

      if constexpr (type_c<U> == type_c<uint8_t>)
        return set_immediate_type_tag(immediate_type_tags::u8);
      else if constexpr (type_c<U> == type_c<uint16_t>)
        return set_immediate_type_tag(immediate_type_tags::u16);
      else if constexpr (type_c<U> == type_c<uint32_t>)
        return set_immediate_type_tag(immediate_type_tags::u32);
      else if constexpr (type_c<U> == type_c<int8_t>)
        return set_immediate_type_tag(immediate_type_tags::i8);
      else if constexpr (type_c<U> == type_c<int16_t>)
        return set_immediate_type_tag(immediate_type_tags::i16);
      else if constexpr (type_c<U> == type_c<int32_t>)
        return set_immediate_type_tag(immediate_type_tags::i32);
      else if constexpr (type_c<U> == type_c<float>)
        return set_immediate_type_tag(immediate_type_tags::f32);
      else
        static_assert(false_v<U>);

      unreachable();
    }

    /// check immediate type
    /// \requires is_immediate() == true
    bool check_immediate_type(immediate_type_tags flag) const noexcept
    {
      assert(is_immediate());
      return get_immediate_type_tag() == flag;
    }

    /// check immediate type
    template <class U>
    bool has_immediate_type() const noexcept
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

    /// check pointer type 
    template <class U>
    bool has_pointer_type() const noexcept;

    /// operator bool
    explicit operator bool() const noexcept
    {
      if (is_pointer())
        return ptr();

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

    constexpr uint32_t get_pointer_tag_imm() const noexcept
    {
      return m_np.flag & (uint32_t)pointer_tags::extract_mask;
    }

    constexpr void set_pointer_tag_imm(pointer_tags flag) noexcept
    {
      m_np.flag &= (uint32_t)pointer_tags::clear_mask;
      m_np.flag |= (uint32_t)flag;
    }

    constexpr uint32_t get_immediate_type_tag_imm() const noexcept
    {
      return m_np.flag & (uint32_t)immediate_type_tags::extract_mask;
    }

    constexpr void set_immediate_type_tag_imm(immediate_type_tags flag) noexcept
    {
      m_np.flag &= (uint32_t)immediate_type_tags::clear_mask;
      m_np.flag |= (uint32_t)flag;
    }

  public:

    /// Ctor
    constexpr object_ptr_storage(Object* p) noexcept
      : m_ptr {p}
    {
      set_pointer_tag_imm(pointer_tags::pointer);
    }

    /// Ctor
    constexpr object_ptr_storage(const Object* p) noexcept
      : m_ptr {p}
    {
      set_pointer_tag_imm(pointer_tags::pointer);
    }

    /// Ctor
    constexpr object_ptr_storage(nullptr_t p) noexcept
      : m_ptr {p}
    {
      set_pointer_tag_imm(pointer_tags::pointer);
    }

    /// Ctor
    template <class T, class = std::enable_if_t<!std::is_pointer_v<T>>>
    constexpr object_ptr_storage(T t) noexcept
      : m_np {{}, {t}}
    {
      set_pointer_tag_imm(pointer_tags::immediate);

      if constexpr (type_c<T> == type_c<uint8_t>)
        set_immediate_type_tag_imm(immediate_type_tags::u8);
      else if constexpr (type_c<T> == type_c<uint16_t>)
        set_immediate_type_tag_imm(immediate_type_tags::u16);
      else if constexpr (type_c<T> == type_c<uint32_t>)
        set_immediate_type_tag_imm(immediate_type_tags::u32);
      else if constexpr (type_c<T> == type_c<int8_t>)
        set_immediate_type_tag_imm(immediate_type_tags::i8);
      else if constexpr (type_c<T> == type_c<int16_t>)
        set_immediate_type_tag_imm(immediate_type_tags::i16);
      else if constexpr (type_c<T> == type_c<int32_t>)
        set_immediate_type_tag_imm(immediate_type_tags::i32);
      else if constexpr (type_c<T> == type_c<float>)
        set_immediate_type_tag_imm(immediate_type_tags::f32);
      else
        static_assert(false_v<T>);
    }

  private:
    union
    {
      /// pointer to object
      const Object* m_ptr;
      /// non-pointer transfarable immediate value
      non_pointer m_np;
    };
  };

  // should be standard layout
  static_assert(std::is_standard_layout_v<object_ptr_storage>);
  static_assert(std::is_trivially_copy_constructible_v<object_ptr_storage>);
  static_assert(std::is_trivially_copy_assignable_v<object_ptr_storage>);
  static_assert(std::is_trivially_copyable_v<object_ptr_storage>);

} // namespace TORI_NS::detail