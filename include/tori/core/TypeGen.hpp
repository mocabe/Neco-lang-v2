// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "BoxedHeapObject.hpp"
#include <cstring>
#include <string>
#include <memory>
#include <variant>
#include <vector>
#include <utility>
#include <exception>
#include <array>

// decl system types
#define TORI_DECL_TYPE(TYPE)                  \
  template <>                                 \
  struct object_type_traits<TYPE> {           \
    static constexpr char name[] = "_" #TYPE; \
  };

namespace TORI_NS {
  /// object type traits
  template <class T>
  struct object_type_traits;
  /// type
  TORI_DECL_TYPE(Type)
  /// HeapObject
  TORI_DECL_TYPE(HeapObject)
} // namespace TORI_NS

namespace TORI_NS::detail {

  namespace interface {
    /// proxy type of closure
    template <class... Ts>
    struct closure : HeapObject {
      /// term
      using term = TmClosure<typename Ts::term...>;
    };

    /// Type variable
    template <class Tag>
    struct Auto {
      /// term
      using term = TmVar<Tag>;
      // to make distinct address for each tag
      static constexpr const int _id_gen = 42;
      /// Id
      static constexpr const void* const id = &_id_gen;
    };

  } // namespace interface

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
    const char* c_str() const {
      return name->data();
    }
    /// compare two value types
    static bool compare(const ValueType& lhs, const ValueType& rhs) {
      if (lhs.name == rhs.name) return true;
      // AVX2
      if constexpr (has_AVX2 && max_name_size == 32) {
        // buffers should be aligned as 32byte
        auto ymm0 =
          _mm256_load_si256(reinterpret_cast<const __m256i*>(lhs.name->data()));
        auto ymm1 =
          _mm256_load_si256(reinterpret_cast<const __m256i*>(rhs.name->data()));
        auto cmpeq = _mm256_cmpeq_epi8(ymm0, ymm1);
        unsigned mask = _mm256_movemask_epi8(cmpeq);
        _mm256_zeroupper();
        return mask == 0xffffffffU;
      } else
        return std::memcmp(        //
                 lhs.name->data(), //
                 rhs.name->data(), //
                 max_name_size) == 0;
    }
  };
  /// Arrow type
  struct ArrowType {
    ObjectPtr<const Type> captured;
    ObjectPtr<const Type> returns;
  };
  /// Any type
  struct VarType {
    uint64_t id;
  };


  // ------------------------------------------
  // Object type
  // ------------------------------------------

  /// value type
  template <class T>
  struct value_type {
    static const Type type;
  };

  template <class T, class... Ts>
  struct arrow_type_impl {
    static const Type type;
  };

  template <class T1, class T2>
  struct arrow_type_impl<T1, T2> {
    static const Type type;
  };

  /// arrow type
  template <class T, class... Ts>
  struct arrow_type;

  template <class T, class... Ts>
  struct arrow_type<T, TmClosure<Ts...>> {
    static constexpr const Type* type = &arrow_type_impl<Ts...>::type;
  };

  /// var type
  template <class T>
  struct vartype {
    static const Type type;
  };

  namespace interface {
    /// object type generator
    template <class T, class>
    struct object_type {};

    // value
    template <class T>
    struct object_type<T, std::enable_if_t<has_TmValue_v<T>>> {
      static constexpr const Type* type = &value_type<T>::type;
      static ObjectPtr<const Type> get() {
        return type;
      }
    };
    // closure
    template <class T>
    struct object_type<T, std::enable_if_t<has_TmClosure_v<T>>> {
      static constexpr const Type* type = arrow_type<T, typename T::term>::type;
      static ObjectPtr<const Type> get() {
        return type;
      }
    };
    // var
    template <class T>
    struct object_type<T, std::enable_if_t<has_TmVar_v<T>>> {
      static constexpr const Type* type = &vartype<T>::type;
      static ObjectPtr<const Type> get() {
        return type;
      }
    };
  } // namespace interface

  /// convert constexpr char array to buffer type for value type
  template <size_t N>
  constexpr ValueType::buffer_type name_to_buffer(char const (&name)[N]) {
    ValueType::buffer_type tmp{};
    // if you want to expand maximum length of type name,
    // change ValueType::max_name_size and re-compile everything.
    static_assert(N <= tmp.size(), "Name of value type is too long.");
    for (size_t i = 0; i < N; ++i) {
      tmp[i] = name[i];
    }
    tmp.back() = 0;
    return tmp;
  }

  /// aligned buffer
  template <class T>
  alignas(32) constexpr const auto vt_name =
    name_to_buffer(object_type_traits<tag_of_t<typename T::term>>::name);

  template <class T>
  const Type value_type<T>::type{static_construct, ValueType{&vt_name<T>}};

  template <class T>
  const Type vartype<T>::type{static_construct,
                              VarType{uint64_t{std::uintptr_t(T::id)}}};

  template <class T, class... Ts>
  const Type arrow_type_impl<T, Ts...>::type{
    static_construct_t{},
    ArrowType{object_type<T>::type, &arrow_type_impl<Ts...>::type}};

  template <class T1, class T2>
  const Type arrow_type_impl<T1, T2>::type{
    static_construct_t{},
    ArrowType{object_type<T1>::type, object_type<T2>::type}};

} // namespace TORI_NS::detail