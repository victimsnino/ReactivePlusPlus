#include <rpp/rpp.hpp>

#include <functional>
#include <iostream>

/**
 * @example readme.cpp
 */


// ![simple_custom_map]
template<typename Fn>
struct simple_map
{
    simple_map(const Fn& fn)
        : fn(fn)
    {
    }

    Fn fn{};

    // 1: define traits for the operator with upstream (previous type) type
    template<rpp::constraint::decayed_type T>
    struct operator_traits
    {
        // 1.1: it could have static asserts to be sure T is applicable for this operator
        static_assert(std::invocable<Fn, T>, "Fn is not invocable with T");

        // 1.2: it should have `result_type` is type of new observable after applying this operator
        using result_type = std::invoke_result_t<Fn, T>;
    };

    // 2: define updated optimal disposables strategy. Set to `rpp::details::observables::default_disposables_strategy` if you don't know what is that.
    template<rpp::details::observables::constraint::disposables_strategy Prev>
    using updated_optimal_disposables_strategy = Prev;


    // 3: implement core logic of operator: accept downstream observer (of result_type) and convert it to upstream observer (of T).
    template<typename Upstream, rpp::constraint::observer Observer>
    auto lift(Observer&& observer) const
    {
        const auto dynamic_observer = std::forward<Observer>(observer).as_dynamic();
        return rpp::make_lambda_observer<Upstream>([dynamic_observer, fn = fn](const auto& v) { dynamic_observer.on_next(fn(v)); },
                                                   [dynamic_observer](const std::exception_ptr& err) { dynamic_observer.on_error(err); },
                                                   [dynamic_observer]() { dynamic_observer.on_completed(); });
    }
};

template<typename Fn>
simple_map(const Fn& fn) -> simple_map<Fn>;

void test()
{
    rpp::source::just(1) | simple_map([](int v) { return std::to_string(v); }) | rpp::ops::subscribe();
}
// ![simple_custom_map]

int main() // NOLINT(bugprone-exception-escape)
{
    // ![readme]
    rpp::source::from_callable(&::getchar)
        | rpp::operators::repeat()
        | rpp::operators::take_while([](char v) { return v != '0'; })
        | rpp::operators::filter(std::not_fn(&::isdigit))
        | rpp::operators::map(&::toupper)
        | rpp::operators::subscribe([](char v) { std::cout << v; });
    // ![readme]

    rpp::source::just(1) | simple_map([](int v) { return std::to_string(v); }) | rpp::ops::subscribe();

    return 0;
}
