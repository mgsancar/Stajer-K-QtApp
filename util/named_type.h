#pragma once

#include <utility>

template <typename T, typename param>
class NamedType
{
public:
     explicit NamedType ( T const& value ) : m_value ( value ) {}
     explicit NamedType ( T&& value = T{} ) : m_value ( std::move ( value ) ) {}
     T& get()
     {
          return m_value;
     }
     T const& get() const
     {
          return m_value;
     }

     operator T() const noexcept {
         return m_value;
     }

private:
     T m_value;
};
