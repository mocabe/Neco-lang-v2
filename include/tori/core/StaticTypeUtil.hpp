// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file Type.hpp - template type utility

#pragma once

#include <type_traits>
#include <utility>

// false_v
#include "../config/config.hpp"

namespace TORI_NS::detail {

  // ------------------------------------------
  // Utility
  // ------------------------------------------
  template <class T, class E>
  struct append_tuple {
    using type = std::tuple<T, E>;
  };
  template <class... Ts, class E>
  struct append_tuple<std::tuple<Ts...>, E> {
    using type = std::tuple<Ts..., E>;
  };
  /// append tuples
  template <class T, class E>
  using append_tuple_t = typename append_tuple<T, E>::type;

  template <class T1, class T2>
  struct concat_tuple {};
  template <class... T1s, class... T2s>
  struct concat_tuple<std::tuple<T1s...>, std::tuple<T2s...>> {
    using type = std::tuple<T1s..., T2s...>;
  };
  /// concat tuples
  template <class T1, class T2>
  using concat_tuple_t = typename concat_tuple<T1, T2>::type;

  template <class X, class T>
  struct set_belong {};
  template <class X>
  struct set_belong<X, std::tuple<>> {
    static constexpr bool value = false;
  };
  template <class X, class TH, class... TT>
  struct set_belong<X, std::tuple<TH, TT...>> {
    static constexpr bool value =
      std::is_same_v<X, TH> || set_belong<X, std::tuple<TT...>>::value;
  };
  /// Does X occur in Set?
  template <class X, class Set>
  static constexpr bool set_belong_v = set_belong<X, Set>::value;

  template <class X, class T>
  struct set_include {};
  template <class T>
  struct set_include<std::tuple<>, T> {
    static constexpr bool value = true;
  };
  template <class XH, class... XT, class T>
  struct set_include<std::tuple<XH, XT...>, T> {
    static constexpr bool value =
      set_belong_v<XH, T> && set_include<std::tuple<XT...>, T>::value;
  };
  /// Does S2 include S1?
  template <class S1, class S2>
  static constexpr bool set_include_v = set_include<S1, S2>::value;

  template <class X, class Y>
  struct set_eq {
    static constexpr bool value =
      set_include<X, Y>::value && set_include_v<Y, X>;
  };
  /// Are X and Y same set?
  template <class X, class Y>
  static constexpr bool set_eq_v = set_eq<X, Y>::value;

  // ------------------------------------------
  // Types
  // ------------------------------------------

  /// Arrow type
  template <class T1, class T2>
  struct arrow {
    using type = arrow<T1, T2>;
    using t1 = T1;
    using t2 = T2;
  };
  /// Type variable
  template <class Tag>
  struct var {
    using type = var<Tag>;
  };
  /// Value type
  template <class Tag>
  struct value {
    using type = value<Tag>;
  };
  // Var value
  template <class Tag>
  struct varvalue {
    using type = varvalue<Tag>;
  };

  /// Type mapping
  template <class T1, class T2>
  struct tyarrow {
    using type = tyarrow<T1, T2>;
  };

  // ------------------------------------------
  // Terms
  // ------------------------------------------

  /// TmApply
  template <class T1, class T2>
  struct TmApply {};
  /// TmClosure
  template <class... Ts>
  struct TmClosure {};
  /// TmValue
  template <class T>
  struct TmValue {
    using type = value<T>;
  };
  /// TmVar
  template <class Tag>
  struct TmVar {
    using type = var<Tag>;
  };
  /// TmVarValue
  template <class Tag>
  struct TmVarValue {
    using type = varvalue<Tag>;
  };

  /// TmFix
  template <class Tag>
  struct TmFix {};

  template <class T, class = void>
  struct has_term : std::false_type {};
  template <class T>
  struct has_term<T, void_t<typename T::term>> : std::true_type {};
  template <class T>
  static constexpr bool has_term_v = has_term<T>::value;

