#pragma once 

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file Apply

#include "DynamicTypeUtil.hpp"
#include "Fix.hpp"

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

    const object_ptr<>& app() const
    {
      assert(m_app != nullptr);
      return m_app;
    }

    const object_ptr<>& arg() const
    {
      assert(m_app != nullptr);
      return m_arg;
    }

    bool evaluated() const
    {
      return m_app == nullptr;
    }

    const object_ptr<>& get_cache() const
    {
      assert(evaluated());
      return m_arg;
    }

    void set_cache(const object_ptr<>& obj)
    {
      assert(!evaluated());
      m_app = nullptr;
      m_arg = obj;
    }

  private:
    /// closure
    /// when evaluated: nullptr
    object_ptr<> m_app;
    /// argument
    /// when evaluated: result
    object_ptr<> m_arg;
  };

  namespace interface {

    /// runtime apply object
    using ApplyR = BoxedHeapObject<ApplyRValue>;

  } // namespace interface

  namespace interface {

    /// compile time apply object
    template <class App, class Arg>
    struct Apply : ApplyR
    {
      /// base
      using base = ApplyR;
      /// term
      using term = tm_apply<typename App::term, typename Arg::term>;

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
  struct is_valid_app_arg : std::false_type
  {
  };

  template <class T>
  struct is_valid_app_arg<T*>
  {
    static constexpr bool value = std::is_base_of_v<HeapObject, T>;
  };

  template <class T>
  struct is_valid_app_arg<object_ptr<T>> : std::true_type
  {
  };

  /// validate argument types for operator<<
  template <class T>
  static constexpr bool is_valid_app_arg_v =
    is_valid_app_arg<std::decay_t<T>>::value;

  namespace interface {

    /// apply operator
    template <
      class T1,
      class T2,
      class =
        std::enable_if_t<is_valid_app_arg_v<T1> && is_valid_app_arg_v<T2>>>
    [[nodiscard]] auto operator<<(T1&& lhs, T2&& rhs)
    {
      // use {} to workaround gcc bug (81486?)
      return object_ptr(
        new Apply {std::forward<T1>(lhs), std::forward<T2>(rhs)});
    }

  } // namespace interface

} // namespace TORI_NS::detail

namespace TORI_NS {
  TORI_DECL_TYPE(ApplyR)
} // namespace TORI_NS