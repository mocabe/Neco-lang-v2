#include <iostream>
#include <tori/core.hpp>
#include <tori/lib.hpp>

using namespace tori;

// simple function test
void simple()
{
  {
    // Int -> Int
    struct F1 : Function<F1, Int, Int>
    {
      return_type code() const
      {
        return arg<0>();
        return eval_arg<0>();
        return eval(arg<0>());
        return eval(eval_arg<0>());
      }
    };
    auto f1 = make_object<F1>();
  }
}
// higher order function test
void higher_order()
{
  {
    // (Int->Int) -> (Int->Int)
    struct F2 : Function<F2, closure<Int, Int>, closure<Int, Int>>
    {
      return_type code() const
      {
        return arg<0>();
        return eval_arg<0>();
        return eval(arg<0>());
        return eval(eval_arg<0>());
      }
    };
    auto f2 = make_object<F2>();
  }
}

// static apply test
void apply()
{
  {
    struct F3 : Function<F3, closure<Int, Int>, Int, Int>
    {
      return_type code() const
      {
        // (Int->Int)Int
        return arg<0>() << arg<1>();
        return eval_arg<0>() << arg<1>();
        return arg<0>() << eval_arg<1>();
        return eval_arg<0>() << eval_arg<1>();
        return eval(arg<0>() << arg<1>());

        // (Int->Int)((Int->Int)Int)
        return arg<0>() << (arg<0>() << arg<1>());
        return arg<0>() << eval(arg<0>() << arg<1>());
        return arg<0>() << (eval_arg<0>() << arg<1>());
        return eval_arg<0>() << (arg<0>() << arg<1>());
        return eval_arg<0>() << eval(arg<0>() << arg<1>());
        return eval_arg<0>() << (eval_arg<0>() << arg<1>());
      }
    };
    auto f3 = make_object<F3>();
  }
}

// polymorphic function test
void polymorphic()
{
  {
    class X;
    // polymorphic closure declaration
    struct F4 : Function<F4, Int, closure<Int, forall<X>>, forall<X>>
    {
      return_type code() const
      {
        return arg<1>() << arg<0>();
      }
    };
    auto f4 = make_object<F4>();
  }
  {
    class X;
    // polymorphic return type
    struct F5If : Function<F5If, Bool, forall<X>, forall<X>, forall<X>>
    {
      return_type code() const
      {
        if (*eval_arg<0>())
          return arg<1>();
        else
          return arg<2>();
      }
    };
    auto f5if = make_object<F5If>();

    // static polymorhic apply
    struct F5_0 : Function<F5_0, Unit, Int>
    {
      return_type code() const
      {
        auto _if = make_object<F5If>();
        auto b = make_object<Bool>();
        auto i = make_object<Int>();
        return _if << b << i << i; // Bool->X->X->X Bool Int Int
      }
    };
    auto f5_0 = make_object<F5_0>();

    // static polymorphic apply
    struct F5_1 : Function<F5_1, closure<Double, Int, Int>, Int>
    {
      return_type code() const
      {
        auto _if = make_object<F5If>();
        auto b = make_object<Bool>();
        auto i = make_object<Int>();
        auto d = make_object<Double>();
        // (Double->Int->Int) ((Bool->X->X->X) Bool Double Double)
        // ((Bool->X->X->X) Bool Int Int)
        return arg<0>() << (_if << b << d << d) << (_if << b << i << i);
      }
    };
    auto f5_1 = make_object<F5_1>();
  }
}

/*
void selfrec() {
  
  // f = λx.f x
  struct R : Function<R, Int, Int>
  {
    return_type code() const
    {
      return make_object<R>() << arg<0>();
    }
  };

  auto sr = make_object<R>() << new Int(42);
  check_type<Int>(sr);
  auto r = eval(sr);
  std::vector<double> a;
}
*/
int main()
{
  simple();
  higher_order();
  apply();
  polymorphic();
 // selfrec();
}