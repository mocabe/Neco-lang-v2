#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file Compile time type info generator

#include "BoxedHeapObject.hpp"
#include "TypeValue.hpp"
#include <cstring>
#include <string>
#include <memory>
#include <vector>
#include <utility>
#include <exception>
#include <array>

// decl system types
#define TORI_DECL_TYPE(TYPE)                  \
  template <>                                 \
  struct object_type_traits<TYPE>             \
  {                                           \
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
    struct closure : HeapObject
    {
      /// term
      using term = tm_closure<typename Ts::term...>;
    };

    /// Type variable value
    template <class Tag>
    struct forall : HeapObject
    {
      // term
      using term = tm_varvalue<Tag>;
    };
  } // namespace interface

  // expected
  template <class T>
  struct expected : HeapObject
  {
    // term
    using term = typename T::term;
  };

  // ------------------------------------------
  // value type

  /// value type
  template <class T>
  struct value_type
  {
    static const Type type;
  };

  // ------------------------------------------
  // arrow type

  template <class T, class... Ts>
  struct arrow_type_impl
  {
    static const Type type;
  };

  template <class T1, class T2>
  struct arrow_type_impl<T1, T2>
  {
    static const Type type;
  };

  /// arrow type
  template <class T, class... Ts>
  struct arrow_type;

  template <class... Ts>
  struct arrow_type<tm_closure<Ts...>>
  {
    static constexpr const Type* type = &arrow_type_impl<Ts...>::type;
  };

  // ------------------------------------------
  // var type

  /// var type
  template <class T>
  struct vartype
  {
    static const Type type;
    // to make distinct address for each tag
    static constexpr const int _id_gen = 42;
    /// Id
    static constexpr const void* const id = &_id_gen;
  };

  // ------------------------------------------
  // object_type type

  template <class T, class = void>
  struct object_type_impl
  {
  };

  // value
  template <class T>
  struct object_type_impl<T, std::enable_if_t<is_tm_value_v<T>>>
  {
    static constexpr const Type* type = &value_type<T>::type;
    static object_ptr<const Type> get()
    {
      return type;
    }
  };

  // closure
  template <class T>
  struct object_type_impl<T, std::enable_if_t<is_tm_closure_v<T>>>
  {
    static constexpr const Type* type = arrow_type<T>::type;
    static object_ptr<const Type> get()
    {
      return type;
    }
  };

  // var
  template <class T>
  struct object_type_impl<T, std::enable_if_t<is_tm_var_v<T>>>
  {
    static constexpr const Type* type = &vartype<T>::type;
    static object_ptr<const Type> get()
    {
      return type;
    }
  };

  // var value
  template <class T>
  struct object_type_impl<T, std::enable_if_t<is_tm_varvalue_v<T>>>
  {
    static constexpr const Type* type = &vartype<T>::type;
    static object_ptr<const Type> get()
    {
      return type;
    }
  };

  // fix
  template <class T>
  struct object_type_impl<T, std::enable_if_t<is_tm_fix_v<T>>>
  {
    static constexpr const Type* type = &value_type<T>::type;
    static object_ptr<const Type> get()
    {
      return type;
    }
  };

  namespace interface {

    /// object type generator
    template <class T>
    [[nodiscard]] object_ptr<const Type> object_type()
    {
      return object_type_impl<typename T::term>::get();
    }

  } // namespace interface

  /// convert constexpr char array to buffer type for value type
  template <uint64_t N>
  constexpr ValueType::buffer_type name_to_buffer(char const (&name)[N])
  {
    auto tmp = ValueType::buffer_type {};
    // if you want to expand maximum length of type name,
    // change ValueType::max_name_size and re-compile everything.
    static_assert(N <= tmp.size(), "Name of value type is too long.");
    for (uint64_t i = 0; i < N; ++i) {
      tmp[i] = name[i];
    }
    tmp.back() = 0;
    return tmp;
  }

  /// aligned buffer
  template <class T>
  alignas(32) constexpr const
    auto vt_name = name_to_buffer(object_type_traits<tag_of_t<T>>::name);

  template <class T>
  const Type value_type<T>::type {static_construct, ValueType {&vt_name<T>}};

  template <class T>
  const Type vartype<T>::type {static_construct,
                               VarType {uint64_t {std::uintptr_t(id)}}};

  template <class T, class... Ts>
  const Type arrow_type_impl<T, Ts...>::type {
    static_construct,
    ArrowType {object_type_impl<T>::type, &arrow_type_impl<Ts...>::type}};

  template <class T1, class T2>
  const Type arrow_type_impl<T1, T2>::type {
    static_construct,
    ArrowType {object_type_impl<T1>::type, object_type_impl<T2>::type}};

  // ------------------------------------------
  // assume_object_type

  template <class Arrow, class Closure>
  struct assume_object_type_h
  {
  };

  template <class Ty>
  struct assume_object_type
  {
  };

  template <class T1, class T2, class... Ts>
  struct assume_object_type_h<arrow<T1, T2>, closure<Ts...>>
  {
    using type = typename assume_object_type_h<
      T2,
      closure<Ts..., typename assume_object_type<T1>::type>>::type;
  };

  template <class T, class... Ts>
  struct assume_object_type_h<T, closure<Ts...>>
  {
    using type = closure<Ts..., typename assume_object_type<T>::type>;
  };

  template <class Tag>
  struct assume_object_type<value<Tag>>
  {
    using type = Tag;
  };

  template <class Tag>
  struct assume_object_type<var<Tag>>
  {
    using type = HeapObject;
  };

  template <class Tag>
  struct assume_object_type<varvalue<Tag>>
  {
    using type = forall<Tag>;
  };

  template <class T1, class T2>
  struct assume_object_type<arrow<T1, T2>>
  {
    using type = typename assume_object_type_h<arrow<T1, T2>, closure<>>::type;
  };

  /// assume memory lauout fron type
  template <class T>
  using assume_object_type_t = typename assume_object_type<T>::type;

} // namespace TORI_NS::detail