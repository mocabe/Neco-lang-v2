#pragma once

#include "DynamicTypeUtil.hpp"
#include <string>
#include <cstring>
#include <string>

namespace TORI_NS::detail {
  class StringValue {
  public:
    StringValue(const std::string& str) {
      m_ptr = new char[str.size() + 1];
      std::copy(str.c_str(), str.c_str() + str.size() + 1, m_ptr);
    }
    StringValue(const StringValue& other) {
      auto len = std::strlen(other.m_ptr);
      m_ptr = new char[len + 1];
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
      return m_ptr;
    }

  private:
    char* m_ptr; // TODO use char8_t
  };

  namespace interface {
    /// String
    using String = BoxedHeapObject<StringValue>;
  } // namespace interface

} // namespace TORI_NS::detail

namespace TORI_NS {
  TORI_DECL_TYPE(String)
} // namespace TORI_NS
