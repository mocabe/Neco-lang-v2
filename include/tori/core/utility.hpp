// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"

#include "object_ptr.hpp"
#include "object_cast.hpp"

namespace TORI_NS::detail {

  namespace interface {

    /// Clone
    /// \effects Call copy constructor of the object from vtable.
    /// \returns `object_ptr<T>` pointing new object.
    /// \throws `std::bad_alloc` when `clone` returned nullptr.
    /// \throws `std::runtime_error` when object is null.
    /// \notes Reference count of new object will be set to 1.
    /// \requires not null.
    template <class T>
    object_ptr<T> clone(const object_ptr<T>& obj)
    {
      assert(obj);

      object_ptr tmp = obj.info_table()->clone(obj.get());

      if (unlikely(!tmp))
        throw std::bad_alloc();

      return static_object_cast<T>(std::move(tmp));
    }

    /// check type
    template <class T, class U>
    void check_type(const object_ptr<U>& obj)
    {
      auto t1 = object_type<T>();
      auto t2 = type_of(obj);
      if (unlikely(!same_type(t1, t2)))
        throw type_error::bad_type_check(t1, t2, obj);
    }

  } // namespace interface

} // namespace TORI_NS::detail