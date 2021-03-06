// Copyright (c) 2018-2019 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#if !defined(TORI_NO_LOCAL_INCLUDE)
#  include "../config/config.hpp"
#  include "meta_type.hpp"
#endif

namespace TORI_NS::detail {

  // ------------------------------------------
  // Types

  /// Arrow type
  template <class T1, class T2>
  struct arrow
  {
  };

  /// Type variable
  template <class Tag>
  struct var
  {
  };

  /// Value type
  template <class Tag>
  struct value
  {
  };

  // Var value
  template <class Tag>
  struct varvalue
  {
  };

  /// Type mapping
  template <class T1, class T2>
  struct tyarrow
  {
  };

  /// error_type
  template <class Tag>
  struct error_type
  {
    constexpr auto tag() const
    {
      return type_c<Tag>;
    }
  };

  /// error tag types
  namespace error_tags {

    /// type_missmatch
    template <class T1, class T2, class Other = std::tuple<>>
    struct type_missmatch
    {
      using left = T1;
      using right = T2;
      using other = Other;
    };

    /// unsolvable_constraints
    template <class T1, class T2, class Other = std::tuple<>>
    struct unsolvable_constraints
    {
      using left = T1;
      using right = T2;
      using other = Other;
    };

    /// circular_constraints
    template <class Var, class Other = std::tuple<>>
    struct circular_constraints
    {
      using var = Var;
      using other = Other;
    };

    /// none
    struct unknown_error
    {
    };

  } // namespace error_tags

  // ------------------------------------------
  // meta_type specializations

  template <class T1, class T2>
  struct meta_type<arrow<T1, T2>>
  {
    constexpr auto t1() const
    {
      return type_c<T1>;
    }
    constexpr auto t2() const
    {
      return type_c<T2>;
    }
  };

  template <class T1, class T2>
  constexpr auto make_arrow(meta_type<T1>, meta_type<T2>)
  {
    return type_c<arrow<T1, T2>>;
  }

  template <class T1, class T2>
  struct meta_type<tyarrow<T1, T2>>
  {
    constexpr auto t1() const
    {
      return type_c<T1>;
    }
    constexpr auto t2() const
    {
      return type_c<T2>;
    }
  };

  template <class T1, class T2>
  constexpr auto make_tyarrow(meta_type<T1>, meta_type<T2>)
  {
    return type_c<tyarrow<T1, T2>>;
  }

  template <class Tag>
  struct meta_type<value<Tag>>
  {
    constexpr auto tag() const
    {
      return type_c<Tag>;
    }
  };

  template <class Tag>
  constexpr auto make_value(meta_type<Tag>)
  {
    return type_c<value<Tag>>;
  }

  template <class Tag>
  struct meta_type<var<Tag>>
  {
    constexpr auto tag() const
    {
      return type_c<Tag>;
    }
  };

  template <class Tag>
  constexpr auto make_var(meta_type<Tag>)
  {
    return type_c<var<Tag>>;
  }

  template <class Tag>
  struct meta_type<varvalue<Tag>>
  {
    constexpr auto tag() const
    {
      return type_c<Tag>;
    }
  };

  template <class Tag>
  constexpr auto make_varvalue(meta_type<Tag>)
  {
    return type_c<varvalue<Tag>>;
  }

  template <class Tag>
  struct meta_type<error_type<Tag>>
  {
    constexpr auto tag() const
    {
      return type_c<Tag>;
    }
  };

  template <class T1, class T2, class Other>
  constexpr auto
    make_unsolvable_constraints(meta_type<T1>, meta_type<T2>, Other)
  {
    return type_c<
      error_type<error_tags::unsolvable_constraints<T1, T2, Other>>>;
  }

  template <class T1, class T2, class Other>
  constexpr auto make_type_missmatch(meta_type<T1>, meta_type<T2>, Other)
  {
    return type_c<error_type<error_tags::type_missmatch<T1, T2, Other>>>;
  }

  template <class Var, class Other>
  constexpr auto make_circular_constraints(meta_type<Var>, Other)
  {
    return type_c<error_type<error_tags::circular_constraints<Var, Other>>>;
  }

  // ------------------------------------------
  // is_error_type

  template <class T>
  struct is_error_type_impl
  {
    static constexpr std::false_type value {};
  };

  template <class Tag>
  struct is_error_type_impl<error_type<Tag>>
  {
    static constexpr std::true_type value {};
  };

  template <class T>
  constexpr auto is_error_type(meta_type<T>)
  {
    return is_error_type_impl<T>::value;
  }

