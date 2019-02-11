// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "box.hpp"
#include "type_value.hpp"

#include <cstring>
#include <string>
#include <memory>
#include <vector>
#include <utility>
#include <exception>
#include <array>

namespace TORI_NS {

  /// object type traits
  template <class T>
  struct object_type_traits;

} // namespace TORI_NS

// define system type names
#define TORI_DECL_TYPE(TYPE)                        \
  template <>                                       \
  struct TORI_NS::object_type_traits<TORI_NS::TYPE> \
  {                                                 \
    static constexpr char name[] = "_" #TYPE;       \
  };

/// type
TORI_DECL_TYPE(Type)
/// Object
TORI_DECL_TYPE(Object)

namespace TORI_NS::detail {

  namespace interface {

    /// proxy type of closure
    template <class... Ts>
    struct closure : Object
    {
      /// term
      static constexpr auto term = make_tm_closure(Ts::term...);
    };

    /// Type variable value
    template <class Tag>
    struct forall : Object
    {
      // term
      static constexpr auto term = type_c<tm_varvalue<Tag>>;
    };

  } // namespace interface

  // ------------------------------------------
  // utility

  template <class... Ts, class T>
  constexpr auto append(meta_type<T>, meta_type<closure<Ts...>>)
  {
    return type_c<closure<Ts..., T>>;
  }

  template <class Tag>
  constexpr auto make_forall(meta_type<Tag>)
  {
    return type_c<forall<Tag>>;
  }

  // ------------------------------------------
  // expected

  /// expected
  template <class T>
  struct expected : Object
  {
    // term
    static constexpr auto term = T::term;
  };

  // ------------------------------------------
  // value type

  /// value type
  template <class T>
  struct value_type_initializer
  {
    static const Type type;
  };

  template <class T>
  constexpr auto value_type_address(meta_type<T>)
  {
    return &value_type_initializer<T>::type;
  }

  /// convert constexpr char array to buffer type for value type
  template <uint64_t N>
  constexpr ValueType::buffer_type create_value_type_name(char const (&name)[N])
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
  alignas(32) inline constexpr const
    auto value_type_name = create_value_type_name(
      object_type_traits<typename decltype(type_c<T>.tag())::type>::name);

  template <class T>
  const Type value_type_initializer<T>::type {static_construct,
                                              ValueType {&value_type_name<T>}};

  // ------------------------------------------
  // arrow type

  template <class T, class... Ts>
  struct arrow_type_initializer
  {
    static const Type type;
  };

  template <class T1, class T2>
  struct arrow_type_initializer<T1, T2>
  {
    static const Type type;
  };

  template <class... Ts>
  constexpr auto arrow_type_address(meta_type<tm_closure<Ts...>>)
  {
    return &arrow_type_initializer<Ts...>::type;
  }

  template <class T, class... Ts>
  const Type arrow_type_initializer<T, Ts...>::type {
    static_construct,
    ArrowType {object_type_impl(type_c<T>),
               &arrow_type_initializer<Ts...>::type}};

  template <class T1, class T2>
  const Type arrow_type_initializer<T1, T2>::type {
    static_construct,
    ArrowType {object_type_impl(type_c<T1>), object_type_impl(type_c<T2>)}};

  // ------------------------------------------
  // var type

  /// var type
  template <class T>
  struct var_type_initializer
  {
    static const Type type;
    // to make distinct address for each tag
    static constexpr const int _id_gen = 42;
    /// Id
    static constexpr const void* const id = &_id_gen;
  };

  template <class T>
  constexpr auto var_type_address(meta_type<T>)
  {
    return &var_type_initializer<T>::type;
  }

  template <class T>
  const Type var_type_initializer<T>::type {
    static_construct,
    VarType {uint64_t {std::uintptr_t(id)}}};

  // ------------------------------------------
  // constexpr version of object_type type

  template <class T>
  constexpr auto object_type_impl(meta_type<T> term)
  {
    if constexpr (is_tm_value(term)) {
      return value_type_address(term);
    } else if constexpr (is_tm_closure(term)) {
      return arrow_type_address(term);
    } else if constexpr (is_tm_var(term)) {
      return var_type_address(term);
    } else if constexpr (is_tm_varvalue(term)) {
      return var_type_address(term);
    } else if constexpr (is_tm_fix(term)) {
      return value_type_address(term);
    } else {
      static_assert(false_v<T>);
    }
  }

  namespace interface {

    /// object type generator
    template <class T>
    [[nodiscard]] object_ptr<const Type> object_type()
    {
      if constexpr (is_transfarable_immediate(type_c<T>))
        return value_type_address(type_c<tm_value<T>>);
      else
        return object_type_impl(T::term);
    }

  } // namespace interface


  // ------------------------------------------
  // assume_object_type

  template <class T, class... Ts>
  constexpr auto guess_object_type_closure(
    meta_type<T> type,
    meta_type<closure<Ts...>> result)
  {
    if constexpr (is_arrow_type(type)) {
      return guess_object_type_closure(
        type.t2(), append(guess_object_type(type.t1()), result));
    } else {
      return append(guess_object_type(type), result);
    }
  }

  /// Guess C++ type of a type.
  /// Unknown types will be converted into `Object` equivalents.
  template <class T>
  constexpr auto guess_object_type(meta_type<T> type)
  {
    if constexpr (is_arrow_type(type)) {
      return guess_object_type_closure(type, type_c<closure<>>);
    } else if constexpr (is_value_type(type)) {
      return type.tag();
    } else if constexpr (is_var_type(type)) {
      return type_c<Object>;
    } else if constexpr (is_varvalue_type(type)) {
      return make_forall(type.tag());
    } else
      static_assert(false_v<T>, "Invalid type");
  }

} // namespace TORI_NS::detail