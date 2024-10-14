#pragma once

// c++23

#include <array>
#include <bit>          // for bit_cast
#include <cstddef>      // for byte, size_t
#include <cstdint>      // for uint8_t
#include <ranges>
#include <span>
#include <stdexcept>    // for runtime_error
#include <string>
#include <tuple>        // for apply
#include <type_traits>  // for decay_t, invoke_result_t
#include <utility>      // for forward, move, pair, declval
#include <vector>

using namespace std::literals;

template <typename T>
concept Trivial = std::is_trivial_v<T>;

template <typename T>
concept TupleLike = requires(T a) {
    std::tuple_size_v<T>;
    std::apply([](const auto&... args) {}, a);
};

template<typename T>
concept ContainerRange = std::ranges::range<T> and
    (requires(T obj){
        obj.push_back(std::declval<typename T::value_type>());
    } or requires(T obj){
        obj.emplace(std::declval<typename T::value_type>());
    });

template <std::invocable auto Callable, typename SizeType = std::size_t>
struct CompiletimeResult {
    template <Trivial T>
    static consteval SizeType countBytes(T) {
        return sizeof(T);
    }

    template <ContainerRange T>
    static consteval SizeType countBytes(const T& rng) {
        SizeType size = 0;
        for (const auto& el : rng) {
            size += countBytes(el);
        }
        return sizeof(SizeType) + size;
    }

    template <TupleLike T>
    static consteval SizeType countBytes(const T& tuple) {
        SizeType size = 0;
        std::apply(
            [&](const auto&... args) {
                const auto eachFn = [&](const auto& el) {
                    size += countBytes(el);
                };
                (eachFn(args), ...);
            },
            tuple);
        return size;
    }

    template <typename T>
    static consteval void push_front(std::vector<std::byte>& vec, T val) {
        auto bytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(val);
        for (std::byte byte : bytes | std::ranges::views::reverse) {
            vec.insert(vec.begin(), byte);
        }
    }

    template <Trivial T>
    static consteval std::vector<std::byte> toBuffer(const T& val) {
        const auto bytesArr =
            std::bit_cast<std::array<std::byte, sizeof(T)>>(val);
        return {bytesArr.begin(), bytesArr.end()};
    }

    template <ContainerRange T>
    static consteval std::vector<std::byte> toBuffer(const T& rng) {
        std::vector<std::byte> buf;
        SizeType size = 0;
        for (const auto& el : rng) {
            const auto elBuf = toBuffer(el);
            size += elBuf.size();
            for (std::byte elByte : elBuf) {
                buf.push_back(elByte);
            }
        }
        push_front(buf, size);
        return buf;
    }

    template <TupleLike T>
    static consteval std::vector<std::byte> toBuffer(const T& tuple) {
        std::vector<std::byte> buf;
        std::apply(
            [&](const auto&... args) {
                const auto eachFn = [&](const auto& el) {
                    const auto elBuf = toBuffer(el);
                    for (std::byte elByte : elBuf) {
                        buf.push_back(elByte);
                    }
                };
                (eachFn(args), ...);
            },
            tuple);
        return buf;
    }

    template <typename Container, typename Elem>
    static constexpr void append(Container& cont, Elem&& el) {
        if constexpr (requires { cont.push_back(std::forward<Elem>(el)); }) {
            cont.push_back(std::forward<Elem>(el));
        } else if constexpr (requires {
                                 cont.emplace(std::forward<Elem>(el));
                             }) {
            cont.emplace(std::forward<Elem>(el));
        } else {
            throw std::runtime_error{"No way to append " +
                                     std::string{typeid(Container).name()} +
                                     ' ' + typeid(Elem).name()};
        }
    }

    template <Trivial T>
    static constexpr std::pair<T, std::size_t> fromBuffer(
        std::span<const std::byte> buf) {
        std::array<std::byte, sizeof(T)> bytes;
        std::size_t i = 0;
        for (std::byte byte : buf | std::ranges::views::take(sizeof(T))) {
            bytes[i++] = byte;
        }
        return {std::bit_cast<T>(bytes), sizeof(T)};
    }