  template <class T>
  constexpr auto is_error_type(T)
  {
    return std::false_type {};
  }

  // ------------------------------------------
  // is_type_missmatch

  template <class T>
  struct is_type_missmatch_impl
  {
    static constexpr std::false_type value {};
  };

  template <class T1, class T2, class Other>
  struct is_type_missmatch_impl<error_tags::type_missmatch<T1, T2, Other>>
  {
    static constexpr std::true_type value {};
  };

  template <class T>
  constexpr auto is_type_missmatch(meta_type<T>)
  {
    return is_type_missmatch_impl<T>::value;
  }

  // ------------------------------------------
  // is_unsolvable_constraints

  template <class T>
  struct is_unsolvable_constraints_impl
  {
    static constexpr std::false_type value {};
  };

  template <class T1, class T2, class Other>
  struct is_unsolvable_constraints_impl<
    error_tags::unsolvable_constraints<T1, T2, Other>>
  {
    static constexpr std::true_type value {};
  };

  template <class T>
  constexpr auto is_unsolvable_constraints(meta_type<T>)
  {
    return is_unsolvable_constraints_impl<T>::value;
  }

  // ------------------------------------------
  // is_circular_constraints

  template <class T>
  struct is_circular_constraints_impl
  {
    static constexpr std::false_type value {};
  };

  template <class Var, class Other>
  struct is_circular_constraints_impl<
    error_tags::circular_constraints<Var, Other>>
  {
    static constexpr std::true_type value {};
  };

  template <class T>
  constexpr auto is_circular_constraints(meta_type<T>)
  {
    return is_circular_constraints_impl<T>::value;
  }

  // ------------------------------------------
  // is_unknown_error

  template <class T>
  struct is_unknown_error_impl
  {
    static constexpr std::false_type value {};
  };

  template <>
  struct is_unknown_error_impl<error_tags::unknown_error> {
    static constexpr std::true_type value{};
  };

  template <class T>
  constexpr auto is_unknown_error(meta_type<T>)
  {
    return is_unknown_error_impl<T>::value;
  }

  // ------------------------------------------
  // is_value_type

  template <class T>
  struct is_value_type_impl
  {
    static constexpr std::false_type value {};
  };

  template <class Tag>
  struct is_value_type_impl<value<Tag>>
  {
    static constexpr std::true_type value {};
  };

  template <class T>
  constexpr auto is_value_type(meta_type<T>)
  {
    return is_value_type_impl<T>::value;
  }

  // ------------------------------------------
  // is_arrow_type

  template <class T>
  struct is_arrow_type_impl
  {
    static constexpr std::false_type value {};
  };

  template <class T1, class T2>
  struct is_arrow_type_impl<arrow<T1, T2>>
  {
    static constexpr std::true_type value {};
  };

  template <class T>
  constexpr auto is_arrow_type(meta_type<T>)
  {
    return is_arrow_type_impl<T>::value;
  }

  // ------------------------------------------
  // is_var_type

  template <class T>
  struct is_var_type_impl
  {
    static constexpr std::false_type value {};
  };

  template <class Tag>
  struct is_var_type_impl<var<Tag>>
  {
    static constexpr std::true_type value {};
  };

  template <class T>
  constexpr auto is_var_type(meta_type<T>)
  {
    return is_var_type_impl<T>::value;
  }

  // ------------------------------------------
  // is_varvalue_type

  template <class T>
  struct is_varvalue_type_impl
  {
    static constexpr std::false_type value {};
  };

  template <class Tag>
  struct is_varvalue_type_impl<varvalue<Tag>>
  {
    static constexpr std::true_type value {};
  };

  template <class T>
  constexpr auto is_varvalue_type(meta_type<T>)
  {
    return is_varvalue_type_impl<T>::value;
  }


  // ------------------------------------------
  // taggen

  template <size_t N>
  struct taggen
  {
  };

  template <size_t N>
  constexpr auto gen_c = type_c<taggen<N>>;

  // ------------------------------------------
  // nextgen

  template <size_t N>
  constexpr auto nextgen(meta_type<taggen<N>>)
  {
    return type_c<taggen<N + 1>>;
  }

  // ------------------------------------------
  // genvar

  template <size_t N>
  constexpr auto gen_var(meta_type<taggen<N>>)
  {
    return type_c<var<taggen<N>>>;
  }

  template <size_t N>
  constexpr auto gen_tm_var(meta_type<taggen<N>>)
  {
    return type_c<tm_var<taggen<N>>>;
  }

} // namespace TORI_NS::detail