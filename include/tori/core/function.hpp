// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "fix.hpp"
#include "exception.hpp"
#include "apply.hpp"
#include "string.hpp"
#include "static_typing.hpp"
#include "dynamic_typing.hpp"
#include "value_cast.hpp"
#include "offset_of_member.hpp"

#include <type_traits>

/// Size of additional space in closure header
#if defined(CLOSURE_HEADER_EXTEND_BYTES)
namespace TORI_NS::detail {
  constexpr uint64_t closure_header_extend_bytes = CLOSURE_HEADER_EXTEND_BYTES;
}
#else
namespace TORI_NS::detail {
  constexpr uint64_t closure_header_extend_bytes = 0;
}
#endif

namespace TORI_NS::detail {

  // ------------------------------------------
  // Closure

  // forward decl
  template <uint64_t N>
  struct ClosureN;

  template <class Closure1 = ClosureN<1>>
  struct Closure;

  /// Info table for closure
  struct closure_info_table : object_info_table
  {
    /// Number of arguments
    const uint64_t n_args;
    /// Size of extended header
    const uint64_t clsr_ext_bytes;
    /// vtable for code
    object_ptr_generic (*code)(const Closure<>*);
  };

  template <class Closure1>
  struct Closure : Object
  {
    /// Arity of this closure.
    /// mutable for evaluation
    mutable uint64_t _arity;

#if defined(CLOSURE_HEADER_EXTEND_BYTES)
    /// additional buffer storage
    std::byte clsr_ext_buffer[closure_header_extend_bytes] = {};
#endif

    /// Get number of args
    uint64_t n_args() const noexcept
    {
      return static_cast<const closure_info_table*>(info_table)->n_args;
    }

    /// Execute core with vtable function
    auto code() const noexcept
    {
      return static_cast<const closure_info_table*>(info_table)->code(this);
    }

    /// get nth argument
    object_ptr_generic& arg(uint64_t n) const noexcept
    {
      constexpr uint64_t offset = offset_of_member(&Closure1::_args);
      static_assert(offset % sizeof(object_ptr_generic) == 0);
      return (
        (object_ptr_generic*)this)[offset / sizeof(object_ptr_generic) + n];
    }

    ///  get arity
    uint64_t& arity() const noexcept
    {
      return _arity;
    }
  };

  /** \brief ClosureN
   * Contains static size array for incoming arguments.
   * Arguments will be filled from back to front.
   * So, first argument of closure will be LAST element of array for arguments.
   * If all arguments are passed, arity becomes zero and the closure is ready to
   * execute code.
   */
  template <uint64_t N>
  struct ClosureN : Closure<>
  {
    /// get raw arg
    template <uint64_t Arg>
    auto& nth_arg() const noexcept
    {
      static_assert(Arg < N, "Invalid index of argument");
      return _args[N - Arg - 1];
    }

    /// args
    /// mutable for evaluation
    mutable std::array<object_ptr_generic, N> _args = {};
  };

  // ------------------------------------------
  // Eval wrapper

  /// wrapper for main code of closure
  template <class T>
  struct vtbl_eval_wrapper
  {
    static object_ptr_generic code(const Closure<>* _this) noexcept
    {
      try {
        auto r = (static_cast<const T*>(_this)->code()).value();
        assert(r);
        return r;
      } catch (const bad_value_cast& e) {
        return new Exception(new BadValueCast(e.from(), e.to()));
      } catch (const type_error::type_error& e) {
        return new Exception(new TypeError(new String(e.what()), e.src()));
      } catch (const eval_error::eval_error& e) {
        return new Exception(new EvalError(new String(e.what()), e.src()));
      } catch (const result_error::result_error& e) {
        return object_ptr<>(e.result());
      } catch (const std::exception& e) {
        return new Exception(new String(e.what()));
      } catch (...) {
        return new Exception(
          new String("Unknown exception thrown while evaluation"));
      }
    }
  };

  // ------------------------------------------
  // remove varvalue

  template <class Term, class Target>
  constexpr auto
    remove_varvalue_impl(meta_type<Term> term, meta_type<Target> target)
  {
    if constexpr (is_tm_varvalue(term)) {
      return subst_term(term, make_tm_var(term.tag()), target);
    } else if constexpr (is_tm_closure(term)) {
      if constexpr (term.size() == 1) {
        return remove_varvalue_impl(head(term), target);
      } else {
        return remove_varvalue_impl(
          tail(term), remove_varvalue_impl(head(term), target));
      }
    } else if constexpr (is_tm_apply(term)) {
      return remove_varvalue_impl(
        term.t2(), remove_varvalue_impl(term.t1(), target));
    } else
      return target;
  }

