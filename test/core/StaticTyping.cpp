#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <iostream>

using namespace tori::detail;

void test_unify() {
  {
    // [X -> int]
    using c = std::tuple<constr<var<class X>, value<int>>>;
    static_assert(
      set_eq_v<std::tuple<tyarrow<var<class X>, value<int>>>, unify_t<c>>);
  }
  {
    // [X = int, Y = X -> X]
    using c = std::tuple<
      constr<var<class X>, value<int>>,
      constr<var<class Y>, arrow<var<class X>, var<class X>>>>;
    static_assert(set_eq_v<
                  std::tuple<
                    tyarrow<var<class X>, value<int>>,
                    tyarrow<var<class Y>, arrow<value<int>, value<int>>>>,
                  unify_t<c>>);
  }
  {
    // [int->int = X -> Y]
    using c = std::tuple<
      constr<arrow<value<int>, value<int>>, arrow<var<class X>, var<class Y>>>>;
    static_assert(
      set_eq_v<
        std::tuple<
          tyarrow<var<class X>, value<int>>, tyarrow<var<class Y>, value<int>>>,
        unify_t<c>>);
  }
  {
    // [int = int -> Y]
    using c = std::tuple<constr<value<int>, arrow<value<int>, var<class Y>>>>;
    // using r = unify_t<c>; // should fail
  }
  {
    // [Y = int -> Y]
    using c = std::tuple<constr<var<class Y>, arrow<value<int>, var<class Y>>>>;
    // using r = unify_t<c>; // should fail
  }
  {
    // []
    using c = std::tuple<>;
    static_assert(set_eq_v<std::tuple<>, unify_t<c>>);
  }
}

void test_type_of() {
  {
    // apply
    // (Int -> Int) Int = Int
    using tm1 = TmApply<TmClosure<TmValue<int>, TmValue<int>>, TmValue<int>>;
    static_assert(std::is_same_v<value<int>, type_of_t<tm1>>);
  }
  {
    // higher-order
    // (Double -> Int -> Int) (Double -> Int) = Int
    using tm1 = TmApply<
      TmClosure<TmClosure<TmValue<double>, TmValue<int>>, TmValue<int>>,
      TmClosure<TmValue<double>, TmValue<int>>>;
    static_assert(std::is_same_v<value<int>, type_of_t<tm1>>);
  }
  {
    // (Double -> X -> X) (Double -> Int) = Int
    using tm1 = TmApply<
      TmClosure<TmClosure<TmValue<double>, TmVar<class X>>, TmVar<class X>>,
      TmClosure<TmValue<double>, TmValue<int>>>;
    static_assert(std::is_same_v<value<int>, type_of_t<tm1>>);
  }
  {
    // ((X -> X) -> (X -> X)) (Int -> Int) = Int -> Int
    using doubleapp = TmClosure<
      TmClosure<TmVar<class X>, TmVar<class X>>,
      TmClosure<TmVar<class X>, TmVar<class X>>>;
    using tm = TmApply<doubleapp, TmClosure<TmValue<int>, TmValue<int>>>;
    static_assert(std::is_same_v<arrow<value<int>, value<int>>, type_of_t<tm>>);
  }
  {
    // fix ((int -> bool) -> (int -> bool)) = int -> bool
    using ff = TmApply<
      TmFix<Fix>,
      TmClosure<
        TmClosure<TmValue<int>, TmValue<bool>>, TmValue<int>, TmValue<bool>>>;
    static_assert(
      std::is_same_v<type_of_t<ff>, arrow<value<int>, value<bool>>>);
  }
  { static_assert(std::is_same_v<type_of_t<TmVar<class X>>, var<class X>>); }
  {
    // (Int->X) X = X
    using term = TmApply<TmClosure<TmValue<Int>, TmVar<class X>>, TmValue<Int>>;
    static_assert(std::is_same_v<var<taggen<0>>, type_of_t<term>>);
  }
  {
    // (Int -> Double) Int = Double
    using term =
      TmApply<TmClosure<TmValue<Int>, TmValue<Double>>, TmValue<Int>>;
    static_assert(std::is_same_v<value<Double>, type_of_t<term>>);
  }
  {
    // (X -> Y -> Int) Double -> Z(placeholder)
    using term = TmApply<
      TmApply<TmClosure<TmVar<class XX>, TmVar<class YY>>, TmValue<Int>>,
      TmValue<Double>>;
    static_assert(std::is_same_v<var<taggen<3>>, type_of_t<term>>);
  }
}

void test_genpoly() {
  {
    // Double -> Var<X> -> Var<X>
    using term = remove_varvalue_t<TmClosure<
      TmClosure<TmValue<double>, TmVarValue<class X>>, TmVarValue<class X>>>;
    // Double -> Var[0] -> Var[0]
    using gterm =
      TmClosure<TmClosure<TmValue<double>, TmVar<taggen<0>>>, TmVar<taggen<0>>>;
    static_assert(std::is_same_v<genpoly_term<term, taggen<0>>, gterm>);
  }
}

void test_assume_object_type() {
  {
    // value<T> -> T
    static_assert(std::is_same_v<assume_object_type_t<value<Int>>, Int>);
  }
  {
    // var<class T> -> HeapObject
    static_assert(
      std::is_same_v<assume_object_type_t<var<class Tag>>, HeapObject>);
  }
  {
    // arrow<S, T> -> closure<S, T>
    static_assert(std::is_same_v<
                  assume_object_type_t<arrow<value<Double>, value<Int>>>,
                  closure<Double, Int>>);
    // arrow<S, arrow<T, U>> -> closure<S, T, U>
    static_assert(std::is_same_v<
                  assume_object_type_t<
                    arrow<value<Int>, arrow<value<Double>, value<Int>>>>,
                  closure<Int, Double, Int>>);
    // arrow<arrow<Double, Int>, arrow<Double, Int>> -> closure<closure<Double,
    // Int>, Double, Int>
    static_assert(
      std::is_same_v<
        assume_object_type_t<arrow<
          arrow<value<Double>, value<Int>>, arrow<value<Double>, value<Int>>>>,
        closure<closure<Double, Int>, Double, Int>>);
  }
}

int main() {
  test_unify();
  test_genpoly();
  test_assume_object_type();
  test_type_of();
}