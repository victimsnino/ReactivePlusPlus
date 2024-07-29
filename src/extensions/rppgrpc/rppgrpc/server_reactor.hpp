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

#include <grpcpp/support/server_callback.h>
#include <rppgrpc/fwd.hpp>
#include <rppgrpc/utils/exceptions.hpp>

#include <deque>

namespace rppgrpc::details
{
    template<rpp::constraint::decayed_type Response>
    struct server_write_data
    {
        std::mutex           write_mutex{};
        std::deque<Response> write{};
        bool                 finished{};
    };

    template<rpp::constraint::decayed_type Response, rpp::constraint::decayed_type TOwner>
    struct server_write_observer_strategy
    {
        template<rpp::constraint::decayed_same_as<Response> T>
        void on_next(T&& message) const
        {
            std::lock_guard lock{owner.get().m_write_data.write_mutex};
            owner.get().m_write_data.write.push_back(std::forward<T>(message));
            if (owner.get().m_write_data.write.size() == 1)
                owner.get().StartWrite(&owner.get().m_write_data.write.front());
        }

        void on_error(const std::exception_ptr&) const
        {
            std::lock_guard lock{owner.get().m_write_data.write_mutex};
            owner.get().m_write_data.finished = true;

            if (owner.get().m_write_data.write.size() == 0)
                owner.get().Finish(grpc::Status{grpc::StatusCode::INTERNAL, "Internal error happens"});
        }
        void on_completed() const
        {
            std::lock_guard lock{owner.get().m_write_data.write_mutex};
            owner.get().m_write_data.finished = true;

            if (owner.get().m_write_data.write.size() == 0)
                owner.get().Finish(grpc::Status::OK);
        }

        static constexpr bool is_disposed() { return false; }
        static constexpr void set_upstream(const rpp::disposable_wrapper&) {}

        std::reference_wrapper<TOwner> owner{};
    };
} // namespace rppgrpc::details

namespace rppgrpc
{
    template<rpp::constraint::decayed_type Request, rpp::constraint::decayed_type Response>
    class server_bidi_reactor final : public grpc::ServerBidiReactor<Request, Response>
    {
        using Base = grpc::ServerBidiReactor<Request, Response>;

    public:
        friend struct details::server_write_observer_strategy<Response, server_bidi_reactor>;

        server_bidi_reactor()
        {
            m_responses.get_observable().subscribe(details::server_write_observer_strategy<Response, server_bidi_reactor>{*this});

            Base::StartSendInitialMetadata();
            Base::StartRead(&m_read);
        }

        auto get_observer()
        {
            return m_responses.get_observer();
        }

        auto get_observable()
        {
            return m_observer.get_observable();
        }

    private:
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
            {
                m_observer.get_observer().on_error(std::make_exception_ptr(rppgrpc::utils::reactor_failed{"OnWriteDone is not ok"}));
                Base::Finish(grpc::Status::CANCELLED);
                return;
            }

            std::lock_guard lock{m_write_data.write_mutex};
            m_write_data.write.pop_front();

            if (!m_write_data.write.empty())
            {
                Base::StartWrite(&m_write_data.write.front());
            }
            else if (m_write_data.finished)
            {
                Base::Finish(grpc::Status::OK);
            }
        }

        void OnDone() override
        {
            m_responses.get_disposable().dispose();
            m_observer.get_observer().on_completed();
            Destroy();
        }

        void OnCancel() override
        {
            m_responses.get_disposable().dispose();
            m_observer.get_observer().on_error(std::make_exception_ptr(rppgrpc::utils::reactor_failed{"OnCancel called"}));
            Base::Finish(grpc::Status::CANCELLED);
        }

    private:
        void Destroy()
        {
            delete this;
        }

    private:
        rpp::subjects::serialized_publish_subject<Response> m_responses{};

        rpp::subjects::publish_subject<Request> m_observer;
        Request                                 m_read{};

        details::server_write_data<Response> m_write_data{};
    };
} // namespace rppgrpc
