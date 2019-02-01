#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file String

#include "type_gen.hpp"

#include <string>
#include <cstring>
#include <string>

// utf8 macro
#define utf8(Str) ::TORI_NS::detail::to_u8(u8##Str)

namespace TORI_NS::detail {

  class StringValue
  {
  public:
    StringValue(const std::string& str)
    {
      m_ptr = new uint8_t[str.size() + 1];
      std::copy(str.c_str(), str.c_str() + str.size() + 1, m_ptr);
    }

    StringValue(const StringValue& other)
    {
      auto len = std::strlen(other.c_str());
      m_ptr = new uint8_t[len + 1];
      std::copy(other.m_ptr, other.m_ptr + len + 1, m_ptr);
    }

    StringValue(StringValue&& other)
    {
      m_ptr = other.m_ptr;
      other.m_ptr = nullptr;
    }

    ~StringValue() noexcept
    {
      delete[] m_ptr;
    }

    /// c_str
    const char* c_str() const noexcept
    {
      return reinterpret_cast<const char*>(m_ptr);
    }

  private:
    uint8_t* m_ptr;
  };

  namespace interface {

    /// UTF-8 String object.
    /// Does not guarantee anything about encoding. User must ensure
    /// input byte sequence is null(`0x00`)-terminated UTF-8 string.
    using String = Box<StringValue>;

  } // namespace interface

} // namespace TORI_NS::detail

// String
TORI_DECL_TYPE(String)