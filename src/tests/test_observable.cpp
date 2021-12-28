// MIT License
// 
// Copyright (c) 2021 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <catch2/catch_test_macros.hpp>

#include <rpp/observable.h>
#include <rpp/observer.h>
#include <rpp/subscriber.h>

SCENARIO("Observable should be subscribable")
{
    GIVEN("observer and observable of same type")
    {
        size_t on_next_called_count = 0;
        auto observer = rpp::observer{[&](int val) { ++on_next_called_count; }};

        size_t on_subscribe_called_count = 0;
        auto observable = rpp::observable{[&](const rpp::subscriber<int>& sub)
        {
            ++on_subscribe_called_count;
            sub.on_next(123);
        }};

        WHEN("subscribe called for observble")
        {
            observable.subscribe(observer);

            THEN("OnSubscribe lambda called once")
            {
                CHECK(on_subscribe_called_count == 1);
            }
            AND_THEN("on_next lambda called once")
            {
                CHECK(on_next_called_count == 1);
            }
        }
    }
}

//struct Base
//{
//    virtual     ~Base() {  }
//    virtual int operator()(int) const = 0;
//};
//
//template<typename F>
//struct Derrived final : Base
//{
//    Derrived(const F& f)
//        : m_f{f} {}
//
//    int operator()(int v) const final { return m_f(v); }
//    F   m_f;
//};
//
//TEST_CASE("Benchmark different approaches")
//{
//    std::array<int, 10> vec{};
//    auto action = [vec](int val){return val+2;};
//
//    BENCHMARK("call lambda", i)
//    {
//        return action(i);
//    };
//
//    std::function as_function = action;
//    BENCHMARK("call lambda as function", i)
//    {
//        return as_function(i);
//    };
//
//    Derrived der{action};
//    BENCHMARK("call lambda from derrived", i)
//    {
//        return der.m_f(i);
//    };
//
//    BENCHMARK("call lambda from derrived via operator()", i)
//    {
//        return der(i);
//    };
//
//    Base* b = [&action](){return static_cast<Base*>(new Derrived<decltype(action)>(action));}();
//    BENCHMARK("call lambda from base", i)
//    {
//        return (*b)(i);
//    };
//
//    BENCHMARK("call lambda from base via cast", i)
//    {
//        return (*static_cast<decltype(der)*>(b)).m_f(i);
//    };
//
//    BENCHMARK("call lambda from base via cast via operator()", i)
//    {
//        return (*static_cast<decltype(der)*>(b))(i);
//    };
//}
//
//TEST_CASE("Benchmark different with construction")
//{
//    std::array<int, 10> vec{};
//    auto action = [vec](int val){return val+2;};
//
//
//    BENCHMARK("create lambda as function", i)
//    {
//        return std::make_shared<std::function<int(int)>>(action);
//    };
//
//    BENCHMARK("create lambda as  derrived share_ptr", i)
//    {
//        return std::make_shared<Derrived<decltype(action)>>(action);
//    };
//
//    //BENCHMARK("call lambda from base via cast", i)
//    //{
//    //    std::shared_ptr<Base> b = std::make_shared<Derrived<decltype(action)>>(action);
//    //    return std::static_pointer_cast<Derrived<decltype(action)>>(b)->m_f(i);
//    //};
//
//    //BENCHMARK("call lambda from base via cast via operator()", i)
//    //{
//    //    std::shared_ptr<Base> b = std::make_shared<Derrived<decltype(action)>>(action);
//    //    return (*std::static_pointer_cast<Derrived<decltype(action)>>(b))(i);
//    //};
//}