  // ------------------------------------------
  // Subst
  // ------------------------------------------
  template <class TyArrow, class Ty>
  struct subst_ {};
  template <class TyT1, class TyT2, class T1, class T2>
  struct subst_<tyarrow<TyT1, TyT2>, arrow<T1, T2>> {
    using type = arrow<
      typename subst_<tyarrow<TyT1, TyT2>, T1>::type,
      typename subst_<tyarrow<TyT1, TyT2>, T2>::type>;
  };
  template <class TyT1, class TyT2, class Tag>
  struct subst_<tyarrow<TyT1, TyT2>, value<Tag>> {
    using type = value<Tag>;
  };
  template <class TyT1, class TyT2, class Tag>
  struct subst_<tyarrow<TyT1, TyT2>, var<Tag>> {
    using type =
      std::conditional_t<std::is_same_v<TyT1, var<Tag>>, TyT2, var<Tag>>;
  };
  /// Apply TyArrow to Ty
  template <class TyArrow, class Ty>
  using subst_t = typename subst_<TyArrow, Ty>::type;

  template <class Arrows, class Ty>
  struct subst_all_ {};
  template <class Ty>
  struct subst_all_<std::tuple<>, Ty> {
    using type = Ty;
  };
  template <class TyArrow, class... Tail, class Ty>
  struct subst_all_<std::tuple<TyArrow, Tail...>, Ty> {
    using type =
      typename subst_all_<std::tuple<Tail...>, subst_t<TyArrow, Ty>>::type;
  };
  /// Apply all TyArrows to Ty
  template <class TyArrows, class Ty>
  using subst_all_t = typename subst_all_<TyArrows, Ty>::type;


  // ------------------------------------------
  // Constr
  // ------------------------------------------

  /// Type constraint
  template <class T1, class T2>
  struct constr {
    using type = constr<T1, T2>;
    using t1 = T1;
    using t2 = T2;
  };

  template <class TyArrow, class Constr>
  struct subst_constr_ {};
  template <class TyT1, class TyT2, class T1, class T2>
  struct subst_constr_<tyarrow<TyT1, TyT2>, constr<T1, T2>> {
    using type = constr<
      subst_t<tyarrow<TyT1, TyT2>, T1>,
      subst_t<tyarrow<TyT1, TyT2>, T2>>;
  };
  /// Apply TyArrow to Constr
  template <class TyArrow, class Constr>
  using subst_constr_t = typename subst_constr_<TyArrow, Constr>::type;

  template <class TyArrow, class Constrs>
  struct subst_constr_all_ {
    static_assert(false_v<TyArrow>, "Invalid argument(s)");
  };
  template <class TyT1, class TyT2, class... Cs>
  struct subst_constr_all_<tyarrow<TyT1, TyT2>, std::tuple<Cs...>> {
    using type = std::tuple<subst_constr_t<tyarrow<TyT1, TyT2>, Cs>...>;
  };
  /// Apply TyArrow to all constrains Cs
  template <class TyArrow, class Cs>
  using subst_constr_all_t = typename subst_constr_all_<TyArrow, Cs>::type;

  // ------------------------------------------
  // Occurs
  // ------------------------------------------
  template <class X, class T>
  struct occurs_ {};
  template <class X, class T1, class T2>
  struct occurs_<X, arrow<T1, T2>> {
    static constexpr bool value =
      occurs_<X, T1>::value || occurs_<X, T2>::value;
  };
  template <class X, class Tag>
  struct occurs_<X, value<Tag>> {
    static constexpr bool value = false;
  };
  template <class X, class Tag>
  struct occurs_<X, var<Tag>> {
    static constexpr bool value = std::is_same_v<X, var<Tag>>;
  };
  template <class X, class Tag>
  struct occurs_<X, varvalue<Tag>> {
    static constexpr bool value = false;
  };
  /// Does X occur in T?
  template <class X, class T>
  static constexpr bool occurs_v = occurs_<X, T>::value;

  // ------------------------------------------
  // Unify
  // ------------------------------------------

