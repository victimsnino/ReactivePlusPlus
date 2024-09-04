#pragma once

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <rpp/disposables/fwd.hpp>

#include <trompeloeil.hpp>

#include <exception>
#include <thread>

namespace trompeloeil
{
    template<>
    inline void reporter<specialized>::send(
        severity s,
        const char*,
        unsigned long,
        const char* msg)
    {
        FAIL_CHECK(msg);
        if (s == severity::fatal)
        {
            std::terminate(); // terminate due to rpp could catch exceptions but we dont want it
        }
    }

    template<>
    inline void reporter<specialized>::sendOk(
        const char* trompeloeil_mock_calls_done_correctly)
    {
        REQUIRE(trompeloeil_mock_calls_done_correctly);
    }
} // namespace trompeloeil

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

private:
    std::shared_ptr<impl_t> m_impl = std::make_shared<impl_t>();
};

inline void wait(const std::unique_ptr<trompeloeil::expectation>& e)
{
    while (!e->is_satisfied())
    {
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }
}
