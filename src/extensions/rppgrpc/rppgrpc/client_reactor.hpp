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

namespace rppgrpc::details
{
    // template<rpp::constraint::decayed_type Request, rpp::constraint::observer Observer>
    // class client_write_reactor final : public grpc::ClientWriteReactor<Request>
    // {
    //     using Response = rpp::utils::extract_observer_type_t<Observer>;
    //     using Base     = grpc::ClientWriteReactor<Request>;

    // public:
    //     template<rpp::constraint::observable_of_type<Request> Observable, rpp::constraint::decayed_same_as<Observer> TObserver>
    //     client_write_reactor(const Observable& messages, TObserver&& events, Response*& ptr_to_write_response)
    //         : m_observer{std::forward<TObserver>(events)}
    //         , m_disposable{messages.subscribe_with_disposable([this]<rpp::constraint::decayed_same_as<Request> T>(T&& message) {
    //             std::lock_guard lock{m_write_mutex};
    //             m_write.push_back(std::forward<T>(message));
    //             if (m_write.size() == 1)
    //                 Base::StartWrite(&m_write.front()); },
    //                                                           [this](const std::exception_ptr&) {
    //                                                               //   Base::StartWritesDone();
    //                                                               context_.TryCancel();
    //                                                           },
    //                                                           [this]() {
    //                                                               Base::StartWritesDone();
    //                                                           })}
    //     {
    //         ptr_to_write_response = &m_read;
    //     }

    //     void Init()
    //     {
    //         Base::StartCall();
    //     }

    // private:
    //     void OnWriteDone(bool ok) override
    //     {
    //         if (!ok)
    //         {
    //             m_observer.on_error(std::make_exception_ptr(rppgrpc::utils::reactor_failed{"OnWriteDone is not ok"}));
    //             context_.TryCancel();
    //             return;
    //         }

    //         std::lock_guard lock{m_write_mutex};
    //         m_write.pop_front();

    //         if (!m_write.empty())
    //         {
    //             Base::StartWrite(&m_write.front());
    //         }
    //     }

    //     void OnDone(const grpc::Status& s) override
    //     {
    //         if (s.ok())
    //         {
    //             m_observer.on_next(std::move(m_read));
    //             m_observer.on_completed();
    //         }
    //         else
    //         {
    //             m_observer.on_error(std::make_exception_ptr(rppgrpc::utils::reactor_failed{s.error_message()}));
    //         }
    //         Destroy();
    //     }

    // private:
    //     void Destroy()
    //     {
    //         m_disposable.dispose();
    //         delete this;
    //     }

    // private:
    //     Observer                m_observer;
    //     rpp::disposable_wrapper m_disposable;

    //     Response m_read{};

    //     std::mutex         m_write_mutex{};
    //     std::list<Request> m_write{};
    // };

    // template<rpp::constraint::observer Observer>
    // class client_read_reactor final : public grpc::ClientReadReactor<rpp::utils::extract_observer_type_t<Observer>>
    // {
    //     using Response = rpp::utils::extract_observer_type_t<Observer>;
    //     using Base     = grpc::ClientReadReactor<Response>;

    // public:
    //     template<rpp::constraint::decayed_same_as<Observer> TObserver>
    //         requires (!rpp::constraint::decayed_same_as<TObserver, client_read_reactor<Observer>>)
    //     explicit client_read_reactor(TObserver&& events)
    //         : m_observer{std::forward<TObserver>(events)}
    //     {
    //     }

    //     client_read_reactor(const client_read_reactor&) = delete;
    //     client_read_reactor(client_read_reactor&&)      = delete;

    //     void Init()
    //     {
    //         Base::StartCall();
    //         Base::StartRead(&m_read);
    //     }

    // private:
    //     void OnReadDone(bool ok) override
    //     {
    //         if (!ok)
    //         {
    //             m_observer.on_error(std::make_exception_ptr(rppgrpc::utils::reactor_failed{"OnReadDone is not ok"}));
    //             context_.TryCancel();
    //             return;
    //         }
    //         m_observer.on_next(m_read);
    //         Base::StartRead(&m_read);
    //     }

    //     void OnDone(const grpc::Status& s) override
    //     {
    //         if (s.ok())
    //         {
    //             m_observer.on_completed();
    //         }
    //         else
    //         {
    //             m_observer.on_error(std::make_exception_ptr(rppgrpc::utils::reactor_failed{s.error_message()}));
    //         }
    //         Destroy();
    //     }

    // private:
    //     void Destroy()
    //     {
    //         delete this;
    //     }

    // private:
    //     Observer m_observer;
    //     Response m_read{};
    // };
} // namespace rppgrpc::details
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
                    Base::StartWritesDone();
                },
                [this]() {
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
            {
                m_requests.get_disposable().dispose();
                m_observer.get_observer().on_error(std::make_exception_ptr(rppgrpc::utils::reactor_failed{"OnReadDone is not ok"}));
                return;
            }
            m_observer.get_observer().on_next(m_read);
            Base::StartRead(&m_read);
        }

        void OnWriteDone(bool ok) override
        {
            if (!ok)
            {
                m_requests.get_disposable().dispose();
                m_observer.get_observer().on_error(std::make_exception_ptr(rppgrpc::utils::reactor_failed{"OnWriteDone is not ok"}));
                return;
            }

            std::lock_guard lock{m_write_mutex};
            m_write.pop_front();

            if (!m_write.empty())
            {
                Base::StartWrite(&m_write.front());
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
    };
} // namespace rppgrpc
