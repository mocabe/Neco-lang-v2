// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

#include "meta_type.hpp"
#include "meta_tuple.hpp"

namespace TORI_NS::detail {

  // ------------------------------------------
  // Term

  /// tm_apply
  template <class T1, class T2>
  struct tm_apply
  {
  };

  /// tm_closure
  template <class... Ts>
  struct tm_closure
  {
  };

  /// tm_value
  template <class T>
  struct tm_value
  {
  };

  /// tm_var
  template <class Tag>
  struct tm_var
  {
  };

  /// tm_varvalue
  template <class Tag>
  struct tm_varvalue
  {
  };

  /// tm_fix
  template <class Tag>
  struct tm_fix
  {
  };

  // ------------------------------------------
  // get_term

  template <class T>
  constexpr auto get_term(meta_type<T> = {})
  {
    if constexpr (is_complete_v<T>)
      return T::term;
    else
      // map incomplete type to type variable
      return type_c<tm_var<T>>;
  }

  // ------------------------------------------
  // Term accessors

  template <class T1, class T2>
  struct meta_type<tm_apply<T1, T2>>
  {
    using type = tm_apply<T1, T2>;
    constexpr auto t1() const
    {
      return type_c<T1>;
    }
    constexpr auto t2() const
    {
      return type_c<T2>;
    }
  };

  template <class Tag>
  struct meta_type<tm_value<Tag>>
  {
    using type = tm_value<Tag>;
    constexpr auto tag() const
    {
      return type_c<Tag>;
    }
  };

  template <class Tag>
  struct meta_type<tm_fix<Tag>>
  {
    using type = tm_fix<Tag>;
    constexpr auto tag() const
    {
      return type_c<Tag>;
    }
  };

  template <class Tag>
  struct meta_type<tm_varvalue<Tag>>
  {
    using type = tm_varvalue<Tag>;
    constexpr auto tag() const
    {
      return type_c<Tag>;
    }
  };

  template <class Tag>
  struct meta_type<tm_var<Tag>>
  {
    using type = tm_var<Tag>;
    constexpr auto tag() const
    {
      return type_c<Tag>;
    }
  };

  template <class... Ts>
  struct meta_type<tm_closure<Ts...>>
  {
    using type = tm_closure<Ts...>;
    constexpr size_t size() const
    {
      return sizeof...(Ts);
    }
  };

  // ------------------------------------------
  // has_term

  template <class T, class = void>
  struct has_term_impl
  {
    static constexpr auto value = false_c;
  };

  template <class T>
  struct has_term_impl<T, std::void_t<decltype(T::term)>>
  {
    static constexpr auto value = true_c;
  };

  template <class T>
  constexpr auto has_term()
  {
    return has_term_impl<T>::value;
  }

  // ------------------------------------------
  // is_tm_apply

  template <class T>
  struct is_tm_apply_impl
  {
    static constexpr auto value = false_c;
  };

  template <class T1, class T2>
  struct is_tm_apply_impl<tm_apply<T1, T2>>
  {
    static constexpr auto value = true_c;
  };

  /// is_apply_v
  template <class T>
  constexpr auto is_tm_apply(meta_type<T>)
  {
    return is_tm_apply_impl<T>::value;
  }

  /// has_apply_v
  template <class T>
  constexpr auto has_tm_apply()
  {
    return is_tm_apply(get_term<T>());
  }

  // ------------------------------------------
  // is_tm_value

  template <class T>
  struct is_tm_value_impl
  {
    static constexpr auto value = false_c;
  };

  template <class Tag>
  struct is_tm_value_impl<tm_value<Tag>>
  {
    static constexpr auto value = true_c;
  };

  /// is_value_v
  template <class T>
  constexpr auto is_tm_value(meta_type<T>)
  {
    return is_tm_value_impl<T>::value;
  }

  /// has_value_v
  template <class T>
  constexpr auto has_tm_value()
  {
    return is_tm_value(get_term<T>());
  }

  // ------------------------------------------
  // is_tm_closure

