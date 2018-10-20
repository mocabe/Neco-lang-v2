#pragma once

// Copyright (c) 2018 mocabe(https://github.com/mocabe)
// This code is licensed under MIT license.

/// \file String

#include "DynamicTypeUtil.hpp"
#include <string>
#include <cstring>
#include <string>

namespace TORI_NS::detail {
  class StringValue {
  public:
    StringValue(const std::string& str) {
      m_ptr = new uint8_t[str.size() + 1];
      std::copy(str.c_str(), str.c_str() + str.size() + 1, m_ptr);
    }
    StringValue(const StringValue& other) {
      auto len = std::strlen(other.c_str());
      m_ptr = new uint8_t[len + 1];
      std::copy(other.m_ptr, other.m_ptr + len + 1, m_ptr);
    }
    StringValue(StringValue&& other) {
      m_ptr = other.m_ptr;
      other.m_ptr = nullptr;
    }
    ~StringValue() noexcept {
      delete[] m_ptr;
    }
    const char* c_str() const noexcept {
      return (const char*)m_ptr;
    }

  private:
    uint8_t* m_ptr;
  };

  namespace interface {
    /// String
    using String = BoxedHeapObject<StringValue>;
  } // namespace interface

} // namespace TORI_NS::detail

namespace TORI_NS {
  TORI_DECL_TYPE(String)
} // namespace TORI_NS
