#pragma once

#include <algorithm>
#include <string_view>

/**
 * Literal class type that wraps a constant expression string
 *
 * Uses implicit conversion to allow templates to accept constant strings
 */
template<std::size_t N>
struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) noexcept
    {
        std::copy_n(str, N, value);
    }

    constexpr explicit StringLiteral(std::string_view str) noexcept 
    {
        if (std::is_constant_evaluated())
        {
            if (str.size() + 1 != N) throw "Wrong length template param provided";
        }
        std::copy_n(str.data(), N, value);
    }

    constexpr const char* c_str() const noexcept
    {
        return value;
    }

    char value[N];
};
