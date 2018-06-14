#include <tori/core.hpp>

int main() {
  using namespace tori::detail;

  // [X -> int]
  using constr1 = std::tuple<constr<var<class X>, value<int>>>;
  static_assert(
    set_eq_v<std::tuple<tyarrow<var<class X>, value<int>>>, unify_t<constr1>>);

  // [X = int, Y = X -> X]
  using constr2 = std::tuple<
    constr<var<class X>, value<int>>,
    constr<var<class Y>, arrow<var<class X>, var<class X>>>>;
  static_assert(set_eq_v<
                std::tuple<
                  tyarrow<var<class X>, value<int>>,
                  tyarrow<var<class Y>, arrow<value<int>, value<int>>>>,
                unify_t<constr2>>);

  // [int->int = X -> Y]
  using constr3 = std::tuple<
    constr<arrow<value<int>, value<int>>, arrow<var<class X>, var<class Y>>>>;
  static_assert(
    set_eq_v<
      std::tuple<
        tyarrow<var<class X>, value<int>>, tyarrow<var<class Y>, value<int>>>,
      unify_t<constr3>>);

  // [int = int -> Y]
  using constr4 =
    std::tuple<constr<value<int>, arrow<value<int>, var<class Y>>>>;
  // using t4 = unify_t<constr4>;

  // [Y -> int -> Y]
  using constr5 = std::tuple<constr<var<Y>, arrow<value<int>, var<class Y>>>>;
  // using t5 = unify_t<constr5>;

  // []
  using constr6 = std::tuple<>;
  static_assert(set_eq_v<std::tuple<>, unify_t<constr6>>);

  { // apply
    using tm1 = TmApply<TmClosure<TmValue<int>, TmValue<int>>, TmValue<int>>;
    using tp = recon_type_t<tm1>;
    using c = recon_constr_t<tm1>;
    using u = unify_t<c>;
    static_assert(std::is_same_v<value<int>, subst_all_t<u, tp>>);
  }
  { // higher-order
    using tm1 = TmApply<
      TmClosure<TmClosure<TmValue<double>, TmValue<int>>, TmValue<int>>,
      TmClosure<TmValue<double>, TmValue<int>>>;
    using tp = recon_type_t<tm1>;
    using c = recon_constr_t<tm1>;
    using u = unify_t<c>;
    static_assert(std::is_same_v<value<int>, subst_all_t<u, tp>>);
  }
  {
    using tm1 = TmApply<
      TmClosure<TmClosure<TmValue<double>, TmVar<class X>>, TmVar<class X>>,
      TmClosure<TmValue<double>, TmValue<int>>>;
    using tp = recon_type_t<tm1>;
    using c = recon_constr_t<tm1>;
    using u = unify_t<c>;
    static_assert(std::is_same_v<value<int>, subst_all_t<u, tp>>);
  }
  {
    using doubleapp = TmClosure<
      TmClosure<TmVar<class X>, TmVar<class X>>,
      TmClosure<TmVar<class X>, TmVar<class X>>>;
    using tm1 = TmApply<
      TmClosure<TmClosure<TmValue<double>, TmVar<class X>>, TmVar<class X>>,
      TmClosure<TmValue<double>, TmValue<int>>>;
    using tp = recon_type_t<tm1>;
    using c = recon_constr_t<tm1>;
    using u = unify_t<c>;
    static_assert(std::is_same_v<value<int>, subst_all_t<u, tp>>);
  }
  {
    using ff = TmFix<TmClosure<
      TmClosure<TmValue<int>, TmValue<bool>>, TmValue<int>, TmValue<bool>>>;
    static_assert(
      std::is_same_v<type_of_t<ff>, arrow<value<int>, value<bool>>>);
  }
  {
    static_assert(subtype_v<value<HeapObject>, value<Type>>);
    static_assert(subtype_v<
                  arrow<value<Type>, value<Type>>,
                  arrow<value<HeapObject>, value<Type>>>);
    static_assert(subtype_v<
                  arrow<value<Type>, value<HeapObject>>,
                  arrow<value<Type>, value<Type>>>);
  }
  {
    static_assert(
      std::is_same_v<
        type_of_t<TmApply<
          TmClosure<TmValue<HeapObject>, TmValue<Type>>, TmValue<Type>>>,
        value<Type>>);
  }
  { 
    static_assert(std::is_same_v<type_of_t<TmVar<class X>>, var<class X>>); 
  }
}