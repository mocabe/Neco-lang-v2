#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#include "object_ptr.hpp"

#include <array>
#include <cstring>

namespace TORI_NS::detail {

  // ------------------------------------------
  // TypeValue union values

  /// Value type
  struct ValueType
  {
    /// Max name length
    static constexpr uint64_t max_name_size = 32;
    /// buffer type
    using buffer_type = std::array<char, max_name_size>;
    /// buffer
    const buffer_type* name;

    /// get C-style string
    const char* c_str() const
    {
      return name->data();
    }

    /// compare two value types
    static bool compare(const ValueType& lhs, const ValueType& rhs)
    {
      if constexpr (has_AVX2 && max_name_size == 32) {
        // AVX2
        // buffers should be aligned as 32byte
        // load each buffers into 256-bit registers
        auto ymm0 =
          _mm256_load_si256(reinterpret_cast<const __m256i*>(lhs.name->data()));
        auto ymm1 =
          _mm256_load_si256(reinterpret_cast<const __m256i*>(rhs.name->data()));
        // compare
        auto cmpeq = _mm256_cmpeq_epi8(ymm0, ymm1);
        // get mask
        unsigned mask = _mm256_movemask_epi8(cmpeq);
        // clear upper bits for other SIMD operations
        _mm256_zeroupper();
        return mask == 0xffffffffU;
      } else if constexpr (has_AVX && max_name_size == 32) {
        // AVX
        // buffers should be aligned as 16byte
        // load buffer into 2 xmm registers
        auto xmm0 = _mm_load_si128(
          reinterpret_cast<const __m128i*>(lhs.name->data() + 0));
        auto xmm1 = _mm_load_si128(
          reinterpret_cast<const __m128i*>(lhs.name->data() + 16));
        // compare registers to another buffer on memory
        auto cmp1 = _mm_cmpeq_epi8(
          xmm0, *reinterpret_cast<const __m128i*>(rhs.name->data() + 0));
        auto cmp2 = _mm_cmpeq_epi8(
          xmm1, *reinterpret_cast<const __m128i*>(rhs.name->data() + 16));
        // get cmp result
        auto cmp = _mm_and_si128(cmp1, cmp2);
        // get mask
        auto mask = _mm_movemask_epi8(cmp);
        return mask == 0xffffU;
      } else {
        // fallback to memcmp
        return std::memcmp(        //
                 lhs.name->data(), //
                 rhs.name->data(), //
                 max_name_size) == 0;
      }
    }
  };

  /// Arrow type
  struct ArrowType
  {
    /// argument type
    object_ptr<const Type> captured;
    /// return type
    object_ptr<const Type> returns;
  };

  /// Any type
  struct VarType
  {
    /// unique id for VarTpye object
    uint64_t id;
  };

  // ------------------------------------------
  // TypeValue

  /// Base class for TypeValue
  class type_object_value
  {
  public:
    /// default ctor is disabled
    type_object_value() = delete;

    // initializers
    type_object_value(ValueType t)
      : value {std::move(t)}
      , m_index {value_index}
    {
    }

    type_object_value(ArrowType t)
      : arrow {std::move(t)}
      , m_index {arrow_index}
    {
    }

    type_object_value(VarType t)
      : var {std::move(t)}
      , m_index {var_index}
    {
    }

    /// Copy constructor
    type_object_value(const type_object_value& other)
      : m_index {other.m_index}
    {
      // copy union
      if (other.m_index == value_index) {
        value = other.value;
      }
      if (other.m_index == arrow_index) {
        arrow = other.arrow;
      }
      if (other.m_index == var_index) {
        var = other.var;
      }
      throw std::bad_cast();
    }

    /// Destructor
    ~type_object_value() noexcept
    {
      // call destructor
      if (m_index == value_index)
        value.~ValueType();
      if (m_index == arrow_index)
        arrow.~ArrowType();
      if (m_index == var_index)
        var.~VarType();
    }

