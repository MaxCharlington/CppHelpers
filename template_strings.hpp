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

    constexpr bool operator==(std::string_view str) const
    {
        return str == value;
    }

    constexpr bool operator==(const StringLiteral& str) const
    {
        return str == std::string_view{value};
    }

    constexpr const char* c_str() const noexcept
    {
        return value;
    }

    constexpr auto size() const -> std::size_t
    {
        return N;
    }

    constexpr auto length() const -> std::size_t
    {
        return N;
    }

    char value[N];
};


template<size_t Hash>
struct HashedStrImpl {
    constexpr static size_t m_hash{Hash};

    template <size_t H>
    constexpr bool operator==(const HashedStrImpl<H>& str) const
    {
        return m_hash == str.hash();
    }

    constexpr size_t hash() const noexcept
    {
        return m_hash;
    }
};

constexpr size_t hash(std::string_view str) noexcept
{   
    return str.length(); // replace real hash impl
}

template<StringLiteral s>
using HashedStr = HashedStrImpl<hash(s.value)>;

template<StringLiteral Str, size_t Hash>
struct StringImpl
{
    constexpr auto get() const -> std::string_view
    {
        return Str.value;
    }

    constexpr auto c_str() const -> const char*
    {
        return Str.value;
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
};

template<StringLiteral S>
using String = StringImpl<S, hash(S.value)>;
