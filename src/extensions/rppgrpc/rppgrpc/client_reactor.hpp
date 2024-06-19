//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/observables/fwd.hpp>

#include <rpp/subjects/publish_subject.hpp>

#include <grpcpp/support/client_callback.h>
#include <rppgrpc/fwd.hpp>
#include <rppgrpc/utils/exceptions.hpp>

#include <deque>

namespace rppgrpc
{
    template<rpp::constraint::decayed_type Request, rpp::constraint::decayed_type Response>
    class client_bidi_reactor final : public grpc::ClientBidiReactor<Request, Response>
    {
        using Base = grpc::ClientBidiReactor<Request, Response>;

    public:
        client_bidi_reactor()
        {
            m_requests.get_observable().subscribe(
                [this]<rpp::constraint::decayed_same_as<Request> T>(T&& message) {
                    std::lock_guard lock{m_write_mutex};
                    m_write.push_back(std::forward<T>(message));
                    if (m_write.size() == 1)
                        Base::StartWrite(&m_write.front());
                },
                [this](const std::exception_ptr&) {
                    std::lock_guard lock{m_write_mutex};
                    m_finished = true;

                    if (m_write.size() == 0)
                        Base::StartWritesDone();
                },
                [this]() {
                    std::lock_guard lock{m_write_mutex};
                    m_finished = true;

                    if (m_write.size() == 0)
                        Base::StartWritesDone();
                });
        }

        void init()
        {
            Base::StartCall();
            Base::StartRead(&m_read);
        }

        auto get_observer()
        {
            return m_requests.get_observer();
        }

        auto get_observable()
        {
            return m_observer.get_observable();
        }

    private:
        using Base::StartCall;
        using Base::StartRead;

        void OnReadDone(bool ok) override
        {
            if (!ok)
                return;

            m_observer.get_observer().on_next(m_read);
            Base::StartRead(&m_read);
        }

        void OnWriteDone(bool ok) override
        {
            if (!ok)
                return;

            std::lock_guard lock{m_write_mutex};
            m_write.pop_front();

            if (!m_write.empty())
            {
                Base::StartWrite(&m_write.front());
            }
            else if (m_finished)
            {
                Base::StartWritesDone();
            }
        }

        void OnDone(const grpc::Status& s) override
        {
            m_requests.get_disposable().dispose();

            if (s.ok())
            {
                m_observer.get_observer().on_completed();
            }
            else
            {
                m_observer.get_observer().on_error(std::make_exception_ptr(rppgrpc::utils::reactor_failed{s.error_message()}));
            }
            delete this;
        }

    private:
        rpp::subjects::serialized_publish_subject<Request> m_requests{};

        rpp::subjects::publish_subject<Response> m_observer;
        Response                                 m_read{};

        std::mutex          m_write_mutex{};
        std::deque<Request> m_write{};
        bool                m_finished{};
    };

    template<rpp::constraint::decayed_type Request>
    class client_write_reactor final : public grpc::ClientWriteReactor<Request>
    {
        using Base = grpc::ClientWriteReactor<Request>;

    public:
        client_write_reactor()
        {
            m_requests.get_observable().subscribe(
                [this]<rpp::constraint::decayed_same_as<Request> T>(T&& message) {
                    std::lock_guard lock{m_write_mutex};
                    m_write.push_back(std::forward<T>(message));
                    if (m_write.size() == 1)
                        Base::StartWrite(&m_write.front());
                },
                [this](const std::exception_ptr&) {
                    std::lock_guard lock{m_write_mutex};
                    m_finished = true;

                    if (m_write.size() == 0)
                        Base::StartWritesDone();
                },
                [this]() {
                    std::lock_guard lock{m_write_mutex};
                    m_finished = true;

                    if (m_write.size() == 0)
                        Base::StartWritesDone();
                });
        }

        void init()
        {
            Base::StartCall();
        }

        auto get_observer()
        {
            return m_requests.get_observer();
        }

        auto get_observable()
        {
            return m_observer.get_observable();
        }

    private:
        using Base::StartCall;

        void OnWriteDone(bool ok) override
        {
            if (!ok)
                return;

            std::lock_guard lock{m_write_mutex};
            m_write.pop_front();

            if (!m_write.empty())
            {
                Base::StartWrite(&m_write.front());
            }
            else if (m_finished)
            {
                Base::StartWritesDone();
            }
        }

        void OnDone(const grpc::Status& s) override
        {
            m_requests.get_disposable().dispose();

            if (s.ok())
            {
                m_observer.get_observer().on_completed();
            }
            else
            {
                m_observer.get_observer().on_error(std::make_exception_ptr(rppgrpc::utils::reactor_failed{s.error_message()}));
            }
            delete this;
        }

    private:
        rpp::subjects::serialized_publish_subject<Request> m_requests{};
        rpp::subjects::publish_subject<rpp::utils::none>   m_observer;

        std::mutex          m_write_mutex{};
        std::deque<Request> m_write{};
        bool                m_finished{};
    };

    template<rpp::constraint::decayed_type Response>
    class client_read_reactor final : public grpc::ClientReadReactor<Response>
    {
        using Base = grpc::ClientReadReactor<Response>;

    public:
        client_read_reactor()
        {
        }

        void init()
        {
            Base::StartCall();
            Base::StartRead(&m_read);
        }

        auto get_observable()
        {
            return m_observer.get_observable();
        }

    private:
        using Base::StartCall;
        using Base::StartRead;

        void OnReadDone(bool ok) override
        {
            if (!ok)
                return;

            m_observer.get_observer().on_next(m_read);
            Base::StartRead(&m_read);
        }

        void OnDone(const grpc::Status& s) override
        {
            if (s.ok())
            {
                m_observer.get_observer().on_completed();
            }
            else
            {
                m_observer.get_observer().on_error(std::make_exception_ptr(rppgrpc::utils::reactor_failed{s.error_message()}));
            }
            delete this;
        }

    private:
        rpp::subjects::publish_subject<Response> m_observer;
        Response                                 m_read{};
    };
} // namespace rppgrpc