    /// Get index
    constexpr uint64_t index() const
    {
      return m_index;
    }

  private:
    // hard-coded type index
    static constexpr uint64_t value_index = 0;
    static constexpr uint64_t arrow_index = 1;
    static constexpr uint64_t var_index = 2;

  private:
    template <uint64_t Idx, class TpVal>
    friend constexpr decltype(auto) TypeValue_get(TpVal&& v);
    template <uint64_t Idx, class TpVal>
    friend constexpr decltype(auto) TypeValue_access(TpVal&& v);
    template <class T>
    friend constexpr uint64_t TypeValue_index();

  private:
    // 16 byte union
    union
    {
      ValueType value;
      ArrowType arrow;
      VarType var;
    };

    // 8 byte index
    uint64_t m_index;
  };

  // ------------------------------------------
  // TypeValue utility

  /// allow access to raw value
  template <uint64_t Idx, class TpVal>
  constexpr decltype(auto) TypeValue_access(TpVal&& v)
  {
    static_assert(std::is_same_v<std::decay_t<TpVal>, type_object_value>);
    if constexpr (Idx == type_object_value::value_index) {
      auto&& ref = std::forward<TpVal>(v).value;
      return ref;
    } else if constexpr (Idx == type_object_value::arrow_index) {
      auto&& ref = std::forward<TpVal>(v).arrow;
      return ref;
    } else if constexpr (Idx == type_object_value::var_index) {
      auto&& ref = std::forward<TpVal>(v).var;
      return ref;
    } else {
      static_assert(false_v<TpVal>, "Invalid index for TypeValue");
    }
  }

  /// get
  template <uint64_t Idx, class TpVal>
  constexpr decltype(auto) TypeValue_get(TpVal&& v)
  {
    static_assert(std::is_same_v<std::decay_t<TpVal>, type_object_value>);
    if (v.m_index != Idx)
      throw std::bad_cast();
    return TypeValue_access<Idx>(std::forward<TpVal>(v));
  }

  /// Get index
  template <class T>
  constexpr uint64_t TypeValue_index()
  {
    if constexpr (std::is_same_v<std::decay_t<T>, ValueType>) {
      return type_object_value::value_index;
    } else if constexpr (std::is_same_v<std::decay_t<T>, ArrowType>) {
      return type_object_value::arrow_index;
    } else if constexpr (std::is_same_v<std::decay_t<T>, VarType>) {
      return type_object_value::var_index;
    } else {
      static_assert(false_v<T>);
    }
  }

  /// std::get() equivalent
  template <class T>
  decltype(auto) get(const type_object_value& val)
  {
    return TypeValue_get<TypeValue_index<T>()>(val);
  }

  /// std::get() equivalent
  template <class T>
  decltype(auto) get(const type_object_value&& val)
  {
    return TypeValue_get<TypeValue_index<T>()>(std::move(val));
  }

  /// std::get() equivalent
  template <class T>
  decltype(auto) get(type_object_value& val)
  {
    return TypeValue_get<TypeValue_index<T>()>(val);
  }

  /// std::get() equivalent
  template <class T>
  decltype(auto) get(type_object_value&& val)
  {
    return TypeValue_get<TypeValue_index<T>()>(std::move(val));
  }

  /// std::get_if() equivalent
  template <class T>
  constexpr std::add_pointer_t<const T> get_if(const type_object_value* val)
  {
    constexpr auto Idx = TypeValue_index<T>();
    if (val && Idx == val->index())
      return &get<T>(*val);
    return nullptr;
  }

  /// std::get_if() equivalent
  template <class T>
  constexpr std::add_pointer_t<T> get_if(type_object_value* val)
  {
    constexpr auto Idx = TypeValue_index<T>();
    if (val && Idx == val->index())
      return &get<T>(*val);
    return nullptr;
  }

} // namespace TORI_NS::detail