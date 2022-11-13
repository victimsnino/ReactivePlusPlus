//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "copy_count_tracker.hpp"
#include "mock_observer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <rppqt/sources/from_signal.hpp>

#include <QObject>

struct TestQObject : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    void EmitSingleValueSignal(int v)
    {
        emit SingleValueSignal(v);
    }

    void EmitMultipleValueSignal(int v, double d, const std::string& s)
    {
        emit MultipleValueSignal(v, d, s);
    }

    void EmitNoValueSignal()
    {
        emit NoValueSignal();
    }

Q_SIGNALS:
    void SingleValueSignal(int v);
    void MultipleValueSignal(int v, double d, const std::string& s);
    void NoValueSignal();
};

SCENARIO("from_signal can see object value from object signal", "[source][from][rppqt]")
{
    GIVEN("qobject with signal with 1 argument and observable from signal from this object")
    {
        mock_observer<int> mock_observer{};
        auto               testobject = std::make_unique<TestQObject>();
        auto               obs        = rppqt::source::from_signal(*testobject, &TestQObject::SingleValueSignal);
        WHEN("emit signal, subscribe on it and emit signal")
        {
            testobject->EmitSingleValueSignal(1);
            obs.subscribe(mock_observer);
            testobject->EmitSingleValueSignal(2);
            THEN("subscriber sees only emission after subscribe")
            {
                CHECK(mock_observer.get_received_values() == std::vector<int>{2});
                CHECK(mock_observer.get_on_error_count() == 0);
                CHECK(mock_observer.get_on_completed_count() == 0);
            }
            AND_WHEN("object destroyed")
            {
                testobject.reset();
                THEN("subscriber sees completion")
                {
                    CHECK(mock_observer.get_received_values() == std::vector<int>{2});
                    CHECK(mock_observer.get_on_error_count() == 0);
                    CHECK(mock_observer.get_on_completed_count() == 1);
                }
            }
        }
        WHEN("object destroyed before subscription")
        {
            testobject.reset();
            AND_WHEN("then subscriber subscribed")
            {
                obs.subscribe(mock_observer);

                THEN("subscriber sees only completion")
                {
                    CHECK(mock_observer.get_received_values() == std::vector<int>{});
                    CHECK(mock_observer.get_on_error_count() == 0);
                    CHECK(mock_observer.get_on_completed_count() == 1);
                }
            }
        }
    }
}

SCENARIO("from_signal sends tuple if multiple values", "[source][from][rppqt]")
{
    GIVEN("object with signal with multiple values and observable from this signal")
    {
         mock_observer<std::tuple<int, double, std::string>> mock_observer{};
        auto               testobject = std::make_unique<TestQObject>();
        auto               obs        = rppqt::source::from_signal(*testobject, &TestQObject::MultipleValueSignal);
        WHEN("subscribe on it and emit signal")
        {
            obs.subscribe(mock_observer);
            testobject->EmitMultipleValueSignal(1, 2, "31");
            THEN("subscriber sees values")
            {
                CHECK(mock_observer.get_received_values() == std::vector<std::tuple<int, double, std::string>>{std::tuple{1, 2.0, "31"}});
                CHECK(mock_observer.get_on_error_count() == 0);
                CHECK(mock_observer.get_on_completed_count() == 0);
            }
        }
    }
}

SCENARIO("from_signal sends special struct if no args in signal", "[source][from][rppqt]")
{
    GIVEN("object with signal with zero values and observable from this signal")
    {
         mock_observer<rpp::utils::none> mock_observer{};
        auto               testobject = std::make_unique<TestQObject>();
        auto               obs        = rppqt::source::from_signal(*testobject, &TestQObject::NoValueSignal);
        WHEN("subscribe on it and emit signal")
        {
            obs.subscribe(mock_observer);
            testobject->EmitNoValueSignal();
            THEN("subscriber sees values")
            {
                CHECK(mock_observer.get_received_values().size() == 1);
                CHECK(mock_observer.get_on_error_count() == 0);
                CHECK(mock_observer.get_on_completed_count() == 0);
            }
        }
    }
}


#include "test_from_signal.moc"