  // error
  template <class ConstrList>
  struct unify_ {
    using type = typename ConstrList::_error_here;
    static_assert(
      false_v<ConstrList>, "Unification error: Unsolvable constraints");
  };
  // helper
  template <class Var, class T, class Tail, bool B = !occurs_v<Var, T>>
  struct unify_h {
    static_assert(false_v<T>, "Unification error: Circular constraints");
  };
  // helper2
  template <
    class Tag1,
    class Tag2,
    class Tail,
    bool B = std::is_same_v<Tag1, Tag2>>
  struct unify_h2 {
    using t1 = typename value<Tag1>::_error_T1;
    using t2 = typename value<Tag2>::_error_T2;
    static_assert(false_v<Tag1>, "Unification error: Type missmatch");
  };
  // helper3
  template <
    class Tag1,
    class Tag2,
    class Tail,
    bool B = std::is_same_v<Tag1, Tag2>>
  struct unify_h3 {
    using t1 = typename varvalue<Tag1>::_error_T1;
    using t2 = typename varvalue<Tag2>::_error_T2;
    static_assert(false_v<Tag1>, "Unification error: Type missmatch");
  };

  // helper
  template <class Var, class T, class Tail>
  struct unify_h<Var, T, Tail, true> {
    using type = append_tuple_t<
      typename unify_<subst_constr_all_t<tyarrow<Var, T>, Tail>>::type,
      tyarrow<Var, T>>;
  };
  // helper2
  template <class Tag1, class Tag2, class Tail>
  struct unify_h2<Tag1, Tag2, Tail, true> {
    using type = typename unify_<Tail>::type;
  };
  // helper3
  template <class Tag1, class Tag2, class Tail>
  struct unify_h3<Tag1, Tag2, Tail, true> {
    using type = typename unify_<Tail>::type;
  };

  // empty set
  template <>
  struct unify_<std::tuple<>> {
    using type = std::tuple<>;
  };
  template <class Tag1, class Tag2, class... Cs>
  struct unify_<std::tuple<constr<value<Tag1>, value<Tag2>>, Cs...>> {
    using type = typename unify_h2<Tag1, Tag2, std::tuple<Cs...>>::type;
  };
  template <class Tag1, class Tag2, class... Cs>
  struct unify_<std::tuple<constr<varvalue<Tag1>, varvalue<Tag2>>, Cs...>> {
    using type = typename unify_h3<Tag1, Tag2, std::tuple<Cs...>>::type;
  };
  template <class Tag1, class Tag2, class... Cs>
  struct unify_<std::tuple<constr<var<Tag1>, var<Tag2>>, Cs...>> {
    using type = std::conditional_t<
      std::is_same_v<var<Tag1>, var<Tag2>>,
      typename unify_<std::tuple<Cs...>>::type,
      typename unify_h<var<Tag2>, var<Tag1>, std::tuple<Cs...>>::type>;
  };
  template <class T1, class Tag, class... Cs>
  struct unify_<std::tuple<constr<T1, var<Tag>>, Cs...>> {
    using type = std::conditional_t<
      std::is_same_v<T1, var<Tag>>,
      typename unify_<std::tuple<Cs...>>::type,
      typename unify_h<var<Tag>, T1, std::tuple<Cs...>>::type>;
  };
  template <class Tag, class T2, class... Cs>
  struct unify_<std::tuple<constr<var<Tag>, T2>, Cs...>> {
    using type = std::conditional_t<
      std::is_same_v<var<Tag>, T2>,
      typename unify_<std::tuple<Cs...>>::type,
      typename unify_h<var<Tag>, T2, std::tuple<Cs...>>::type>;
  };
  template <class S1, class S2, class T1, class T2, class... Cs>
  struct unify_<std::tuple<constr<arrow<S1, S2>, arrow<T1, T2>>, Cs...>> {
    using type =
      typename unify_<std::tuple<constr<S1, T1>, constr<S2, T2>, Cs...>>::type;
  };

  /// Unification
  template <class Cs>
  using unify_t = typename unify_<Cs>::type;

  // ------------------------------------------
  // TmClosure to Tuple
  // ------------------------------------------
  template <class T>
  struct to_tuple {};

  template <class... Ts>
  struct to_tuple<TmClosure<Ts...>> {
    using type = std::tuple<Ts...>;
  };

  template <class T>
  using to_tuple_t = typename to_tuple<T>::type;

  // ------------------------------------------
  // Tuple to TmClosure
  // ------------------------------------------

  template <class T>
  struct to_TmClosure {};
  template <class... Ts>
  struct to_TmClosure<std::tuple<Ts...>> {
    using type = TmClosure<Ts...>;
  };
  template <class T>
  using to_TmClosure_t = typename to_TmClosure<T>::type;

