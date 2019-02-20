#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <catch2/catch.hpp>
#include <iostream>

using namespace tori;

TEST_CASE("simple function test")
{
  SECTION("Int->Int")
  {
    struct F : Function<F, Int, Int>
    {
      return_type code() const
      {
        return arg<0>();
        return eval_arg<0>();
        return eval(arg<0>());
        return eval(eval_arg<0>());
      }
    };
    auto f = make_object<F>();
  }
}

TEST_CASE("higher order function test")
{
  SECTION("(Int->Int) -> (Int->Int)")
  {
    struct F : Function<F, closure<Int, Int>, closure<Int, Int>>
    {
      return_type code() const
      {
        return arg<0>();
        return eval_arg<0>();
        return eval(arg<0>());
        return eval(eval_arg<0>());
      }
    };
    auto f = make_object<F>();
  }
}

TEST_CASE("static apply test")
{
  SECTION("")
  {
    struct F : Function<F, closure<Int, Int>, Int, Int>
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
    auto f = make_object<F>();
  }
}

TEST_CASE("polymorphic function test")
{
  SECTION("polymorphic closure declaration")
  {
    class X;
    struct F : Function<F, Int, closure<Int, forall<X>>, forall<X>>
    {
      return_type code() const
      {
        return arg<1>() << arg<0>();
      }
    };
    auto f4 = make_object<F>();
  }

  SECTION("polymorphic return type")
  {
    class X;
    struct If : Function<If, Bool, forall<X>, forall<X>, forall<X>>
    {
      return_type code() const
      {
        if (*eval_arg<0>())
          return arg<1>();
        else
          return arg<2>();
      }
    };
    auto _if = make_object<If>();

    SECTION("static polymorphic apply")
    {
      struct F : Function<F, Unit, Int>
      {
        return_type code() const
        {
          auto _if = make_object<If>();
          auto b = make_object<Bool>();
          auto i = make_object<Int>();
          return _if << b << i << i; // Bool->X->X->X Bool Int Int
        }
      };
      auto f = make_object<F>();
    }

    SECTION("static polymorphic apply 2")
    {
      // static polymorphic apply
      struct F : Function<F, closure<Double, Int, Int>, Int>
      {
        return_type code() const
        {
          auto _if = make_object<If>();
          auto b = make_object<Bool>();
          auto i = make_object<Int>();
          auto d = make_object<Double>();
          // (Double->Int->Int) ((Bool->X->X->X) Bool Double Double)
          // ((Bool->X->X->X) Bool Int Int)
          return arg<0>() << (_if << b << d << d) << (_if << b << i << i);
        }
      };
      auto f = make_object<F>();
    }
  }
}

/* FIXME
TEST_CASE("selfrec")
{

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
}
*/