  template <class Term>
  constexpr auto remove_varvalue(meta_type<Term> term)
  {
    return remove_varvalue_impl(term, term);
  }

  // ------------------------------------------
  // return type checking

  template <class T1, class T2>
  constexpr auto check_return_type(meta_type<T1> t1, meta_type<T2> t2)
  {
    if constexpr (t1 != t2) {
      static_assert(false_v<T1>, "return type does not match.");
      using _t1 = typename T1::_expected;
      using _t2 = typename T2::_provided;
      static_assert(false_v<_t1, _t2>, "compile-time type check failed.");
    }
  }

  /// Return type checker
  template <class Term>
  class return_type_checker
  {
  public:
    static constexpr auto return_type = type_of(type_c<Term>);

    /// object_ptr<U>&&
    template <class U>
    return_type_checker(object_ptr<U> obj)
      : m_value {std::move(obj)}
    {
      // check return type
      check_return_type(return_type, type_of(get_term<U>()));
    }

    /// immediate<U>
    template <class U>
    return_type_checker(immediate<U> imm)
      : m_value {std::move(imm)}
    {
      // check return type
      check_return_type(return_type, type_of(get_term<U>()));
    }

    /// U*
    template <class U>
    return_type_checker(U* ptr)
      : m_value(object_ptr(ptr))
    {
      // check return type
      check_return_type(return_type, type_of(get_term<U>()));
    }

    /// U
    template <class U>
    return_type_checker(U value)
      : m_value(immediate(value))
    {
      // check return type
      check_return_type(return_type, type_of(get_term<U>()));
    }

    /// deleted
    return_type_checker() = delete;
    /// deleted
    return_type_checker(const return_type_checker& other) = delete;
    /// deleted
    return_type_checker(return_type_checker&& other) = delete;

    /// value
    object_ptr_generic&& value() &&
    {
      return std::move(m_value);
    }

  private:
    object_ptr_generic m_value;
  };

  // ------------------------------------------
  // Function

  namespace interface {

    /// CRTP utility to create closure type.
    template <class T, class... Ts>
    struct Function : ClosureN<sizeof...(Ts) - 1>
    {
      static_assert(
        sizeof...(Ts) > 1,
        "Closure should have argument and return type");

      /// export term
      static constexpr auto term =
        remove_varvalue(make_tm_closure(get_term<Ts>()...));

      /// Closure info table initializer
      struct info_table_initializer
      {
        /// static closure infor
        static const closure_info_table info_table;
      };

      /// Get N'th argument
      template <uint64_t N>
      auto arg() const
      {
        using To = std::tuple_element_t<N, std::tuple<Ts...>>;
        auto obj = ClosureN<sizeof...(Ts) - 1>::template nth_arg<N>();
        // convert argument without type check.
        // runtime type system should enforce that argumet type is valid.
        return static_auto_cast<To>(obj);
      }

      /// Evaluate N'th argument and take result
      template <uint64_t N>
      auto eval_arg() const
      {
        // workaround: gcc 8.1
        return eval(this->template arg<N>());
        // TODO: Replace argument with result.
      }

      /// Ctor
      Function() noexcept
        : ClosureN<sizeof...(Ts) - 1> {
            {{1u,
              static_cast<const object_info_table*>(
                &info_table_initializer::info_table)},
             sizeof...(Ts) - 1}}
      {
      }

      /// Copy ctor
      Function(const Function& other) noexcept
        : ClosureN<sizeof...(Ts) - 1> {
            {
              {1u,
               static_cast<const object_info_table*>(
                 &info_table_initializer::info_table)},
              other._arity,
            },
            other._args}
      {
      }

    protected:
      // return type for code()
      using return_type = return_type_checker<typename decltype(
        get_term(get<sizeof...(Ts) - 1>(tuple_c<Ts...>)))::type>;

    private:
      using ClosureN<sizeof...(Ts) - 1>::nth_arg;
      using ClosureN<sizeof...(Ts) - 1>::arg;

      template <auto FP>
      struct concept_checker
      {
      };

      /// check signature of code()
      void check_code()
      {
        static_assert(
          std::is_same_v<return_type, decltype(std::declval<const T>().code())>,
          " `return_type code() const` was not found.");
      }
      using concept_check_code = concept_checker<&Function::check_code>;
    };

    // Initialize closure infotable
    template <class T, class... Ts>
    const closure_info_table
      Function<T, Ts...>::info_table_initializer::info_table = {
        {object_type<T>(),
         sizeof(T),
         object_header_extend_bytes,
         vtbl_destroy_func<T>,
         vtbl_clone_func<T>},
        sizeof...(Ts) - 1,
        closure_header_extend_bytes,
        vtbl_eval_wrapper<T>::code};

  } // namespace interface

} // namespace TORI_NS::detail