  // ------------------------------------------
  // Typing
  // ------------------------------------------

  /// Tag type generator
  template <size_t N>
  struct taggen {};

  template <class GenTag>
  struct genvar_ {};

  template <size_t N>
  struct genvar_<taggen<N>> {
    using type = var<taggen<N>>;
    using next = taggen<N + 1>;
  };

  /// Generate new type variable
  template <class Gen>
  using genvar_t = typename genvar_<Gen>::type;

  /// Get next tag generator
  template <class Gen>
  using nextgen_t = typename genvar_<Gen>::next;

  template <class From ,class To, class In>
  struct subst_term_ {};

  template <class From, class To, class... Ts>
  struct subst_term_<From, To, TmClosure<Ts...>> {
    using type = std::conditional_t<
      std::is_same_v<From, TmClosure<Ts...>>,
      To,
      TmClosure<typename subst_term_<From, To, Ts>::type...>>;
  };

  template <class From, class To, class T1, class T2>
  struct subst_term_<From, To, TmApply<T1, T2>> {
    using type = std::conditional_t<
      std::is_same_v<From, TmApply<T1, T2>>,
      To,
      TmApply<
        typename subst_term_<From, To, T1>::type,
        typename subst_term_<From, To, T2>::type>>;
  };

  template <class From, class To, class T>
  struct subst_term_<From, To, TmValue<T>> {
    using type =
      std::conditional_t<std::is_same_v<From, TmValue<T>>, To, TmValue<T>>;
  };

  template <class From, class To, class Tag>
  struct subst_term_<From, To, TmVar<Tag>> {
    using type =
      std::conditional_t<std::is_same_v<From, TmVar<Tag>>, To, TmVar<Tag>>;
  };

  template <class From, class To, class Tag>
  struct subst_term_<From, To, TmVarValue<Tag>> {
    using type = std::
      conditional_t<std::is_same_v<From, TmVarValue<Tag>>, To, TmVarValue<Tag>>;
  };

  template <class From, class To, class Tag>
  struct subst_term_<From, To, TmFix<Tag>> {
    using type =
      std::conditional_t<std::is_same_v<From, TmFix<Tag>>, To, TmFix<Tag>>;
  };

  template <class From, class To, class Term>
  using subst_term_t = typename subst_term_<From, To, Term>::type;

  template <class T, class Gen, class Target>
  struct genpoly_ {
    using term = Target;
    using gen = Gen;
  };

  template <class T, class Gen, class Target>
  struct genpoly_impl_ {};

  template <class Tag, class... Ts, class Gen, class Target>
  struct genpoly_impl_<std::tuple<TmVar<Tag>, Ts...>, Gen, Target> {
    using _var = TmVar<Gen>;
    using t = genpoly_impl_<
      std::tuple<Ts...>,
      nextgen_t<Gen>,
      subst_term_t<TmVar<Tag>, _var, Target>>;

    using term = typename t::term;
    using gen = typename t::gen;
  };

  template <class T, class... Ts, class Gen, class Target>
  struct genpoly_impl_<std::tuple<T, Ts...>, Gen, Target> {
    using t = genpoly_impl_<std::tuple<Ts...>, Gen, Target>;
    using term  = typename t::term;
    using gen  = typename t::gen;
  };

  template <class Tag,class Gen, class Target>
  struct genpoly_impl_<std::tuple<TmVar<Tag>>, Gen, Target> {
    using _var = TmVar<Gen>;
    using term = subst_term_t<TmVar<Tag>, _var, Target>;
    using gen = nextgen_t<Gen>;
  };

  template <class T, class Gen, class Target>
  struct genpoly_impl_<std::tuple<T>, Gen, Target> {
    using term = typename genpoly_<T, Gen, Target>::term;
    using gen = typename genpoly_<T, Gen, Target>::gen;
  };

  template <class T1, class T2, class Gen, class Target>
  struct genpoly_<TmApply<T1,T2>, Gen, Target> {
    using t1 = genpoly_<T1, Gen, Target>;
    using t2 = genpoly_<T2, typename t1::gen, typename t1::term>;
    using term = typename t2::term;
    using gen = typename t2::gen;
  };

