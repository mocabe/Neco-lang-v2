#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file TypeValue

#include "HeapObject.hpp"
#include <array>
#include <cstring>

namespace TORI_NS::detail {

  // ------------------------------------------
  // Type
  // ------------------------------------------

  /// Value type
  struct ValueType {
    /// Max name length
    static constexpr size_t max_name_size = 32;
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
      if (lhs.name == rhs.name)
        return true;
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
      } else
        return std::memcmp(        //
                 lhs.name->data(), //
                 rhs.name->data(), //
                 max_name_size) == 0;
    }
  };

  /// Arrow type
  struct ArrowType {
    /// argument type
    object_ptr<const Type> captured;
    /// return type
    object_ptr<const Type> returns;
  };

  /// Any type
  struct VarType {
    /// unique id for VarTpye object
    uint64_t id;
  };

  /// Base class for TypeValue
  class TypeValueStorage {
  public:
    /// default ctor is disabled
    TypeValueStorage() = delete;

    // initializers
    TypeValueStorage(const ValueType& t) : value {t}, m_index {value_index} {}
    TypeValueStorage(const ArrowType& t) : arrow {t}, m_index {arrow_index} {}
    TypeValueStorage(const VarType& t) : var {t}, m_index {var_index} {}
    /// Copy constructor
    TypeValueStorage(const TypeValueStorage& other) : m_index {other.m_index}
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
    ~TypeValueStorage() noexcept
    {
      // call destructor
      if (m_index == value_index)
        value.~ValueType();
      if (m_index == arrow_index)
        arrow.~ArrowType();
      if (m_index == var_index)
        var.~VarType();
    }

    /// Get index of type in union
    template <class T>
    static constexpr uint64_t index_of()
    {
      if constexpr (std::is_same_v<T, ValueType>)
        return value_index;
      else if constexpr (std::is_same_v<T, ArrowType>)
        return arrow_index;
      else if constexpr (std::is_same_v<T, VarType>)
        return var_index;
      else
        static_assert(false_v<T>, "Invalid type");
    }

    /// Provides raw access to union
    template <uint64_t Idx, class TpValStorage>
    static constexpr decltype(auto) raw_access(TpValStorage&& v)
    {
      if constexpr (Idx == index_of<ValueType>())
        return (v.value);
      else if constexpr (Idx == index_of<ArrowType>())
        return (v.arrow);
      else if constexpr (Idx == index_of<VarType>())
        return (v.var);
      else
        static_assert(false_v<TpValStorage>);
    }

    /// Get index
    constexpr size_t index() const
    {
      return m_index;
    }

  private:
    // hard-coded type index
    static constexpr uint64_t value_index = 0;
    static constexpr uint64_t arrow_index = 1;
    static constexpr uint64_t var_index = 2;

  public:
    /// Get type from index value.
    /// Returns void when index is invalid
    template <uint64_t Idx>
    using type_of = std::conditional_t<
      Idx == value_index,
      ValueType,
      std::conditional_t<Idx == arrow_index,
                         ArrowType,
                         std::conditional_t<Idx == var_index, VarType, void>>>;

  private:
    // 16 byte union
    union {
      ValueType value;
      ArrowType arrow;
      VarType var;
    };
    // 8 byte index
    uint64_t m_index;
  };

  /// Simulates std::variant<ValueType, ArrowType, VarType>
  class TypeValue : private TypeValueStorage {
    /// Storage type
    using storage = TypeValueStorage;

  public: // friend functions
    template <uint64_t Idx, class TpVal>
    friend constexpr decltype(auto) TypeValue_get(TpVal&&);

  public:
    // inherit ctors
    using TypeValueStorage::TypeValueStorage;

    /// Get index
    constexpr uint64_t index() const
    {
      return storage::index();
    }

  private: // Helps accessing storage
    TypeValueStorage& get_storage() &
    {
      return *this;
    }
    TypeValueStorage&& get_storage() &&
    {
      return std::move(*this);
    }
    const TypeValueStorage& get_storage() const&
    {
      return *this;
    }
    const TypeValueStorage&& get_storage() const&&
    {
      return std::move(*this);
    }
  };

  /// Access TypeValue from index.
  /// Throws std::bad_cast when index is wrong.
  template <uint64_t Idx, class TpVal>
  constexpr decltype(auto) TypeValue_get(TpVal&& val)
  {
    if (Idx != val.index())
      throw std::bad_cast();
    return TypeValueStorage::raw_access<Idx>(val.get_storage());
  }

  /// std::get() equivalent
  template <uint64_t Idx>
  decltype(auto) get(const TypeValue& val)
  {
    return TypeValue_get<Idx>(val);
  }

  /// std::get() equivalent
  template <uint64_t Idx>
  decltype(auto) get(const TypeValue&& val)
  {
    return TypeValue_get<Idx>(std::move(val));
  }

  /// std::get() equivalent
  template <uint64_t Idx>
  decltype(auto) get(TypeValue& val)
  {
    return TypeValue_get<Idx>(val);
  }

  /// std::get() equivalent
  template <uint64_t Idx>
  decltype(auto) get(TypeValue&& val)
  {
    return TypeValue_get<Idx>(std::move(val));
  }

  /// std::get() equivalent
  template <class T>
  decltype(auto) get(const TypeValue& val)
  {
    return get<TypeValueStorage::index_of<T>()>(val);
  }

  /// std::get() equivalent
  template <class T>
  decltype(auto) get(const TypeValue&& val)
  {
    return get<TypeValueStorage::index_of<T>()>(std::move(val));
  }

  /// std::get() equivalent
  template <class T>
  decltype(auto) get(TypeValue& val)
  {
    return get<TypeValueStorage::index_of<T>()>(val);
  }

  /// std::get() equivalent
  template <class T>
  decltype(auto) get(TypeValue&& val)
  {
    return get<TypeValueStorage::index_of<T>()>(std::move(val));
  }

  /// std::get_if() equivalent
  template <size_t Idx>
  constexpr std::add_pointer_t<const TypeValueStorage::type_of<Idx>>
    get_if(const TypeValue* val)
  {
    if (val && Idx == val->index())
      return &get<Idx>(*val);
    return nullptr;
  }

  /// std::get_if() equivalent
  template <size_t Idx>
  constexpr std::add_pointer_t<TypeValueStorage::type_of<Idx>>
    get_if(TypeValue* val)
  {
    if (val && Idx == val->index())
      return &get<Idx>(*val);
    return nullptr;
  }

  /// std::get_if() equivalent
  template <class T>
  constexpr std::add_pointer_t<const T> get_if(const TypeValue* val)
  {
    return get_if<TypeValueStorage::index_of<T>()>(val);
  }

  /// std::get_if() equivalent
  template <class T>
  constexpr std::add_pointer_t<T> get_if(TypeValue* val)
  {
    return get_if<TypeValueStorage::index_of<T>()>(val);
  }
} // namespace TORI_NS::detail