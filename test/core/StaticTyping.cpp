#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <iostream>

using namespace tori::detail;

void test_unify()
{
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
    static_assert(set_eq_v<
                  std::tuple<
                    tyarrow<var<class X>, value<int>>,
                    tyarrow<var<class Y>, value<int>>>,
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

void test_type_of()
{
  {
    // apply
    // (Int -> Int) Int = Int
    using tm1 =
      tm_apply<tm_closure<tm_value<int>, tm_value<int>>, tm_value<int>>;
    static_assert(std::is_same_v<value<int>, type_of_t<tm1>>);
  }
  {
    // higher-order
    // (Double -> Int -> Int) (Double -> Int) = Int
    using tm1 = tm_apply<
      tm_closure<tm_closure<tm_value<double>, tm_value<int>>, tm_value<int>>,
      tm_closure<tm_value<double>, tm_value<int>>>;
    static_assert(std::is_same_v<value<int>, type_of_t<tm1>>);
  }
  {
    // (Double -> X -> X) (Double -> Int) = Int
    using tm1 = tm_apply<
      tm_closure<
        tm_closure<tm_value<double>, tm_var<class X>>,
        tm_var<class X>>,
      tm_closure<tm_value<double>, tm_value<int>>>;
    static_assert(std::is_same_v<value<int>, type_of_t<tm1>>);
  }
  {
    // ((X -> X) -> (X -> X)) (Int -> Int) = Int -> Int
    using doubleapp = tm_closure<
      tm_closure<tm_var<class X>, tm_var<class X>>,
      tm_closure<tm_var<class X>, tm_var<class X>>>;
    using tm = tm_apply<doubleapp, tm_closure<tm_value<int>, tm_value<int>>>;
    static_assert(std::is_same_v<arrow<value<int>, value<int>>, type_of_t<tm>>);
  }
  {
    // fix ((int -> bool) -> (int -> bool)) = int -> bool
    using ff = tm_apply<
      tm_fix<Fix>,
      tm_closure<
        tm_closure<tm_value<int>, tm_value<bool>>,
        tm_value<int>,
        tm_value<bool>>>;
    static_assert(
      std::is_same_v<type_of_t<ff>, arrow<value<int>, value<bool>>>);
  }
  {
    static_assert(std::is_same_v<type_of_t<tm_var<class X>>, var<class X>>);
  }
  {
    // (Int->X) X = X
    using term =
      tm_apply<tm_closure<tm_value<Int>, tm_var<class X>>, tm_value<Int>>;
    static_assert(std::is_same_v<var<taggen<0>>, type_of_t<term>>);
  }
  {
    // (Int -> Double) Int = Double
    using term =
      tm_apply<tm_closure<tm_value<Int>, tm_value<Double>>, tm_value<Int>>;
    static_assert(std::is_same_v<value<Double>, type_of_t<term>>);
  }
  {
    // (X -> Y -> Int) Double -> Z(placeholder)
    using term = tm_apply<
      tm_apply<tm_closure<tm_var<class XX>, tm_var<class YY>>, tm_value<Int>>,
      tm_value<Double>>;
    static_assert(std::is_same_v<var<taggen<3>>, type_of_t<term>>);
  }
}

void test_genpoly()
{
  {
    // Double -> Var<X> -> Var<X>
    using term = remove_varvalue_t<tm_closure<
      tm_closure<tm_value<double>, tm_varvalue<class X>>,
      tm_varvalue<class X>>>;
    // Double -> Var[0] -> Var[0]
    using gterm = tm_closure<
      tm_closure<tm_value<double>, tm_var<taggen<0>>>,
      tm_var<taggen<0>>>;
    static_assert(std::is_same_v<genpoly_term<term, taggen<0>>, gterm>);
  }
}

void test_assume_object_type()
{
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
    static_assert(std::is_same_v<
                  assume_object_type_t<arrow<
                    arrow<value<Double>, value<Int>>,
                    arrow<value<Double>, value<Int>>>>,
                  closure<closure<Double, Int>, Double, Int>>);
  }
}

int main()
{
  test_unify();
  test_genpoly();
  test_assume_object_type();
  test_type_of();
}