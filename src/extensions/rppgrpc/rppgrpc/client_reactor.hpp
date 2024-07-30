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
#include <rppgrpc/details/base.hpp>
#include <rppgrpc/fwd.hpp>
#include <rppgrpc/utils/exceptions.hpp>

#include <deque>

namespace rppgrpc
{
    /**
     * @brief RPP's based implementation for grpc client bidirectional reactor.
     * @details To use it you need:
     * - create it via `new` operator OR be sure it is alive while it is used inside grpc.
     * - pass it to `stub->async()->GrpcBidirectionalStream(ctx, reactor);`
     * - call `reactor->init()` method for actual starting of grpc logic
     * - to access values FROM stream you can subscribe to observable obtained via `reactor->get_observable()` (same observable WOULD emit on_completed in case of successful stream termination and on_error in case of some errors with grpc stream)
     * - to pass values TO stream you can emit values to observer obtained via `reactor->get_observer()`
     */
    template<rpp::constraint::decayed_type Request, rpp::constraint::decayed_type Response>
    class client_bidi_reactor final : public grpc::ClientBidiReactor<Request, Response>
        , private details::base_writer<Request>
    {
        using Base = grpc::ClientBidiReactor<Request, Response>;

    public:
        client_bidi_reactor() = default;

        void init()
        {
            Base::StartCall();
            Base::StartRead(&m_read);
        }

        using details::base_writer<Request>::get_observer;

        auto get_observable()
        {
            return m_observer.get_observable();
        }

    private:
        void start_write(const Request& v) override
        {
            Base::StartWrite(&v);
        }

        void finish_writes() override
        {
            Base::StartWritesDone();
        }

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

            details::base_writer<Request>::handle_write_done();
        }

        void OnDone(const grpc::Status& s) override
        {
            details::base_writer<Request>::handle_on_done();

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

    /**
     * @brief RPP's based implementation for grpc client write reactor
     * @details To use it you need:
     * - create it via `new` operator OR be sure it is alive while it is used inside grpc.
     * - pass it to `stub->async()->GrpcWriteStream(ctx, &request, reactor);`
     * - call `reactor->init()` method for actual starting of grpc logic
     * - to pass values TO stream you can emit values to observer obtained via `reactor->get_observer()`
     * - reactor provides `reactor->get_observable()` method but such as observable emits nothing and can be used only to be notified about completion/error
     */
    template<rpp::constraint::decayed_type Request>
    class client_write_reactor final : public grpc::ClientWriteReactor<Request>
        , private details::base_writer<Request>
    {
        using Base = grpc::ClientWriteReactor<Request>;

    public:
        client_write_reactor() = default;

        void init()
        {
            Base::StartCall();
        }

        using details::base_writer<Request>::get_observer;

        auto get_observable()
        {
            return m_observer.get_observable();
        }

    private:
        void start_write(const Request& v) override
        {
            Base::StartWrite(&v);
        }

        void finish_writes() override
        {
            Base::StartWritesDone();
        }

        using Base::StartCall;

        void OnWriteDone(bool ok) override
        {
            if (!ok)
                return;

            details::base_writer<Request>::handle_write_done();
        }

        void OnDone(const grpc::Status& s) override
        {
            details::base_writer<Request>::handle_on_done();

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
        rpp::subjects::publish_subject<rpp::utils::none> m_observer;
    };

    /**
     * @brief RPP's based implementation for grpc client read reactor.
     * @details To use it you need:
     * - create it via `new` operator OR be sure it is alive while it is used inside grpc.
     * - pass it to `stub->async()->GrpcReadStream(ctx, &response, reactor);`
     * - call `reactor->init()` method for actual starting of grpc logic
     * - to access values FROM stream you can subscribe to observable obtained via `reactor->get_observable()` (same observable WOULD emit on_completed in case of successful stream termination and on_error in case of some errors with grpc stream)
     */
    template<rpp::constraint::decayed_type Response>
    class client_read_reactor final : public grpc::ClientReadReactor<Response>
    {
        using Base = grpc::ClientReadReactor<Response>;

    public:
        client_read_reactor() = default;

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