  template <class T>
  struct is_tm_closure_impl
  {
    static constexpr auto value = false_c;
  };

  template <class... Ts>
  struct is_tm_closure_impl<tm_closure<Ts...>>
  {
    static constexpr auto value = true_c;
  };

  /// is_closure_v
  template <class T>
  constexpr auto is_tm_closure(meta_type<T>)
  {
    return is_tm_closure_impl<T>::value;
  }

  /// has_closure_v
  template <class T>
  constexpr auto has_tm_closure()
  {
    return is_tm_closure(get_term<T>());
  }

  // ------------------------------------------
  // is_tm_var

  template <class T>
  struct is_tm_var_impl
  {
    static constexpr auto value = false_c;
  };

  template <class Tag>
  struct is_tm_var_impl<tm_var<Tag>>
  {
    static constexpr auto value = true_c;
  };

  /// is_var_v
  template <class T>
  constexpr auto is_tm_var(meta_type<T>)
  {
    return is_tm_var_impl<T>::value;
  }

  /// has_var_v
  template <class T>
  constexpr auto has_tm_var()
  {
    return is_tm_var(get_term<T>());
  }

  // ------------------------------------------
  // is_tm_varvalue

  template <class T>
  struct is_tm_varvalue_impl
  {
    static constexpr auto value = false_c;
  };

  template <class Tag>
  struct is_tm_varvalue_impl<tm_varvalue<Tag>>
  {
    static constexpr auto value = true_c;
  };

  /// is_varvalue_v
  template <class T>
  constexpr auto is_tm_varvalue(meta_type<T>)
  {
    return is_tm_varvalue_impl<T>::value;
  }

  /// has_varvalue_v
  template <class T>
  constexpr auto has_tm_varvalue()
  {
    return is_varvalue(get_term<T>());
  }

  // ------------------------------------------
  // is_tm_fix

  template <class T>
  struct is_tm_fix_impl
  {
    static constexpr auto value = false_c;
  };

  template <class Tag>
  struct is_tm_fix_impl<tm_fix<Tag>>
  {
    static constexpr auto value = true_c;
  };

  // is_tm_fix
  template <class T>
  constexpr auto is_tm_fix(meta_type<T>)
  {
    return is_tm_fix_impl<T>::value;
  }

  // has_tm_fix
  template <class T>
  constexpr auto has_tm_fix()
  {
    return is_tm_fix(get_term<T>());
  }

  // ------------------------------------------
  // to_tuple

  template <class... Ts>
  constexpr auto to_tuple(meta_type<tm_closure<Ts...>>)
  {
    return tuple_c<Ts...>;
  }

  // ------------------------------------------
  // to_tm_closure

  template <class... Ts>
  constexpr auto to_tm_closure(meta_tuple<Ts...>)
  {
    return type_c<tm_closure<Ts...>>;
  }

  // ------------------------------------------
  // make_tm_closure

  template <class... Ts>
  constexpr auto make_tm_closure(meta_type<Ts>...)
  {
    return type_c<tm_closure<Ts...>>;
  }

  // ------------------------------------------
  // make_tm_apply

  template <class T1, class T2>
  constexpr auto make_tm_apply(meta_type<T1>, meta_type<T2>)
  {
    return type_c<tm_apply<T1, T2>>;
  }

  // ------------------------------------------
  // make_tm_var

  template <class Tag>
  constexpr auto make_tm_var(meta_type<Tag>)
  {
    return type_c<tm_var<Tag>>;
  }

  // ------------------------------------------
  // head

  template <class T, class... Ts>
  constexpr auto head(meta_type<tm_closure<T, Ts...>>)
  {
    return type_c<T>;
  }

  // ------------------------------------------
  // tail

  template <class T, class... Ts>
  constexpr auto tail(meta_type<tm_closure<T, Ts...>>)
  {
    return type_c<tm_closure<Ts...>>;
  }

  constexpr auto tail(meta_type<tm_closure<>> term)
  {
    return term;
  }

}