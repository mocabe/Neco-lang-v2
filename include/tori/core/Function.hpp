// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

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
  // Offset of member
  // ------------------------------------------

  template <typename T1, typename T2>
  struct offset_of_member_impl {
    union U {
      U() : c{} {}
      ~U() {}
      char c[sizeof(T2)];
      T2 o;
    };
    static U u;
    static constexpr size_t get(T1 T2::*member) {
      size_t i = 0;
      for (; i < sizeof(T2); ++i)
        if (((void*)&(u.c[i])) == &(u.o.*member)) break;

      // g++ bug 67371 workaround
      if (i >= sizeof(T2))
        throw;
      else
        return i;
    }
  };
  template <class T1, class T2>
  typename offset_of_member_impl<T1, T2>::U offset_of_member_impl<T1, T2>::u{};

  /// get offset of member
  template <class T1, class T2>
  constexpr size_t offset_of_member(T1 T2::*member) {
    return offset_of_member_impl<T1, T2>::get(member);
  }

  // ------------------------------------------
  // Closure
  // ------------------------------------------

  // forward decl
  template <std::size_t N>
  struct ClosureN;

  template <class Closure1 = ClosureN<1>>
  struct Closure;

  /// Info table for closure
  struct ClosureInfoTable : ObjectInfoTable {
    /// Number of arguments
    size_t n_args;
    /// Size of extended header
    size_t clsr_ext_bytes;
    /// vtable for code
    ObjectPtr<> (*code)(Closure<>*);
  };

  template <class Closure1>
  struct Closure : HeapObject {
    /// Arity of this object.
    struct _arity {
      constexpr _arity() noexcept : atomic{0} {}
      constexpr _arity(size_t v) noexcept : atomic{v} {}
      constexpr _arity(const _arity& arty) noexcept : atomic{arty.raw} {}
      union {
        std::atomic<size_t> atomic;
        size_t raw;
      };
    } arity;

#if defined(CLOSURE_HEADER_EXTEND_BYTES)
    /// additional buffer storage
    std::byte clsr_ext_buffer[closure_header_extend_bytes] = {};
#endif

    /// get nth argument
    ObjectPtr<>& args(size_t n) {
      constexpr size_t offset = offset_of_member(&Closure1::args);
      static_assert(offset % sizeof(ObjectPtr<>) == 0);
      return ((ObjectPtr<>*)this)[offset / sizeof(ObjectPtr<>) + n];
    }

    /// get nth argument
    const ObjectPtr<>& args(size_t n) const {
      constexpr size_t offset = offset_of_member(&Closure1::args);
      static_assert(offset % sizeof(ObjectPtr<>) == 0);
      return ((ObjectPtr<>*)this)[offset / sizeof(ObjectPtr<>) + n];
    }

    /// Execute core with vtable function
    ObjectPtr<> code() noexcept {
      return static_cast<const ClosureInfoTable*>(info_table)->code(this);
    }

    /// Get number of args
    size_t n_args() const noexcept {
      return static_cast<const ClosureInfoTable*>(info_table)->n_args;
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
  struct ClosureN : Closure<> {
    ObjectPtr<> args[N] = {};
    /// get raw arg
    template <size_t Arg>
    ObjectPtr<>& nth_arg() noexcept {
      static_assert(Arg < N, "Invalid index of argument");
      return args[N - Arg - 1];
    }
    /// get raw arg
    template <size_t Arg>
    const ObjectPtr<>& nth_arg() const noexcept {
      static_assert(Arg < N, "Invalid index of argument");
      return args[N - Arg - 1];
    }
  };

  // ------------------------------------------
  // Eval wrapper
  // ------------------------------------------

  /// wrapper for main code of closure
  template <class T>
  struct vtbl_eval_wrapper {
    static ObjectPtr<> code(Closure<>* _this) noexcept {
      try {
        return std::move((static_cast<const T*>(_this)->code()).value);
      } catch (const bad_value_cast& e) {
        return new Exception(new BadValueCast(e.from(), e.to()));
      } catch (const bad_closure_cast& e) {
        return new Exception(new BadClosureCast(e.from(), e.to()));
      } catch (type_error& e) {
        return new Exception(new TypeError(new String(e.what()), e.src()));
      } catch (eval_error& e) {
        return new Exception(new EvalError(new String(e.what()), e.src()));
      } catch (result_error& e) {
        return ObjectPtr<>(e.result());
      } catch (const std::exception& e) {
        return new Exception(new String(e.what()));
      } catch (...) { return new Exception(new String("Unknown exception")); }
    }
  };

  // ------------------------------------------
  // remove_last
  // ------------------------------------------
  template <class T>
  struct remove_last {};
  template <class T>
  struct remove_last<std::tuple<T>> {
    using type = std::tuple<>;
  };
  template <class T, class... Ts>
  struct remove_last<std::tuple<T, Ts...>> {
    using type = concat_tuple_t<
      std::tuple<T>,
      typename remove_last<std::tuple<Ts...>>::type>;
  };
  template <class T>
  using remove_last_t = typename remove_last<T>::type;

  // ------------------------------------------
  // remove varvalue
  // ------------------------------------------

  template <class T, class Target>
  struct remove_varvalue_ {
    using type = Target;
  };

  // replace TmVarValue<Tag> to TmVar<Tag>
  template <class Tag, class Target>
  struct remove_varvalue_<TmVarValue<Tag>, Target> {
    using _var = TmVar<Tag>;
    using type = subst_term_t<TmVarValue<Tag>, _var, Target>;
  };

  template <class T, class... Ts, class Target>
  struct remove_varvalue_<TmClosure<T, Ts...>, Target> {
    using t = remove_varvalue_<T, Target>;
    using type =
      typename remove_varvalue_<TmClosure<Ts...>, typename t::type>::type;
  };

  template <class T, class Target>
  struct remove_varvalue_<TmClosure<T>, Target> {
    using type = typename remove_varvalue_<T, Target>::type;
  };
  template <class T1, class T2, class Target>
  struct remove_varvalue_<TmApply<T1,T2>, Target> {
    using t1 = remove_varvalue_<T1, Target>;
    using t2 = remove_varvalue_<T2, typename t1::type>;
    using type = typename t2::type;
  };

  template <class Term>
  using remove_varvalue_t = typename remove_varvalue_<Term, Term>::type;

  // ------------------------------------------
  // Function
  // ------------------------------------------
  namespace interface {
    template <class T, class... Ts>
    struct Function : ClosureN<sizeof...(Ts) - 1> {
      static_assert(
        sizeof...(Ts) > 1, "Closure should have argument and return type");

      /// export term
      using term = remove_varvalue_t<TmClosure<typename Ts::term...>>;

      /// Closure info table initializer
      struct info_table_initializer {
        /// static closure infor
        static const ClosureInfoTable info_table;
      };

      /// Get N'th argument
      template <size_t N>
      auto arg() const {
        using To = std::tuple_element_t<N, std::tuple<Ts...>>;
        auto obj = ClosureN<sizeof...(Ts) - 1>::template nth_arg<N>();
        ++(obj.head()->refcount.atomic); // +1
        return ObjectPtr(static_cast<expected<To>*>(obj.head()));
      }

      /// Evaluate N'th argument and take result
      template <size_t N>
      auto eval_arg() const {
        return eval(arg<N>());
      }

      /// Ctor
      constexpr Function() noexcept
        : ClosureN<sizeof...(Ts) - 1>{
            {{1, static_cast<const ObjectInfoTable*>(
                   &info_table_initializer::info_table)},
             sizeof...(Ts) - 1}} {}

    private:
      template <class T1, class T2, bool B>
      struct check_return_type_;

      template <class T1, class T2>
      struct check_return_type {
        // clang bug(?) workaround
        using type = check_return_type_<T1, T2, std::is_same_v<T1,T2>>;
      };

      template <class T1, class T2>
      struct check_return_type_<T1, T2, false> {
        using t1 = typename T1::_expected;
        using t2 = typename T2::_provided;
        static_assert(false_v<T1>, "return type does not match");
      };
      template <class T1, class T2>
      struct check_return_type_<T1, T2, true> {};

    protected:
      /// Return type checker
      struct ReturnType {
        using return_term = typename std::
          tuple_element_t<sizeof...(Ts) - 1, std::tuple<Ts...>>::term;
        using return_type = type_of_t<return_term>;
        template <class U>
        ReturnType(const ObjectPtr<U>& obj) : value{ObjectPtr<>(obj)} {
          // check return type
          ignore(check_return_type<return_type, type_of_t<typename U::term>>{});
        }

        template <class U>
        ReturnType(ObjectPtr<U>&& obj) : value{ObjectPtr<>(std::move(obj))} {
          // check return type
          ignore(check_return_type<return_type, type_of_t<typename U::term>>{});
        }

        template <class U>
        ReturnType(U* ptr) : ReturnType(ObjectPtr(ptr)) {}

        ReturnType() = delete;
        ReturnType(const ReturnType& other) : value{other.value} {}
        ReturnType(ReturnType&& other) : value{std::move(other.value)} {}

        const ObjectPtr<> value;
      };

    private:
      using ClosureN<sizeof...(Ts) - 1>::nth_arg;
      using ClosureN<sizeof...(Ts) - 1>::args;

      template <auto P>
      struct concept_checker {};

      void check_code() {
        static_assert(
          std::is_same_v<ReturnType, decltype(std::declval<const T>().code())>,
          " `ReturnType code() const` was not found.");
      }
      using concept_check_code = concept_checker<&Function::check_code>;
    };
    // Initialize closure infotable
    template <class T, class... Ts>
    const ClosureInfoTable
      Function<T, Ts...>::info_table_initializer::info_table = { //
        {object_type<T>(),                                  //
         sizeof(T),                                              //
         object_header_extend_bytes,                             //
         vtbl_destroy_func<T>, vtbl_clone_func<T>},              //
        sizeof...(Ts) - 1,                                       //
        closure_header_extend_bytes,                             //
        vtbl_eval_wrapper<T>::code};

  } // namespace interface

} // namespace TORI_NS::detail
