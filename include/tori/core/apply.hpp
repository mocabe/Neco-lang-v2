// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once 

#include "../config/config.hpp"
#include "box.hpp"
#include "type_gen.hpp"
#include "fix.hpp"
#include "object_ptr_generic.hpp"

namespace TORI_NS::detail {

  /// value of ApplyR
  class ApplyRValue
  {
  public:
    template <
      class App,
      class Arg,
      class = std::enable_if_t<!std::is_same_v<std::decay_t<App>, ApplyRValue>>>
    ApplyRValue(App&& app, Arg&& arg)
      : m_app {std::forward<App>(app)}
      , m_arg {std::forward<Arg>(arg)}
    {
    }

    const object_ptr_generic& app() const
    {
      return m_app;
    }

    const object_ptr_generic& arg() const
    {
      return m_arg;
    }

    bool evaluated() const
    {
      return m_app.is_pointer() && _get_storage(m_app).ptr();
    }

    const object_ptr_generic& get_cache() const
    {
      assert(evaluated());
      return m_arg;
    }

    void set_cache(const object_ptr_generic& obj)
    {
      assert(!evaluated());
      m_app = nullptr;
      m_arg = obj;
    }

  private:
    /// closure
    /// when evaluated: nullptr
    object_ptr_generic m_app;
    /// argument
    /// when evaluated: result
    object_ptr_generic m_arg;
  };

  namespace interface {

    /// runtime apply object
    using ApplyR = Box<ApplyRValue>;

  } // namespace interface

  namespace interface {

    // apply x y
    //  (x:T, y:U) -> Apply<T,U>
    //  (x:generic, y:U) -> Apply
    //  (x:T, y:generic) -> Apply

    /// compile time apply object
    template <class App, class Arg>
    struct Apply : ApplyR
    {
      /// base
      using base = ApplyR;
      /// term
      static constexpr auto term = make_tm_apply(App::term, Arg::term);

      // clang-format off

      Apply(object_ptr<App> ap, object_ptr<Arg> ar) 
        : base(std::move(ap), std::move(ar)) {}

      Apply(App* ap, Arg* ar) 
        : base(object_ptr<>(ap), object_ptr<>(ar)) {}

      Apply(App* ap, object_ptr<Arg> ar) 
        : base(ap, std::move(ar)) {}

      Apply(object_ptr<App> ap, Arg* ar) 
        : base(std::move(ap), ar) {}

      // clang-format on
    };

  } // namespace interface

  template <class T>
  constexpr auto is_valid_app_arg(meta_type<T>)
  {
    return std::false_type {};
  }

  template <class T>
  constexpr auto is_valid_app_arg(meta_type<T*>)
  {
    return std::bool_constant<std::is_base_of_v<Object, T>> {};
  }

  template <class T>
  constexpr auto is_valid_app_arg(meta_type<object_ptr<T>>)
  {
    return std::true_type {};
  }

  namespace interface {

    /// apply operator
    template <
      class T1,
      class T2,
      class = std::enable_if_t<
        is_valid_app_arg(type_c<std::decay_t<T1>>) &&
        is_valid_app_arg(type_c<std::decay_t<T2>>)>>
    [[nodiscard]] auto operator<<(T1&& lhs, T2&& rhs)
    {
      // use {} to workaround gcc bug (81486?)
      return object_ptr(
        new Apply {std::forward<T1>(lhs), std::forward<T2>(rhs)});
    }

  } // namespace interface

} // namespace TORI_NS::detail

// ApplyR
TORI_DECL_TYPE(ApplyR)