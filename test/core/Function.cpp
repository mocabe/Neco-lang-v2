#include <iostream>
#include <tori/core.hpp>
#include <tori/lib.hpp>

using namespace tori;

void simple() {
  {
    // simple
    struct F1 : Function<F1, Int, Int> {
      ReturnType code() const {
        return arg<0>();
        return eval_arg<0>();
        return eval(arg<0>());
        return eval(eval_arg<0>());
      }
    };
    auto f1 = make_object<F1>();
  }
}
void higher_order() {
  {
    // higher-order
    struct F2 : Function<F2, closure<Int, Int>, closure<Int, Int>> {
      ReturnType code() const {
        return arg<0>();
        return eval_arg<0>();
        return eval(arg<0>());
        return eval(eval_arg<0>());
      }
    };
    auto f2 = make_object<F2>();
  }
}

void apply() {
  {
    // static apply
    struct F3 : Function<F3, closure<Int, Int>, Int, Int> {
      ReturnType code() const {
        return arg<0>() << arg<1>();
        return eval_arg<0>() << arg<1>();
        return arg<0>() << eval_arg<1>();
        return eval_arg<0>() << eval_arg<1>();
        return eval(arg<0>() << arg<1>());

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

void polymorphic() {
  {
    // polymorphic closure declaration
    struct F4
      : Function<F4, Int, closure<Int, forall<class F4_X>>, forall<class F4_X>> {
      ReturnType code() const {
        return arg<1>() << arg<0>();
      }
    };
    auto f4 = make_object<F4>();
  }
  {
    // polymorphic return type
    struct F5If
      : Function<F5If, Bool, forall<class X>, forall<class X>, forall<class X>> {
      ReturnType code() const {
        if (*eval_arg<0>())
          return arg<1>();
        else
          return arg<2>();
      }
    };
    auto f5if = make_object<F5If>();

    // static polymorhic apply
    struct F5_0 : Function<F5_0, Unit, Int> {
      ReturnType code() const {
        auto _if = make_object<F5If>();
        auto b = make_object<Bool>();
        auto i = make_object<Int>();
        return _if << b << i << i;
      }
    };
    auto f5_0 = make_object<F5_0>();

    // static polymorphic apply
    struct F5_1 : Function<F5_1, closure<Double, Int, Int>, Int> {
      ReturnType code() const {
        auto _if = make_object<F5If>();
        auto b = make_object<Bool>();
        auto i = make_object<Int>();
        auto d = make_object<Double>();
        return arg<0>() << (_if << b << d << d) << (_if << b << i << i);
      }
    };
    auto f5_1 = make_object<F5_1>();
  }
}

int main() {
  simple();
  higher_order();
  apply();
  polymorphic();
}