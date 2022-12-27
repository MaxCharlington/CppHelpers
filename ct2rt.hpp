#ifndef CT2RT_HPP
#define CT2RT_HPP

#include <algorithm>
#include <array>
#include <string>
#include <type_traits>
#include <tuple>

/* Example:
struct S
{
    int a;
    std::string b;

    constexpr S(int a, const std::string& b) : a{a}, b{b} {}


    // Special methods to support compiletime to runtime transfer
    template<std::array<std::size_t, 1> Sizes>
    constexpr auto serialize() const
    {
        std::array<char, Sizes[0] + 1> b_repr;
        std::copy_n(b.c_str(), Sizes[0] + 1, b_repr.data());
        return std::make_tuple(a, b_repr);
    }

    constexpr auto sizes() const -> std::array<std::size_t, 1>
    {
        return { b.size() };
    }

    S(const auto& repr) : a{std::get<0>(repr)}, b{std::get<1>(repr).data()} {}
};

static constexpr auto create()
{
    S s{1, "Hello"};
    s.b += ", world!";
    // compiletime calculations...
    return [=]{
        return s;
    };
}

*/

namespace detail
{
    template<auto Val>
    constexpr bool UsableInTemplate = true;

    template<typename T>
    concept CTriviallyCopyConstructible = std::is_trivially_copy_constructible_v<T>;

    template<typename T>
    concept CCompound = std::is_compound_v<T>;

}  // namespace detail

template<std::invocable auto Getter>
static consteval auto to_runtime()
{
    static_assert(requires{ {Getter()} -> std::invocable; },
        "template parameter 'Getter' should be callable returning callable without parameters");
    static_assert(requires{ {Getter()()} -> detail::CCompound; },
        "template parameter 'Getter' should be callable returning callable returning object of compound type");
    static_assert(requires{ detail::UsableInTemplate<Getter()().sizes()>; },
        "object returned from 'sizes' method should be usable as template parameter");
    static_assert(requires{ {Getter()().template serialize<Getter()().sizes()>()} -> detail::CTriviallyCopyConstructible; },
        "'serialize' should return value capable to copy from compiletime to runtime");
    static_assert(std::is_constructible_v<decltype(Getter()()), decltype(Getter()().template serialize<Getter()().sizes()>())>,
        "'serialize' should return value that is capable to create object from");

    constexpr auto sizes = Getter()().sizes();
    return Getter()().template serialize<sizes>();
}


/* Usage:
int main() {
    S s{to_runtime<create>()};
}
*/

#endif  // CT2RT_HPP