    template <ContainerRange T>
    static constexpr std::pair<T, std::size_t> fromBuffer(
        std::span<const std::byte> buf) {
        const auto size = fromBuffer<SizeType>(buf).first;

        T rng;
        const auto itemsBytesRange =
            buf | std::ranges::views::drop(sizeof(SizeType));
        auto remainingSize = size;
        while (remainingSize > 0) {
            auto [item, readSize] = fromBuffer<typename T::value_type>(
                itemsBytesRange |
                std::ranges::views::drop(size - remainingSize));
            remainingSize -= readSize;
            append(rng, std::move(item));
        }
        return {std::move(rng), size + sizeof(SizeType)};
    }

    template <TupleLike T>
    static constexpr std::pair<T, std::size_t> fromBuffer(
        std::span<const std::byte> buf) {
        T tuple{};
        const auto count = std::tuple_size_v<T>;
        std::size_t readBytes = 0;
        std::apply(
            [&](auto&... args) {
                const auto eachFn = [&](auto& el) {
                    using Type = std::decay_t<decltype(el)>;
                    auto [obj, read] = fromBuffer<Type>(
                        buf | std::ranges::views::drop(readBytes));
                    readBytes += read;
                    el = std::move(obj);
                };
                (eachFn(args), ...);
            },
            tuple);
        return {std::move(tuple), readBytes};
    }

    static consteval auto populateBuf() {
        const auto bytes = toBuffer(Callable());
        std::array<std::byte, Size> arr;
        for (std::size_t i = 0; i < Size; i++) {
            arr[i] = bytes[i];
        }
        return arr;
    }

    consteval CompiletimeResult() : buf{populateBuf()} {}

    using ReturnType = std::invoke_result_t<decltype(Callable)>;

    template<typename T, typename U = ReturnType>
    consteval static bool compatible()
    {
        return std::is_same_v<T, U> or
            []{
                if constexpr (requires{
                    std::declval<typename T::value_type>(); std::declval<typename U::value_type>();
                })
                {
                    return
                        std::is_same_v<typename T::value_type, typename U::value_type> and
                        std::is_trivial_v<typename T::value_type> and
                        (
                            (std::ranges::view<T> and std::ranges::range<U>) or
                            (std::ranges::range<T> and std::ranges::view<U>)
                        );
                }
                else
                {
                    return false;
                }
            }();
    }

    template<typename T>
    static constexpr bool Compatible = compatible<T>();

    template<typename T> requires std::is_same_v<T, ReturnType>
    constexpr operator T() const { return fromBuffer<ReturnType>(buf).first; }

    static constexpr std::size_t Size = countBytes(Callable());
    std::array<std::byte, Size> buf;
};

template <std::invocable auto Callable, typename SizeType>
static constexpr auto cross_container = CompiletimeResult<Callable, SizeType>{};

static_assert(
    static_cast<std::vector<std::string>>(
        cross_container<[] { return std::vector<std::string>{"b"s}; }, uint8_t>
    )[0] == "b");

static_assert(
    static_cast<std::array<std::string, 1>>(
        cross_container<[] { return std::array<std::string, 1>{"b"s}; }, uint8_t>
    )[0] == "b");

static_assert(
    std::get<0>(
        static_cast<std::tuple<std::string>>(
            cross_container<[] { return std::tuple<std::string>{"b"s}; }, uint8_t>
        )
    ) == "b");

static_assert(
    static_cast<std::pair<std::string, int>>(
        cross_container<[] { return std::pair<std::string, int>{"b"s, 5}; }, uint8_t>
    ).first == "b");

static_assert(
    cross_container<[] { return std::vector<std::string>{"b"s}; }, uint8_t>.buf.size() == 3);

static_assert(
    cross_container<[] { return std::array<std::string, 1>{"b"s}; }, uint8_t>.buf.size() == 2);

static_assert(
    cross_container<[] { return std::string{"b"s}; }, uint8_t>.buf.size() == 2);

static_assert(
    cross_container<[] { return std::string{"b"s}; }, uint16_t>.buf.size() == 3);

static_assert(
    cross_container<[] { return 1; }, uint16_t>.buf.size() == 4);

static_assert(
    cross_container<[] { return 1; }, uint64_t>.buf.size() == 4);


// TODO
static_assert(
    cross_container<[] { return 1; }, uint64_t>.template compatible<int, int>());

static_assert(
    cross_container<[] { return 1; }, uint64_t>.template compatible<
        std::vector<int>, std::span<int>>());
