#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <catch2/catch.hpp>

#include <iostream>

using namespace tori::detail;

TEST_CASE("subst_type")
{

  SECTION("[X->Int] X == Int")
  {
    auto var = genvar();
    auto tyarrow = TyArrow {var, object_type<Int>()};
    REQUIRE(same_type(subst_type(tyarrow, var), object_type<Int>()));
  }

  SECTION("[X->Int]X->Int == Int->Int")
  {
    auto var = genvar();
    auto tyarrow = TyArrow {var, object_type<Int>()};
    auto type = make_object<Type>(ArrowType {var, object_type<Int>()});
    REQUIRE(same_type(
      subst_type(tyarrow, type),
      make_object<Type>(ArrowType {object_type<Int>(), object_type<Int>()})));
  }
}

TEST_CASE("subst_type_all")
{
  { // (Y->X) -> (Float->Y) [X->Int, Y->Float] => (Float->Int) -> (Float->Float)
    auto X = genvar();
    auto Y = genvar();
    std::vector tyarrows = {TyArrow {X, object_type<Int>()},
                            TyArrow {Y, object_type<Float>()}};
    auto type = make_object<Type>(
      ArrowType {new Type(ArrowType {Y, X}),
                 new Type(ArrowType {object_type<Float>(), Y})});

    REQUIRE(same_type(
      subst_type_all(tyarrows, type),
      make_object<Type>(ArrowType {
        new Type(ArrowType {object_type<Float>(), object_type<Int>()}),
        new Type(ArrowType {object_type<Float>(), object_type<Float>()})})));
  }
}

TEST_CASE("unify")
{

  // compare two TyArrow vectors
  auto eq_arrows = [](auto&& result, auto&& ans) {
    if (result.size() != ans.size())
      return false;
    for (auto&& r : result) {
      auto b = [&]() {
        for (auto&& a : ans) {
          if (same_type(r.from, a.from) && same_type(r.to, a.to))
            return true;
        }
        return false;
      }();
      if (!b)
        return false;
    }
    return true;
  };

  auto print_tyarrows = [](auto&& tyarrows) {
    std::cout << "[";
    for (auto&& ta : tyarrows) {
      std::cout << to_string(ta.from) << "->" << to_string(ta.to) << ", ";
    }
    std::cout << "]\n";
  };

  auto X = genvar();
  auto Y = genvar();
  auto Z = genvar();
  auto U = genvar();
  auto W = genvar();

  SECTION("[X=int]")
  {
    std::vector cs = {Constr {X, object_type<Int>()}};
    auto result = unify(cs, nullptr);

    SECTION("reunify")
    {
      for (auto&& ta : result) {
        cs = subst_constr_all(ta, cs);
      }
      REQUIRE(unify(cs, nullptr).empty());
    }

    SECTION("eq")
    {
      std::vector ans = {TyArrow {X, object_type<Int>()}};
      print_tyarrows(result);
      print_tyarrows(ans);
      CHECK(eq_arrows(result, ans));
    }
  }

  SECTION("[X=Int, Y=X->X]")
  {
    std::vector cs = {Constr {X, object_type<Int>()},
                      Constr {Y, new Type(ArrowType {X, X})}};
    std::vector result = unify(cs, nullptr);

    SECTION("reunify")
    {
      for (auto&& ta : result) {
        cs = subst_constr_all(ta, cs);
      }
      REQUIRE(unify(cs, nullptr).empty());
    }

    SECTION("eq")
    {
      std::vector ans = {
        TyArrow {X, object_type<Int>()},
        TyArrow {Y,
                 new Type(ArrowType {object_type<Int>(), object_type<Int>()})}};

      print_tyarrows(result);
      print_tyarrows(ans);
      CHECK(eq_arrows(result, ans));
    }
  }

  SECTION("[Int-Int=X->Y]")
  {
    std::vector cs = {
      Constr {new Type(ArrowType {object_type<Int>(), object_type<Int>()}),
              new Type(ArrowType {X, Y})}};
    std::vector result = unify(cs, nullptr);

    SECTION("reunify")
    {
      for (auto&& ta : result) {
        cs = subst_constr_all(ta, cs);
      }
      REQUIRE(unify(cs, nullptr).empty());
    }

    SECTION("eq")
    {
      std::vector ans = {TyArrow {X, object_type<Int>()},
                         TyArrow {Y, object_type<Int>()}};
      print_tyarrows(result);
      print_tyarrows(ans);
      CHECK(eq_arrows(result, ans));
    }
  }

  SECTION("[Int->Int=X->Y]")
  {
    std::vector cs = {
      Constr {new Type(ArrowType {X, Y}), new Type(ArrowType {Y, Z})},
      Constr {Z, new Type(ArrowType {U, W})}};
    auto result = unify(cs, nullptr);

    SECTION("reunify")
    {
      for (auto&& ta : result) {
        cs = subst_constr_all(ta, cs);
      }
      REQUIRE(unify(cs, nullptr).empty());
    }

    SECTION("eq")
    {
      std::vector ans = {TyArrow {X, new Type(ArrowType {U, W})},
                         TyArrow {Y, new Type(ArrowType {U, W})},
                         TyArrow {Z, new Type(ArrowType {U, W})}};
      print_tyarrows(result);
      print_tyarrows(ans);
      CHECK(eq_arrows(result, ans));
    }
  }

  SECTION("[]")
  {
    std::vector<Constr> cs;
    auto result = unify(cs, nullptr);
    REQUIRE(result.empty());
  }
}

