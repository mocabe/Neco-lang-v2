// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

/// \file BinaryOperator

#include "../core.hpp"
#include <functional>

namespace TORI_NS::detail {

  /// Binary operator function
  template <class T, class R, template <class> class E>
  struct BinaryOperator : Function<BinaryOperator<T, R, E>, T, T, R>
  {
    typename BinaryOperator::return_type code() const
    {
      // assume BoxedHeapObject
      using Tp = typename T::value_type;
      E<Tp> op;
      auto lhs = this->template eval_arg<0>();
      auto rhs = this->template eval_arg<1>();
      return new R {op(*lhs, *rhs)};
    }
  };

  /// T -> T -> Bool
  template <class T, template <class> class E>
  using CompOp = BinaryOperator<T, Bool, E>;

  /// T -> T -> T
  template <class T, template <class> class E>
  using ArithOp = BinaryOperator<T, T, E>;

  namespace interface {

    // Arithmetic operators

    template <class T>
    using Plus = ArithOp<T, std::plus>;
    template <class T>
    using Minus = ArithOp<T, std::minus>;
    template <class T>
    using Multiples = ArithOp<T, std::multiplies>;
    template <class T>
    using Divides = ArithOp<T, std::divides>;
    template <class T>
    using Modulus = ArithOp<T, std::modulus>;
    template <class T>
    using Negate = ArithOp<T, std::negate>;

    // Comparison operators

    template <class T>
    using EqualTo = CompOp<T, std::equal_to>;
    template <class T>
    using NotEqualTo = CompOp<T, std::not_equal_to>;
    template <class T>
    using Greater = CompOp<T, std::greater>;
    template <class T>
    using Less = CompOp<T, std::less>;
    template <class T>
    using GreaterEqual = CompOp<T, std::greater_equal>;
    template <class T>
    using LessEqual = CompOp<T, std::less_equal>;

    // Logical operators

    template <class T>
    using LogicalAnd = CompOp<T, std::logical_and>;
    template <class T>
    using LogicalOr = CompOp<T, std::logical_or>;
    template <class T>
    using LogicalNot = CompOp<T, std::logical_not>;

    // Bitwise operators

    template <class T>
    using BitAnd = ArithOp<T, std::bit_and>;
    template <class T>
    using BitOr = ArithOp<T, std::bit_or>;
    template <class T>
    using BitXor = ArithOp<T, std::bit_xor>;

  } // namespace interface

// define primitive operators
#define TORI_DECL_BINARY_OP(TYPE)                \
  using Plus##TYPE = Plus<TYPE>;                 \
  using Minus##TYPE = Minus<TYPE>;               \
  using Multiples##TYPE = Multiples<TYPE>;       \
  using Divides##TYPE = Divides<TYPE>;           \
  using Modulus##TYPE = Modulus<TYPE>;           \
  using Negate##TYPE = Negate<TYPE>;             \
  using EqualTo##TYPE = EqualTo<TYPE>;           \
  using NotEqualTo##TYPE = NotEqualTo<TYPE>;     \
  using Greater##TYPE = Greater<TYPE>;           \
  using Less##TYPE = Less<TYPE>;                 \
  using GreaterEqual##TYPE = GreaterEqual<TYPE>; \
  using LessEqual##TYPE = LessEqual<TYPE>;       \
  using LogicalAnd##TYPE = LogicalAnd<TYPE>;     \
  using LogicalOr##TYPE = LogicalOr<TYPE>;       \
  using LogicalNot##TYPE = LogicalNot<TYPE>;     \
  using BitAnd##TYPE = BitAnd<TYPE>;             \
  using BitOr##TYPE = BitOr<TYPE>;               \
  using BitXor##TYPE = BitXor<TYPE>;

  namespace interface {

    TORI_DECL_BINARY_OP(Int8)
    TORI_DECL_BINARY_OP(Int16)
    TORI_DECL_BINARY_OP(Int32)
    TORI_DECL_BINARY_OP(Int64)

    TORI_DECL_BINARY_OP(Char)
    TORI_DECL_BINARY_OP(Short)
    TORI_DECL_BINARY_OP(Int)
    TORI_DECL_BINARY_OP(Long)

    TORI_DECL_BINARY_OP(UInt8)
    TORI_DECL_BINARY_OP(UInt16)
    TORI_DECL_BINARY_OP(UInt32)
    TORI_DECL_BINARY_OP(UInt64)

    TORI_DECL_BINARY_OP(UChar)
    TORI_DECL_BINARY_OP(UShort)
    TORI_DECL_BINARY_OP(UInt)
    TORI_DECL_BINARY_OP(ULong)

    TORI_DECL_BINARY_OP(Float)
    TORI_DECL_BINARY_OP(Double)

    TORI_DECL_BINARY_OP(Bool)

  } // namespace interface

#undef TORI_DECL_BINARY_OP

} // namespace TORI_NS::detail