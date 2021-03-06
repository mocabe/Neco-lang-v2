// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#if !defined(TORI_NO_LOCAL_INCLUDE)
#  include "offset_of_member.hpp"
#  include "static_typing.hpp"
#  include "dynamic_typing.hpp"
#  include "apply.hpp"
#  include "string.hpp"
#  include "exception.hpp"
#  include "eval_error.hpp"
#  include "bad_value_cast.hpp"
#  include "result_error.hpp"
#  include "type_error.hpp"
#  include "value_cast.hpp"
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
    /// vtable for code
    object_ptr<const Object> (*code)(const Closure<>*) noexcept;
  };

  template <class Closure1>
  struct Closure : Object
  {
    /// Arity of this closure.
    mutable uint64_t m_arity;

    /// Get number of args
    auto n_args() const noexcept
    {
      return static_cast<const closure_info_table*>(info_table)->n_args;
    }

    /// Execute core with vtable function
    auto code() const noexcept
    {
      return static_cast<const closure_info_table*>(info_table)->code(this);
    }

    /// get nth argument
    auto& arg(uint64_t n) const noexcept
    {
      using arg_type = typename decltype(Closure1::m_args)::value_type;
      constexpr uint64_t offset = offset_of_member(&Closure1::m_args);
      static_assert(offset % sizeof(arg_type) == 0);
      return ((arg_type*)this)[offset / sizeof(arg_type) + n];
    }

    ///  get arity
    auto& arity() const noexcept
    {
      return m_arity;
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
      return m_args[N - Arg - 1];
    }

    /// args
    mutable std::array<object_ptr<const Object>, N> m_args = {};
  };

  // ------------------------------------------
  // vtbl_code_func

  /// vrtable function to call code()
  template <class T>
  object_ptr<const Object> vtbl_code_func(const Closure<>* _this) noexcept
  {
    auto ret = [&]() -> object_ptr<const Object> {
      try {
        auto r = (static_cast<const T*>(_this)->exception_handler()).value();
        TORI_ASSERT(r);
        return r;

        // bad_value_cast
      } catch (const bad_value_cast& e) {
        return add_exception_tag(to_Exception(e));

        // type_error
      } catch (const type_error::circular_constraint& e) {
        return add_exception_tag(to_Exception(e));
      } catch (const type_error::type_missmatch& e) {
        return add_exception_tag(to_Exception(e));
      } catch (const type_error::bad_type_check& e) {
        return add_exception_tag(to_Exception(e));
      } catch (const type_error::type_error& e) {
        return add_exception_tag(to_Exception(e));

        // result_error
      } catch (const result_error::exception_result& e) {
        return add_exception_tag(to_Exception(e));
      } catch (const result_error::result_error& e) {
        return add_exception_tag(to_Exception(e));

        // eval_error
      } catch (const eval_error::bad_fix& e) {
        return add_exception_tag(to_Exception(e));
      } catch (const eval_error::bad_apply& e) {
        return add_exception_tag(to_Exception(e));
      } catch (const eval_error::too_many_arguments& e) {
        return add_exception_tag(to_Exception(e));
      } catch (const eval_error::eval_error& e) {
        return add_exception_tag(to_Exception(e));

        // std::exception
      } catch (const std::exception& e) {
        return add_exception_tag(to_Exception(e));

        // unknown
      } catch (...) {
        return add_exception_tag(make_object<Exception>(
          make_object<String>("Unknown exception thrown while evaluation"),
          object_ptr(nullptr)));
      }
    }();

    // vtbl_code_func should not return Undefined value
    TORI_ASSERT(ret);
    // exception object retuned fron vtbl_code_func should have pointer tag
    if (!has_exception_tag(ret))
      TORI_ASSERT(!value_cast_if<Exception>(ret));

    return ret;
  }

  // ------------------------------------------
  // return type checking

  template <class T1, class T2>
  constexpr auto check_return_type(meta_type<T1> t1, meta_type<T2> t2)
  {
    if constexpr (t1 != t2) {
      static_assert(false_v<T1>, "return type does not match.");
      using _t1 = typename T1::_print_expected;
      using _t2 = typename T2::_print_provided;
      static_assert(false_v<_t1, _t2>, "compile-time type check failed.");
    }
  }

  /// Return type checker
  template <class T>
  class return_type_checker
  {
    static constexpr auto return_type = type_of(get_term<T>());

  public:
    /// object_ptr<U>&&
    template <class U>
    return_type_checker(object_ptr<U> obj) noexcept
      : m_value {std::move(obj)}
    {
      // check return type
      check_return_type(return_type, type_of(get_term<U>()));
    }

    /// U*
    template <class U>
    return_type_checker(U* ptr) noexcept
      : m_value(ptr)
    {
      // check return type
      check_return_type(return_type, type_of(get_term<U>()));
    }

    /// copy
    return_type_checker(const return_type_checker& other)
      : m_value {other.m_value}
    {
    }

    /// move
    return_type_checker(return_type_checker&& other)
      : m_value {std::move(other.m_value)}
    {
    }

    // deleted
    return_type_checker() = delete;
    return_type_checker(nullptr_t) = delete;

    /// value
    auto&& value() && noexcept
    {
      return std::move(m_value);
    }

  private:
    object_ptr<const Object> m_value;
  };

  /// Return type checker
  template <class T>
  class exception_handler_return_type_checker
  {
  public:
    /// return_type ctor
    exception_handler_return_type_checker(return_type_checker<T> e)
      : m_value {std::move(e).value()}
    {
    }

    /// Exception ctor
    template <class U>
    exception_handler_return_type_checker(object_ptr<U> e)
      : m_value {object_ptr<const Exception>(std::move(e))}
    {
      m_value = add_exception_tag(std::move(m_value));
    }

    /// Exception ctor
    exception_handler_return_type_checker(const Exception* e)
      : exception_handler_return_type_checker(object_ptr(e))
    {
      m_value = add_exception_tag(std::move(m_value));
    }

    // deleted
    exception_handler_return_type_checker() = delete;
    exception_handler_return_type_checker(nullptr_t) = delete;
    exception_handler_return_type_checker(
      const exception_handler_return_type_checker&) = delete;
    exception_handler_return_type_checker(
      exception_handler_return_type_checker&&) = delete;

    /// value
    auto&& value() && noexcept
    {
      return std::move(m_value);
    }

  private:
    object_ptr<const Object> m_value;
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

      /// specifier
      static constexpr auto specifier =
        normalize_specifier(type_c<closure<Ts...>>);

      /// Ctor
      Function() noexcept
        : ClosureN<sizeof...(Ts) - 1> {
            {{static_cast<const object_info_table*>(
               &info_table_initializer::info_table)},
             sizeof...(Ts) - 1},
          }
      {
      }

      /// Copy ctor
      Function(const Function& other) noexcept
        : ClosureN<sizeof...(Ts) - 1> {
            {
              {static_cast<const object_info_table*>(
                &info_table_initializer::info_table)},
              other.m_arity,
            },
            other.m_args}
      {
      }

      /// Move ctor
      Function(Function&& other) noexcept
        : ClosureN<sizeof...(Ts) - 1> {
            {
              {static_cast<const object_info_table*>(
                &info_table_initializer::info_table)},
              std::move(other.m_arity),
            },
            std::move(other.m_args)}
      {
      }

      /// operator=
      Function& operator=(const Function& other) noexcept
      {
        m_arity = other.m_arity;
        m_args = other.m_args;
        return *this;
      }

      /// operator=
      Function& operator=(Function&& other) noexcept
      {
        m_arity = std::move(other.m_arity);
        m_args = std::move(other.m_args);
        return *this;
      }

    protected:
      /// argument proxy type
      template <size_t N>
      using argument_proxy_t =
        std::add_const_t<typename decltype(get_argument_proxy_type(
          normalize_specifier(get<N>(tuple_c<Ts...>))))::type>;

      /// return type for code()
      using return_type =
        return_type_checker<argument_proxy_t<sizeof...(Ts) - 1>>;

      /// return type for exception_handler()
      using exception_handler_return_type =
        exception_handler_return_type_checker<
          argument_proxy_t<sizeof...(Ts) - 1>>;

      /// get N'th argument thunk
      template <uint64_t N>
      [[nodiscard]] auto arg() const noexcept
      {
        using To = argument_proxy_t<N>;
        static_assert(std::is_standard_layout_v<To>);
        auto obj = ClosureN<sizeof...(Ts) - 1>::template nth_arg<N>();
        TORI_ASSERT(obj);
        return static_object_cast<To>(obj);
      }

      /// evaluate N'th argument and take result
      template <uint64_t N>
      [[nodiscard]] auto eval_arg() const
      {
        // workaround: gcc 8.1
        return eval(this->template arg<N>());
      }

    public:
      /// default exception handler
      exception_handler_return_type exception_handler() const
      {
        return static_cast<const T*>(this)->code();
      }

    private:
      // You Can't See Me!
      using base = ClosureN<sizeof...(Ts) - 1>;
      using base::nth_arg;
      using base::arg;
      using base::n_args;
      using base::arity;
      using base::code;
      using base::m_arity;
      using base::m_args;

      /// Closure info table initializer
      struct info_table_initializer
      {
        /// static closure info
        alignas(64) static const closure_info_table info_table;
      };

      /// check signature of code()
      void check_code()
      {
        static_assert(
          std::is_same_v<return_type, decltype(std::declval<const T>().code())>,
          " `return_type code() const` was not found.");
      }

      // check_code
      using concept_check_code = concept_checker<&Function::check_code>;
    };

    // Initialize closure infotable
    template <class T, class... Ts>
    alignas(64) const closure_info_table
      Function<T, Ts...>::info_table_initializer::info_table = { //
        {object_type<T>(),                                       //
         sizeof(T),                                              //
         vtbl_destroy_func<T>,                                   //
         vtbl_clone_func<T>},                                    //
        sizeof...(Ts) - 1,                                       //
        vtbl_code_func<T>};                                      //

  } // namespace interface

} // namespace TORI_NS::detail
