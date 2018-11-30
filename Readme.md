# Tori
[![Build Status](https://travis-ci.org/mocabe/tori.svg?branch=master)](https://travis-ci.org/mocabe/tori)
[![Build status](https://ci.appveyor.com/api/projects/status/tce0cl19095ynvg3/branch/master?svg=true)](https://ci.appveyor.com/project/mocabe/tori/branch/master)

`Tori` is light-weight, header-only runtime typed object/closure library written in C++17.  

## Feature
- Lazy evaluation
- Run-time/Compile-time type check
- Automatic type information generation
- Simple reference counting GC
- Higher-order closures
- Polymorphic closures
- Custom data types

## Compile
`Tori` is heavily abusing latest c++ features. It requires full support for `c++17`.  

|compiler|version|  
|:--:|:--:|
| `g++` | 7.3 |  
| `clang++` | 6.0.1 |  
| `msvc`    | Visual Studio 2017 15.8 Preview 3 |  

## How does it work?

### Hello, World  

This is simple "Hello, World" example using this library.  
We create a closure object which has a type of `String -> Unit` to print string.  
```cpp
#include <tori/core.hpp>
#include <tori/lib.hpp>
#include <iostream>

// just for this example...
using namespace tori;

// String -> Unit
struct Print : Function<Print, String, Unit> {
  return_type code() const
  {
    // write C++ code
    std::cout << eval_arg<0>()->c_str() << std::endl;
    // return type is checked in compile time
    return new Unit();
  }
};

int main()
{
  // create closure
  auto print = make_object<Print>();
  // apply
  auto apply = print << new String("Hello, World!");
  // runtime type check
  check_type<Unit>(apply);
  // eval
  eval(apply); // Hello, World!
}
```

### Higher order closures

You can also create higher-order closure like this.  
You need to evaluate arguments to access its data.
```cpp
#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <iostream>
// just for this example...
using namespace tori;

// Apply second argument twice to first argument
struct ApplyTwice : Function<ApplyTwice, closure<Int, Int>, Int, Int> {
  return_type code() const
  {
    // arg<1> (arg<0> arg<1>)
    return arg<0>() << (arg<0>() << arg<1>());
  }
};

struct X2 : Function<X2, Int, Int> {
  return_type code() const
  {
    // use eval_arg<0>() instead of arg<0>()
    // to evaluate argument.
    return new Int(*eval_arg<0>() * 2);
  }
};

int main()
{
  // create objects
  auto applyTwice = make_object<ApplyTwice>();
  auto func = make_object<X2>();
  // apply
  auto apply = applyTwice << func << new Int(42);
  // type check
  check_type<Int>(apply);
  // evaluate
  auto result = eval(apply);
  // use value_cast<T>() for dynamic cast.
  std::cout << *value_cast<Int>(result) << std::endl; // 168
}
```

### Polymorphic closures

You can also create polymorphic typed closures!  
This is simple implementation of `if` function.
```cpp
#include <tori/core.hpp>
#include <tori/lib.hpp>

#include <iostream>
// just for this example...
using namespace tori;

// Bool -> X -> X -> X
struct If
  : Function<::If, Bool, forall<class X>, forall<class X>, forall<class X>> {
  return_type code() const
  {
    if (*eval_arg<0>())
      return arg<1>();
    else
      return arg<2>();
  }
};

int main()
{
  // create objects
  auto if_ = make_object<::If>();
  auto b = make_object<Bool>(true);
  auto i1 = make_object<Int>(42);
  auto i2 = make_object<Int>(24);

  // apply
  auto app = if_ << b << i1 << i2;
  // check type
  check_type<Int>(app);
  // result
  auto result = eval(app);
  assert(*value_cast<Int>(result) == 42);
}
```

### Custom data types
You can create your own data types.  
```cpp
struct MyVector {
  float_t x,y,z;
};

// define custom data type
using MyVectorObject = BoxedHeapObject<MyVector>;

template <>
struct tori::object_type_traits<MyVectorObject> {
    // define unique name of your type here
    // type name starting with underscore(_) is reserved.
    static constexpr const char[] name = "MyVectorObject";
  };
}

// create object
auto myVectorObject = make_object<MyVectorObject>(1.f,2.f,3.f);
```