  template <class... Ts, class Gen, class Target>
  struct genpoly_<TmClosure<Ts...>, Gen, Target> {
    using t = genpoly_impl_<std::tuple<Ts...>, Gen, Target>;
    using term = typename t::term;
    using gen = typename t::gen;
  };

  template <class Term, class Gen>
  using genpoly_term = typename genpoly_<Term, Gen, Term>::term;

  template <class Term, class Gen>
  using genpoly_gen = typename genpoly_<Term, Gen, Term>::gen;

  template <class T, class Gen>
  struct recon_ {};

  template <class T, class Gen>
  struct recon_h {
    using type = typename recon_<T, Gen>::type;
    using gen = typename recon_<T, Gen>::gen;
    using c = typename recon_<T, Gen>::c;
  };

  template <class T, class ...Ts, class Gen>
  struct recon_h<TmClosure<T, Ts...>, Gen> {
    // t1
    using _t1 = recon_h<T, Gen>;
    using _t1_t = typename _t1::type;
    using _t1_gen = typename _t1::gen;
    using _t1_c = typename _t1::c;
    // t2
    using _t2 = recon_h<TmClosure<Ts...>, _t1_gen>;
    using _t2_t = typename _t2::type;
    using _t2_gen = typename _t2::gen;
    using _t2_c = typename _t2::c;
    // here we go...
    using type = arrow<_t1_t, _t2_t>;
    using gen = _t2_gen;
    using c = concat_tuple_t<_t1_c, _t2_c>;
  };
  template <class T, class Gen>
  struct recon_h<TmClosure<T>, Gen> {
    // unwrap
    using type = typename recon_h<T, Gen>::type;
    using gen = typename recon_h<T, Gen>::gen;
    using c = typename recon_h<T, Gen>::c;
  };

  template <class Tag, class Gen>
  struct recon_<TmValue<Tag>, Gen> {
    using type = typename TmValue<Tag>::type;
    using c = std::tuple<>;
    using gen = Gen;
  };
  template <class Tag, class Gen>
  struct recon_<TmVar<Tag>, Gen> {
    using type = typename TmVar<Tag>::type;
    using c = std::tuple<>;
    using gen = Gen;
  };
  template <class Tag, class Gen>
  struct recon_<TmVarValue<Tag>, Gen> {
    using type = typename TmVarValue<Tag>::type;
    using c = std::tuple<>;
    using gen = Gen;
  };
  template <class... Ts, class Gen>
  struct recon_<TmClosure<Ts...>, Gen> {
    using rcn = recon_h<
      genpoly_term<TmClosure<Ts...>, Gen>,
      genpoly_gen<TmClosure<Ts...>, Gen>>;
    using type = typename rcn::type;
    using gen = typename rcn::gen;
    using c = typename rcn::c;
  };
  template <class Tag, class T, class Gen>
  struct recon_<TmApply<TmFix<Tag>, T>, Gen> {
    // recon T
    using _t = recon_<T, Gen>;
    using _t_t = typename _t::type;
    using _t_gen = typename _t::gen;
    using _t_c = typename _t::c;
    //
    using type = genvar_t<_t_gen>;
    using gen = nextgen_t<_t_gen>;
    using c = append_tuple_t<_t_c, constr<_t_t, arrow<type, type>>>;
  };
  template <class T1, class T2, class Gen>
  struct recon_<TmApply<T1, T2>, Gen> {
    using _t1 = recon_<T1, Gen>;
    using _t2 = recon_<T2, typename _t1::gen>;
    using _gen = typename _t2::gen;
    using type = genvar_t<_gen>;
    using gen = nextgen_t<_gen>;
    using c = append_tuple_t<
      concat_tuple_t<typename _t1::c, typename _t2::c>,
      constr<typename _t1::type, arrow<typename _t2::type, type>>>;
  };

  /// Get type of term
  template <class Term>
  using recon_type_t = typename recon_<Term, taggen<0>>::type;
  /// Get constraint set of term
  template <class Term>
  using recon_constr_t = typename recon_<Term, taggen<0>>::c;

  /// Infer type of term
  template <class Term>
  using type_of_t =
    subst_all_t<unify_t<recon_constr_t<Term>>, recon_type_t<Term>>;

