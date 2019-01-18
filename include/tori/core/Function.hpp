#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file Function

#include "DynamicTypeUtil.hpp"
#include "ValueCast.hpp"
#include "ClosureCast.hpp"
#include "Fix.hpp"
#include "Exception.hpp"
#include "Apply.hpp"
#include "String.hpp"

#include <type_traits>

/// Size of additional space in closure header
#if defined(CLOSURE_HEADER_EXTEND_BYTES)
namespace TORI_NS::detail {
  constexpr size_t closure_header_extend_bytes = CLOSURE_HEADER_EXTEND_BYTES;
}
#else
namespace TORI_NS::detail {
  constexpr size_t closure_header_extend_bytes = 0;
}
#endif

namespace TORI_NS::detail {

  // ------------------------------------------
  // Closure

  // forward decl
  template <std::size_t N>
  struct ClosureN;

  template <class Closure1 = ClosureN<1>>
  struct Closure;

  /// Info table for closure
  struct closure_info_table : object_info_table
  {
    /// Number of arguments
    size_t n_args;
    /// Size of extended header
    size_t clsr_ext_bytes;
    /// vtable for code
    object_ptr<> (*code)(Closure<>*);
  };

  template <class Closure1>
  struct Closure : HeapObject
  {
    /// Arity of this closure
    atomic_refcount<uint64_t> _arity;

#if defined(CLOSURE_HEADER_EXTEND_BYTES)
    /// additional buffer storage
    std::byte clsr_ext_buffer[closure_header_extend_bytes] = {};
#endif

    /// Get number of args
    size_t n_args() const noexcept
    {
      return static_cast<const closure_info_table*>(info_table)->n_args;
    }

    /// Execute core with vtable function
    object_ptr<> code() noexcept
    {
      return static_cast<const closure_info_table*>(info_table)->code(this);
    }

    /// get nth argument
    object_ptr<>& arg(size_t n)
    {
      constexpr size_t offset = offset_of_member(&Closure1::_args);
      static_assert(offset % sizeof(object_ptr<>) == 0);
      return ((object_ptr<>*)this)[offset / sizeof(object_ptr<>) + n];
    }

    ///  get arity
    atomic_refcount<uint64_t>& arity()
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
  template <std::size_t N>
  struct ClosureN : Closure<>
  {
    /// get raw arg
    template <size_t Arg>
    object_ptr<>& nth_arg() noexcept
    {
      static_assert(Arg < N, "Invalid index of argument");
      return _args[N - Arg - 1];
    }

    /// get raw arg
    template <size_t Arg>
    const object_ptr<>& nth_arg() const noexcept
    {
      static_assert(Arg < N, "Invalid index of argument");
      return _args[N - Arg - 1];
    }

    /// args
    std::array<object_ptr<>, N> _args = {};
  };

  // ------------------------------------------
  // Eval wrapper

  /// wrapper for main code of closure
  template <class T>
  struct vtbl_eval_wrapper
  {
    static object_ptr<> code(Closure<>* _this) noexcept
    {
      try {
        return (static_cast<const T*>(_this)->code()).value();
      } catch (const bad_value_cast& e) {
        return new Exception(new BadValueCast(e.from(), e.to()));
      } catch (const bad_closure_cast& e) {
        return new Exception(new BadClosureCast(e.from(), e.to()));
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
  // remove_last

  template <class T>
  struct remove_last
  {
  };

  template <class T>
  struct remove_last<std::tuple<T>>
  {
    using type = std::tuple<>;
  };

  template <class T, class... Ts>
  struct remove_last<std::tuple<T, Ts...>>
  {
    using type = concat_tuple_t<
      std::tuple<T>,
      typename remove_last<std::tuple<Ts...>>::type>;
  };

  /// remove last element from tuple
  template <class T>
  using remove_last_t = typename remove_last<T>::type;

  // ------------------------------------------
  // remove varvalue

  template <class T, class Target>
  struct remove_varvalue_impl
  {
    using type = Target;
  };

  template <class Tag, class Target>
  struct remove_varvalue_impl<tm_varvalue<Tag>, Target>
  {
    using _var = tm_var<Tag>;
    using type = subst_term_t<tm_varvalue<Tag>, _var, Target>;
  };

  template <class T, class... Ts, class Target>
  struct remove_varvalue_impl<tm_closure<T, Ts...>, Target>
  {
    using t = remove_varvalue_impl<T, Target>;
    using type =
      typename remove_varvalue_impl<tm_closure<Ts...>, typename t::type>::type;
  };

  template <class T, class Target>
  struct remove_varvalue_impl<tm_closure<T>, Target>
  {
    using type = typename remove_varvalue_impl<T, Target>::type; //
  };

  template <class T1, class T2, class Target>
  struct remove_varvalue_impl<tm_apply<T1, T2>, Target>
  {
    using t1 = remove_varvalue_impl<T1, Target>;
    using t2 = remove_varvalue_impl<T2, typename t1::type>;
    using type = typename t2::type;
  };

  /// replace tm_varvalue<Tag> to tm_var<Tag>
  template <class Term>
  using remove_varvalue_t = typename remove_varvalue_impl<Term, Term>::type;

  // ------------------------------------------
  // return type checking

  template <class T1, class T2, bool B = std::is_same_v<T1, T2>>
  struct check_return_type
  {
  };

  template <class T1, class T2>
  struct check_return_type<T1, T2, false>
  {
    using t1 = typename T1::_expected;
    using t2 = typename T2::_provided;
    static_assert(false_v<T1>, "return type does not match");
  };

  /// Return type checker
  template <class Term>
  class return_type_checker
  {
  public:
    using return_type = type_of_t<Term>;

    /// object_ptr<U>&&
    template <class U>
    return_type_checker(object_ptr<U> obj)
      : m_value {std::move(obj)}
    {
      // check return type
      [[maybe_unused]] check_return_type<
        return_type,
        type_of_t<typename U::term>>
        check {};
    }

    /// U*
    template <class U>
    return_type_checker(U* ptr)
      : return_type_checker(object_ptr(ptr))
    {
    }

    /// deleted
    return_type_checker() = delete;
    /// deleted
    return_type_checker(const return_type& other) = delete;
    /// deleted
    return_type_checker(return_type&& other) = delete;

    /// value
    object_ptr<> value() &&
    {
      return std::move(m_value);
    }

  private:
    object_ptr<> m_value;
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
      using term = remove_varvalue_t<tm_closure<typename Ts::term...>>;

      /// Closure info table initializer
      struct info_table_initializer
      {
        /// static closure infor
        static const closure_info_table info_table;
      };

      /// Get N'th argument
      template <size_t N>
      auto arg() const
      {
        using To = std::tuple_element_t<N, std::tuple<Ts...>>;
        auto obj = ClosureN<sizeof...(Ts) - 1>::template nth_arg<N>();
        obj.head()->refcount.fetch_add(); // +1
        return object_ptr(static_cast<expected<To>*>(obj.get()));
      }

      /// Evaluate N'th argument and take result
      template <size_t N>
      auto eval_arg() const
      {
        return eval(arg<N>());
      }

      /// Ctor
      constexpr Function() noexcept
        : ClosureN<sizeof...(Ts) - 1> {
            {{1,
              static_cast<const object_info_table*>(
                &info_table_initializer::info_table)},
             sizeof...(Ts) - 1}}
      {
      }

    protected:
      // return type for code()
      using return_type = return_type_checker<typename std::tuple_element_t<
        sizeof...(Ts) - 1,
        std::tuple<Ts...>>::term>;

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
