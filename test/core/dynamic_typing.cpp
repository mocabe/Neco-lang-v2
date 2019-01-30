
#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <iostream>
int main()
{
  using namespace tori::detail;
  {
    auto X = genvar();
    auto Y = genvar();
    std::vector<Constr> cs = {{X, object_type<Int>()},
                              {Y, new Type(ArrowType {X, X})}};
    auto ta = unify(cs, nullptr);
    std::cout << std::endl;
    std::cout << "{X = Int, Y = X->X}" << std::endl;
    for (auto a : ta) {
      std::cout << "TyArrow{" << to_string(a.from) << " -> " << to_string(a.to)
                << "}\n";
    }
  }
  {
    auto X = genvar();
    auto Y = genvar();
    std::vector<Constr> cs = {
      {new Type(ArrowType {object_type<Int>(), object_type<Int>()}),
       new Type(ArrowType {X, Y})}};
    auto ta = unify(cs, nullptr);
    std::cout << std::endl;
    std::cout << "{Int->Int = X->Y}" << std::endl;
    for (auto a : ta) {
      std::cout << "TyArrow{" << to_string(a.from) << " -> " << to_string(a.to)
                << "}\n";
    }
  }
  {
    auto X = genvar();
    auto Y = genvar();
    auto Z = genvar();
    auto U = genvar();
    auto W = genvar();
    std::vector<Constr> cs = {
      {new Type(ArrowType {X, Y}), new Type(ArrowType {Y, Z})},
      {Z, new Type(ArrowType {U, W})}};
    auto ta = unify(cs, nullptr);
    std::cout << std::endl;
    std::cout << "{X->Y = Y->Z, Z = U->W}" << std::endl;
    for (auto a : ta) {
      std::cout << "TyArrow{" << to_string(a.from) << " -> " << to_string(a.to)
                << "}\n";
    }
  }
  {
    auto X = genvar();
    std::vector<Constr> cs = {
      {new Type(ArrowType {object_type<Int>(), object_type<Double>()}),
       new Type(ArrowType {object_type<Int>(), X})}};
    auto ta = unify(cs, nullptr);
    std::cout << std::endl;
    std::cout << "{Int->Dobule = Int->genvar[0]}" << std::endl;
    for (auto a : ta) {
      std::cout << "TyArrow{" << to_string(a.from) << " -> " << to_string(a.to)
                << "}\n";
    }
  }
  {
    std::vector<Constr> cs = {};
    auto ta = unify(cs, nullptr);
    assert(ta.empty());
  }
  {
    auto Y = genvar();
    std::vector<Constr> cs = {
      {object_type<Int>(), new Type(ArrowType {object_type<Int>(), Y})}};
    try {
      std::cout << "hey" << std::endl;
      auto ta = unify(cs, nullptr);
      assert(false);
    } catch (type_error::type_error&) {
    }
  }
  {
    auto Y = genvar();
    std::vector<Constr> cs = {{Y, new Type(ArrowType {object_type<Int>(), Y})}};
    try {
      std::cout << "hey" << std::endl;
      auto ta = unify(cs, nullptr);
      assert(false);
    } catch (type_error::type_error&) {
    }
  }
  {
    struct F : Function<F, closure<Int, Int>, Int, Int>
    {
      return_type code() const
      {
        return new Int();
      }
    };
    auto f = make_object<F>();
    auto app = make_object<Fix>() << f << new Int();
    check_type<Int>(app);
  }
  {
    try {
      auto app = make_object<Fix>() << new Int();
      auto tp = type_of(app);
      assert(false);
    } catch (type_error::type_error&) {
    }
  }
  {
    struct ThunkTest : Function<ThunkTest, Unit, Int>
    {
      return_type code() const
      {
        std::cout << "ThunkTest evaluated" << std::endl;
        return new Int(21);
      }
    };
    auto test = make_object<ThunkTest>();
    auto add = make_object<PlusInt>();
    // (a -> a) ((Unit -> Int) Unit)
    auto appThunk = test << new Unit();
    check_type<Int>(appThunk);
    auto app = add << appThunk << appThunk;
    check_type<Int>(app);
    auto result = eval(app);
    assert(*value_cast<Int>(result) == 42);
  }
  {
    object_ptr<Int> i = new Int(42);
    object_ptr<const Int> ci = i;
  }
  {
    std::shared_ptr<int> i = std::make_shared<int>(42);
    std::shared_ptr<const int> ci = i;
  }
}