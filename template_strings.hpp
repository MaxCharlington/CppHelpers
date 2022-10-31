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

    constexpr bool operator==(std::string_view str) const noexcept
    {
        return str == value;
    }

    constexpr bool operator==(const StringLiteral& str) const noexcept
    {
        return str == std::string_view{value};
    }

    constexpr operator std::string_view() const noexcept
    {
        return value;
    }

    constexpr const char* c_str() const noexcept
    {
        return value;
    }

    constexpr auto size() const noexcept -> std::size_t
    {
        return N;
    }

    constexpr auto length() const noexcept -> std::size_t
    {
        return N;
    }

    char value[N];
};

constexpr size_t hash(std::string_view str) noexcept
{   
    return str.length(); // replace real hash impl
}

template<StringLiteral Str, size_t Hash>
struct StringImpl
{
    constexpr auto get() const -> std::string_view
    {
        return Str;
    }

    constexpr auto c_str() const -> const char*
    {
        return Str.c_str();
    }

    constexpr auto size() const -> std::size_t
    {
        return Str.size();
    }

    constexpr auto length() const -> std::size_t
    {
        return Str.length();
    }

    template<StringLiteral S, size_t H>
    constexpr bool operator==(const StringImpl<S, H>&) const
    {
        return Hash == H and Str == StringImpl<S, H>::str;
    }

    constexpr operator std::string_view() const noexcept
    {
        return Str;
    }
};

template<StringLiteral S>
using String = StringImpl<S, hash(S.value)>;
