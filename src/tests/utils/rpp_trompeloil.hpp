#pragma once

#include <doctest/doctest.h>
#include <doctest/trompeloeil.hpp>

#include <rpp/disposables/fwd.hpp>

#include <rpp/observers/observer.hpp>

#include <trompeloeil.hpp>

#include <exception>
#include <thread>

template<typename T>
class mock_observer
{
public:
    struct impl_t
    {
        impl_t() = default;

        MAKE_MOCK1(on_next_lvalue, void(const T&), const);
        MAKE_MOCK1(on_next_rvalue, void(T&&), const);
        MAKE_MOCK1(on_error, void(const std::exception_ptr& err), const);
        MAKE_MOCK0(on_completed, void(), const);
    };

    impl_t& operator*() const noexcept { return *m_impl; }

    void on_next(const T& v) const noexcept
    {
        m_impl->on_next_lvalue(v);
    }

    void on_next(T&& v) const noexcept
    {
        m_impl->on_next_rvalue(std::move(v));
    }

    void on_error(const std::exception_ptr& err) const noexcept { m_impl->on_error(err); }
    void on_completed() const noexcept { m_impl->on_completed(); }

    static bool is_disposed() noexcept { return false; }
    static void set_upstream(const rpp::disposable_wrapper&) noexcept {}

    auto get_observer() const { return rpp::observer<T, mock_observer<T>>{*this}; }
    auto get_observer(rpp::composite_disposable_wrapper d) const { return rpp::observer_with_disposable<T, mock_observer<T>>{std::move(d), *this}; }

private:
    std::shared_ptr<impl_t> m_impl = std::make_shared<impl_t>();
};

template<typename T>
inline void wait(const std::unique_ptr<T>& e)
{
    while (!e->is_satisfied())
    {
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }
}
