#ifndef SEQUENCES_H
#define SEQUENCES_H
#include <cstddef>
#include <tuple>
struct Sequences{

    // Utility struct
    template<int I, typename ... Ts>
    struct VariadicTypeAt { using Type = typename std::tuple_element<I, std::tuple<Ts...>>::type; };

    // Utility struct
    template<typename ... Ts>
    struct VariadicTypeLast {
        using Type = typename VariadicTypeAt<sizeof ... (Ts) - 1, Ts...>::Type;
    };

    // Recursive case for removing the first S arguments and returning the rest
    template <std::size_t S, typename T, typename... Args>
    static auto pop_front(T&& first, Args&&... rest) {
        if constexpr (S > 1) {
            return pop_front_impl<S - 1>(std::forward<Args>(rest)...);
        } else {
            return std::make_tuple(std::forward<Args>(rest)...);
        }
    }
    // Recursive case for removing the last S arguments and returning the rest
    template <std::size_t S, typename T, typename... Args,typename std::enable_if<(S <= sizeof...(Args)), int>::type = 0>
    static auto pop_back(T&& first, Args&&... rest) {
        if constexpr (sizeof...(Args) == S) {
            return std::make_tuple(std::forward<T>(first));
        } else {
            return std::tuple_cat(std::make_tuple(std::forward<T>(first)), pop_back<S>(std::forward<Args>(rest)...));
        }
    }
private:
    // Base case for recursion
    template <std::size_t S, typename T>
    static auto pop_front_impl(T&& arg) {
        return std::make_tuple();
    }
    // Base case for recursion
    template <std::size_t S, typename T>
    static auto pop_back_impl(T&& arg) {
        return std::make_tuple(std::forward<T>(arg));
    }
};

#endif // SEQUENCES_H
