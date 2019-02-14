// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

#pragma once

#include "../config/config.hpp"
#include "object_ptr.hpp"

namespace TORI_NS::detail {

  namespace interface {

    /// Object info table
    struct object_info_table
    {
      /// pointer to type object
      object_ptr<const Type> obj_type;
      /// total size of object
      uint64_t obj_size;
      /// size of additional storage
      uint64_t obj_ext_bytes;
      /// vtable of delete function
      void (*destroy)(const Object*);
      /// vtable of clone function
      Object* (*clone)(const Object*);
    };

  } // namespace interface

} // namespace TORI_NS::detail
