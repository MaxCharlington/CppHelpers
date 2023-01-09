#ifndef CT2RT_HPP
#define CT2RT_HPP

#include <algorithm>
#include <array>
#include <string>
#include <type_traits>
#include <tuple>

/* Usage:
int main() {
    S s{to_runtime<create>()};
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

template<std::invocable auto Get>
static consteval auto to_runtime()
{
    static_assert(requires{ {Get()} -> std::invocable; },
        "template parameter 'Get' should be callable returning callable without parameters");
    static_assert(requires{
            {Get()()} -> detail::CCompound;
        }, "template parameter 'Get' should be callable returning callable returning object of compound type");
    static_assert(requires{
            {Get()().sizes_count()} -> std::same_as<std::size_t>;
        }, "'sizes_count' method should return std::size_t representing count of sizes");
    static_assert(requires{
            detail::UsableInTemplate<
                Get()().template sizes<
                    Get()().sizes_count()
                >()
            >; }, "object returned from 'sizes' method should be usable as template parameter");
    static_assert(requires{
            {Get()().template serialize<
                Get()().template sizes<
                    Get()().sizes_count()
                >()
            >()} -> detail::CTriviallyCopyConstructible;
        }, "'serialize' should return value capable to copy from compiletime to runtime");
    static_assert(std::is_constructible_v<
            decltype(Get()()),
            decltype(Get()().template serialize<
                Get()().template sizes<
                    Get()().sizes_count()
                >()
            >()
        )>, "'serialize' should return value that is capable to create object from");

    constexpr std::size_t sizes_count = Get()().sizes_count();
    constexpr auto sizes = Get()().template sizes<sizes_count>();
    return Get()().template serialize<sizes>();
}

#endif  // CT2RT_HPP
