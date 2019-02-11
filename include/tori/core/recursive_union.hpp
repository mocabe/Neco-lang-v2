// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

#include <utility>

namespace TORI_NS::detail {

  template <class... Ts>
  struct recursive_union_storage;

  template <class T>
  struct recursive_union_storage<T>
  {
    constexpr recursive_union_storage()
      : value {}
    {
    }

    template <class U, class = std::enable_if_t<std::is_same_v<T, U>>>
    constexpr recursive_union_storage(U t)
      : value {t}
    {
    }

    template <class U>
    constexpr auto& get() const
    {
      if constexpr (std::is_same_v<T, U>)
        return value;
      else
        static_assert(false_v<U>, "No matching type to access union member");
    }

    template <class U>
    constexpr auto& get()
    {
      if constexpr (std::is_same_v<T, U>)
        return value;
      else
        static_assert(false_v<U>, "No matching type to access union member");
    }

    union
    {
      T value;
    };
  };

  template <class T, class... Ts>
  struct recursive_union_storage<T, Ts...>
  {
    constexpr recursive_union_storage()
      : value {}
    {
    }

    template <class U>
    constexpr recursive_union_storage(
      U u,
      std::enable_if_t<std::is_same_v<U, T>>* = nullptr)
      : value {u}
    {
    }

    template <class U>
    constexpr recursive_union_storage(
      U u,
      std::enable_if_t<!std::is_same_v<U, T>>* = nullptr)
      : next {u}
    {
    }

    template <class U>
    constexpr auto& get() const
    {
      if constexpr (std::is_same_v<T, U>)
        return value;
      else
        return next.template get<U>();
    }

    template <class U>
    constexpr auto& get()
    {
      if constexpr (std::is_same_v<T, U>)
        return value;
      else
        return next.template get<U>();
    }

    union
    {
      T value;
      recursive_union_storage<Ts...> next;
    };
  };

  template <class... Ts>
  class recursive_union : private recursive_union_storage<Ts...>
  {
  public:
    constexpr recursive_union()
      : recursive_union_storage<Ts...> {}
    {
    }
    template <class U>
    constexpr recursive_union(U u)
      : recursive_union_storage<Ts...> {u}
    {
    }

    friend constexpr const recursive_union_storage<Ts...>&
      _get_storage(const recursive_union<Ts...>& u)
    {
      return u;
    }

    friend constexpr recursive_union_storage<Ts...>&
      _get_storage(recursive_union<Ts...>& u)
    {
      return u;
    }
  };

  template <class T, class... Ts>
  constexpr auto& get(const recursive_union<Ts...>& u)
  {
    return _get_storage(u).template get<T>();
  }

  template <class T, class... Ts>
  constexpr auto& get(recursive_union<Ts...>& u)
  {
    return _get_storage(u).template get<T>();
  }
}