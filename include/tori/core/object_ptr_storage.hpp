// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "object.hpp"

#include <cstring>

namespace TORI_NS::detail {

  /// internal storage of object_ptr
  struct object_ptr_storage
  {
    /// pointer tag values. stored in lowest 3 bits of 64bit data.
    enum class pointer_tags : uint32_t
    {
      // clang-format off

      // h <---------------------> l
      //                        |<>|
      //                        tag (3bit)
      //                        ^^^^^^^^^^

      pointer   = 0x00000000, //< plain pointer
      apply     = 0x00000001, //< pointer to apply (optional)
      exception = 0x00000002, //< pointer to exception (optional)

      extract_mask = 0x00000007, // ...0111
      clear_mask   = 0xFFFFFFF8, // ...1000

      // clang-format on
    };

    /// extract pointer tag flag
    pointer_tags get_pointer_tag() const noexcept
    {
      uint32_t flag;
      std::memcpy(&flag, &m_np.flag, sizeof(flag));
      return static_cast<pointer_tags>(
        flag & (uint32_t)pointer_tags::extract_mask);
    }

    /// set new pointer tag flag
    void set_pointer_tag(pointer_tags flag) noexcept
    {
      uint32_t flg;
      std::memcpy(&flg, &m_np.flag, sizeof(flg));
      flg &= (uint32_t)pointer_tags::clear_mask;
      flg |= (uint32_t)flag;
      std::memcpy(&m_np.flag, &flg, sizeof(m_np.flag));
    }

    /// get pointer
    /// Ideally, optimized into single AND instruction.
    Object* ptr() const noexcept
    {
      // load as non-pointer
      non_pointer np;
      std::memcpy(&np, &m_np, sizeof(np));
      // clear pointer tag
      np.flag &= (uint32_t)pointer_tags::clear_mask;
      // cast to pointer
      Object* ptr;
      std::memcpy(&ptr, &np, sizeof(ptr));
      return ptr;
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

    /// static?
    bool is_static() const noexcept
    {
      assert(ptr());
      return ptr()->refcount.load() == 0;
    }

    /// non pointer struct
    struct non_pointer
    {
      /// pointer tag and flags
      uint32_t flag;
      /// reserved for future use
      uint32_t reserved;
    };

  private:
    // -------------------------------------------
    // Helper functions for constexpr initializer.
    // gcc and clang accepst non-constexpr call but msvc doesn't.
    // Can be removed in C++20 since memcpy will become constexpr.

    constexpr uint32_t get_pointer_tag_constexpr() const noexcept
    {
      return m_np.flag & (uint32_t)pointer_tags::extract_mask;
    }

    constexpr void set_pointer_tag_constexpr(pointer_tags flag) noexcept
    {
      m_np.flag &= (uint32_t)pointer_tags::clear_mask;
      m_np.flag |= (uint32_t)flag;
    }

  public:

    /// Ctor
    constexpr object_ptr_storage(Object* p) noexcept
      : m_ptr {p}
    {
      set_pointer_tag_constexpr(pointer_tags::pointer);
    }

    /// Ctor
    constexpr object_ptr_storage(const Object* p) noexcept
      : m_ptr {p}
    {
      set_pointer_tag_constexpr(pointer_tags::pointer);
    }

    /// Ctor
    constexpr object_ptr_storage(nullptr_t p) noexcept
      : m_ptr {p}
    {
      set_pointer_tag_constexpr(pointer_tags::pointer);
    }

  private:
    union
    {
      /// pointer to object
      const Object* m_ptr;
      /// non pointer value
      non_pointer m_np;
    };
  };

  // should be standard layout
  static_assert(std::is_standard_layout_v<object_ptr_storage>);
  static_assert(std::is_trivially_copy_constructible_v<object_ptr_storage>);
  static_assert(std::is_trivially_copy_assignable_v<object_ptr_storage>);
  static_assert(std::is_trivially_copyable_v<object_ptr_storage>);

} // namespace TORI_NS::detail