  // ------------------------------------------
  // Util
  // ------------------------------------------
  template <class T>
  struct is_TmApply : std::false_type {};
  template <class T1, class T2>
  struct is_TmApply<TmApply<T1, T2>> : std::true_type {};

  template <class T>
  struct is_TmValue : std::false_type {};
  template <class Tag>
  struct is_TmValue<TmValue<Tag>> : std::true_type {};

  template <class T>
  struct is_TmClosure : std::false_type {};
  template <class... Ts>
  struct is_TmClosure<TmClosure<Ts...>> : std::true_type {};

  template <class T>
  struct is_TmVar : std::false_type {};
  template <class Tag>
  struct is_TmVar<TmVar<Tag>> : std::true_type {};

  template <class T>
  struct is_TmVarValue : std::false_type {};
  template <class Tag>
  struct is_TmVarValue<TmVarValue<Tag>> : std::true_type {};

  template <class T>
  struct is_TmFix : std::false_type {};
  template <class Tag>
  struct is_TmFix<TmFix<Tag>> : std::true_type {};

  /// is_apply_v
  template <class T>
  static constexpr bool is_TmApply_v = is_TmApply<T>::value;
  /// has_apply_v
  template <class T>
  static constexpr bool has_TmApply_v = is_TmApply_v<typename T::term>;
  /// is_value_v
  template <class T>
  static constexpr bool is_TmValue_v = is_TmValue<T>::value;
  /// has_value_v
  template <class T>
  static constexpr bool has_TmValue_v = is_TmValue_v<typename T::term>;
  /// is_closure_v
  template <class T>
  static constexpr bool is_TmClosure_v = is_TmClosure<T>::value;
  /// has_closure_v
  template <class T>
  static constexpr bool has_TmClosure_v = is_TmClosure_v<typename T::term>;
  /// is_var_v
  template <class T>
  static constexpr bool is_TmVar_v = is_TmVar<T>::value;
  /// has_var_v
  template <class T>
  static constexpr bool has_TmVar_v = is_TmVar_v<typename T::term>;
  /// is_varvalue_v
  template <class T>
  static constexpr bool is_TmVarValue_v = is_TmVarValue<T>::value;
  /// has_varvalue_v
  template <class T>
  static constexpr bool has_TmVarValue_v = is_TmVarValue_v<typename T::term>;
  // is_TmFix
  template <class T>
  static constexpr bool is_TmFix_v = is_TmFix<T>::value;
  // has_TmFix
  template <class T>
  static constexpr bool has_TmFix_v = is_TmFix_v<typename T::term>;

  template <class T>
  struct is_value_type_ : std::false_type {};

  template <class Tag>
  struct is_value_type_<value<Tag>> : std::true_type {};
  /// is_value_v
  template <class T>
  static constexpr bool is_value_type_v = is_value_type_<T>::value;

  template <class T>
  struct is_arrow_type_ : std::false_type {};

  template <class T1, class T2>
  struct is_arrow_type_<arrow<T1, T2>> : std::true_type {};
  /// is_arrow_type_v
  template <class T>
  static constexpr bool is_arrow_type_v = is_arrow_type_<T>::value;

  template <class T>
  struct is_vartype_ : std::false_type {};

  template <class Tag>
  struct is_vartype_<var<Tag>> : std::true_type {};
  /// is_vartype_v
  template <class T>
  static constexpr bool is_vartype_v = is_vartype_<T>::value;

  template <class T>
  struct is_varvalue_type_ : std::false_type {};

  template <class Tag>
  struct is_varvalue_type_<varvalue<Tag>> : std::true_type {};
  /// is_varvalue_v
  template <class T>
  static constexpr bool is_varvalue_type_v = is_varvalue_type_<T>::value;

  template <class T>
  struct tag_of {};
  template <class Tag>
  struct tag_of<TmValue<Tag>> {
    using type = Tag;
  };
  template <class Tag>
  struct tag_of<TmVar<Tag>> {
    using type = Tag;
  };
  template <class Tag>
  struct tag_of<TmVarValue<Tag>> {
    using type = Tag;
  };
  template <class Tag>
  struct tag_of<TmFix<Tag>> {
    using type = Tag;
  };
  template <class T>
  using tag_of_t = typename tag_of<T>::type;

} // namespace TORI_NS::detail