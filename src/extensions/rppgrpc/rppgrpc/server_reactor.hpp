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

#include <grpcpp/support/server_callback.h>
#include <rppgrpc/details/base.hpp>
#include <rppgrpc/fwd.hpp>
#include <rppgrpc/utils/exceptions.hpp>

#include <iostream>
#define DEBUG(X) std::cout << X << std::endl;

namespace rppgrpc
{
    /**
     * @brief RPP's based implementation for grpc server bidirectional reactor.
     * @details To use it you need:
     * - create it via `new` operator
     * - return it from bidirection method of CallbackService interface
     * - to access values FROM stream you can subscribe to observable obtained via `reactor->get_observable()` (same observable WOULD emit on_completed in case of successful stream termination and on_error in case of some errors with grpc stream)
     * - to pass values TO stream you can emit values to observer obtained via `reactor->get_observer()`
     *
     * @warning grpc server reactor have to finish manually, so it is expected that you call `on_completed()` on reactor->get_observer()
     *
     * @snippet server_reactor.cpp bidi_reactor
     *
     */
    template<rpp::constraint::decayed_type Request, rpp::constraint::decayed_type Response>
    class server_bidi_reactor final : public grpc::ServerBidiReactor<Request, Response>
        , private details::base_writer<Response>
        , private details::base_reader<Request>
    {
        using Base = grpc::ServerBidiReactor<Request, Response>;

    public:
        server_bidi_reactor()
        {
            Base::StartSendInitialMetadata();
            details::base_reader<Request>::handle_read_done(true);
        }

        ~server_bidi_reactor() override
        {
            DEBUG("server_bidi_reactor::~()");
        }

        using details::base_writer<Response>::get_observer;
        using details::base_reader<Request>::get_observable;

    private:
        void start_write(const Response& v) override
        {
            DEBUG("server_bidi_reactor::start_write()");
            Base::StartWrite(&v);
        }

        void start_read(Request& data) override
        {
            DEBUG("server_bidi_reactor::start_read()");
            Base::StartRead(&data);
        }

        void finish_writes(const grpc::Status& status) override
        {
            DEBUG("server_bidi_reactor::finish_writes()");
            Base::Finish(status);
        }

        void OnReadDone(bool ok) override
        {
            DEBUG("server_bidi_reactor::OnReadDone()");
            if (!ok)
                return;

            details::base_reader<Request>::handle_read_done();
        }

        void OnWriteDone(bool ok) override
        {
            DEBUG("server_bidi_reactor::OnWriteDone()");
            if (!ok)
                return;

            details::base_writer<Response>::handle_write_done();
        }

        void OnDone() override
        {
            DEBUG("server_bidi_reactor::OnDone()");

            details::base_writer<Response>::handle_on_done();
            details::base_reader<Request>::handle_on_done({});

            delete this;
        }

        void OnCancel() override
        {
            DEBUG("server_bidi_reactor::OnCancel()");

            details::base_writer<Response>::handle_on_done();
            details::base_reader<Request>::handle_on_done(std::make_exception_ptr(rppgrpc::utils::reactor_failed{"OnCancel called"}));

            Base::Finish(grpc::Status::CANCELLED);
        }
    };

    /**
     * @brief RPP's based implementation for grpc server write reactor.
     * @details To use it you need:
     * - create it via `new` operator
     * - return it from write-based method of CallbackService interface
     * - reactor provides `reactor->get_observable()` method but such as observable emits nothing and can be used only to be notified about completion/error
     * - to pass values TO stream you can emit values to observer obtained via `reactor->get_observer()`
     *
     * @snippet server_reactor.cpp write_reactor
     *
     */
    template<rpp::constraint::decayed_type Response>
    class server_write_reactor final : public grpc::ServerWriteReactor<Response>
        , private details::base_writer<Response>
        , private details::base_reader<rpp::utils::none>
    {
        using Base = grpc::ServerWriteReactor<Response>;

    public:
        server_write_reactor()
        {
            Base::StartSendInitialMetadata();
        }
        ~server_write_reactor() override
        {
            DEBUG("server_write_reactor::~()");
        }

        using details::base_writer<Response>::get_observer;
        using details::base_reader<rpp::utils::none>::get_observable;

    private:
        void start_write(const Response& v) override
        {
            DEBUG("server_write_reactor::start_write()");
            Base::StartWrite(&v);
        }

        void start_read(rpp::utils::none& data) override {}

        void finish_writes(const grpc::Status& status) override
        {
            DEBUG("server_write_reactor::finish_writes()");
            Base::Finish(status);
        }

        void OnWriteDone(bool ok) override
        {
            DEBUG("server_write_reactor::OnWriteDone()");
            if (!ok)
            {
                Base::Finish(grpc::Status::OK);
                return;
            }

            details::base_writer<Response>::handle_write_done();
        }

        void OnDone() override
        {
            DEBUG("server_write_reactor::OnDone()");
            details::base_writer<Response>::handle_on_done();
            details::base_reader<rpp::utils::none>::handle_on_done({});

            delete this;
        }

        void OnCancel() override
        {
            DEBUG("server_write_reactor::OnCancel()");

            details::base_writer<Response>::handle_on_done();
            details::base_reader<rpp::utils::none>::handle_on_done(std::make_exception_ptr(rppgrpc::utils::reactor_failed{"OnCancel called"}));

            Base::Finish(grpc::Status::CANCELLED);
        }
    };

    /**
     * @brief RPP's based implementation for grpc server read reactor.
     * @details To use it you need:
     * - create it via `new` operator
     * - return it from read-based method of CallbackService interface
     * - to access values FROM stream you can subscribe to observable obtained via `reactor->get_observable()` (same observable WOULD emit on_completed in case of successful stream termination and on_error in case of some errors with grpc stream)
     *
     * @snippet server_reactor.cpp read_reactor
     *
     */
    template<rpp::constraint::decayed_type Request>
    class server_read_reactor final : public grpc::ServerReadReactor<Request>
        , private details::base_reader<Request>
    {
        using Base = grpc::ServerReadReactor<Request>;

    public:
        server_read_reactor()
        {
            Base::StartSendInitialMetadata();
            details::base_reader<Request>::handle_read_done(true);
        }

        ~server_read_reactor() override
        {
            DEBUG("server_read_reactor::~()");
        }

        using details::base_reader<Request>::get_observable;

    private:
        void start_read(Request& data) override
        {
            DEBUG("server_read_reactor::start_read()");
            Base::StartRead(&data);
        }

        void OnReadDone(bool ok) override
        {
            DEBUG("server_read_reactor::OnReadDone()");
            if (!ok)
            {
                Base::Finish(grpc::Status::OK);
                return;
            }

            details::base_reader<Request>::handle_read_done();
        }

        void OnDone() override
        {
            DEBUG("server_read_reactor::OnDone()");
            details::base_reader<Request>::handle_on_done({});

            delete this;
        }

        void OnCancel() override
        {
            DEBUG("server_read_reactor::OnCancel()");
            details::base_reader<Request>::handle_on_done(std::make_exception_ptr(rppgrpc::utils::reactor_failed{"OnCancel called"}));
        }
    };
} // namespace rppgrpc