TEST_CASE("type_of")
{
  SECTION("type")
  {
    auto t = genvar();
    auto type = type_of(t);
    REQUIRE(same_type(type, object_type<Type>()));
  }

  SECTION("value")
  {
    auto i = make_object<Int>();
    auto type = type_of(i);
    REQUIRE(same_type(type, object_type<Int>()));
  }

  SECTION("arrow")
  {
    {
      struct A : Function<A, closure<Int, Int>, Int>
      {
        return_type code() const
        {
        }
      };
      auto a = make_object<A>();
      auto type = type_of(a);
      REQUIRE(same_type(type, object_type<closure<closure<Int, Int>, Int>>()));
    }
  }
  SECTION("apply")
  {
    SECTION("simple")
    {
      struct A : Function<A, Int, Double>
      {
        return_type code() const
        {
        }
      };

      auto a = make_object<A>();
      auto i = make_object<Int>();

      SECTION("Apply")
      {
        object_ptr app = new Apply {a, i};
        auto type = type_of(app);
        REQUIRE(same_type(type, object_type<Double>()));
      }

      SECTION("ApplyR")
      {
        object_ptr app = make_object<ApplyR>(a, i);
        auto type = type_of(app);
        REQUIRE(same_type(type, object_type<Double>()));
      }

      SECTION("operator<<")
      {
        auto app = a << i;
        auto type = type_of(app);
        REQUIRE(same_type(type, object_type<Double>()));
      }
    }
  }
  SECTION("higher-order")
  {
    struct A : Function<A, closure<Double, Int>, Double, Int>
    {
      return_type code() const
      {
      }
    };

    struct B : Function<B, Double, Int>
    {
      return_type code() const
      {
      }
    };

    auto a = make_object<A>();
    auto b = make_object<B>();
    auto i = make_object<Int>();

    SECTION("Apply")
    {
      object_ptr app = new Apply {new Apply {a, b}, i};
      auto type = type_of(app);
      REQUIRE(same_type(type, object_type<Int>()));
    }

    SECTION("operator<<")
    {
      auto app = a << b << i;
      auto type = type_of(app);
      REQUIRE(same_type(type, object_type<Int>()));
    }
  }

  SECTION("polymorphic")
  {
    struct A : Function<A, Bool, forall<class X>, forall<class X>>
    {
      return_type code() const
      {
      }
    };

    auto a = make_object<A>();
    auto b = make_object<Bool>();
    auto i = make_object<Int>();
    auto d = make_object<Double>();

    SECTION("mono")
    {
      auto app = a << b << i << i;
      auto type = type_of(app);
      REQUIRE(same_type(type, object_type<Int>()));
    }

    SECTION("poly")
    {
      auto app = a << b << (a << b << d << d) << d;
      auto type = type_of(app);
      REQUIRE(same_type(type, object_type<Double>()));
    }
  }

  SECTION("fix")
  {

    struct A : Function<A, closure<Int, Double>, closure<Int, Double>>
    {
      return_type code() const
      {
      }
    };

    struct B : Function<B, closure<Int, Double>, Int, Double>
    {
      return_type code() const
      {
      }
    };

    struct C : Function<C, Int, Double, Int, Double>
    {
      return_type code() const
      {
      }
    };

    auto a = make_object<A>();
    auto b = make_object<B>();
    auto c = make_object<C>();
    auto fix = make_object<Fix>();

    SECTION("a")
    {
      auto app = fix << a;
      auto type = type_of(app);
      REQUIRE(same_type(type, object_type<closure<Int, Double>>()));
    }

    SECTION("b")
    {
      auto app = fix << b;
      auto type = type_of(app);
      REQUIRE(same_type(type, object_type<closure<Int, Double>>()));
    }

    SECTION("c")
    {
      auto app = fix << c;
      auto type = type_of(app);
      REQUIRE(same_type(type, object_type<closure<Int, Double>>()));
    }
  }
}
