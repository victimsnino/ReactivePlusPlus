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

namespace rppgrpc
{
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

        using details::base_writer<Response>::get_observer;
        using details::base_reader<Request>::get_observable;

    private:
        void start_write(const Response& v) override
        {
            Base::StartWrite(&v);
        }

        void start_read(Request& data) override
        {
            Base::StartRead(&data);
        }

        void finish_writes(const grpc::Status& status) override
        {
            Base::Finish(status);
        }

        void OnReadDone(bool ok) override
        {
            if (!ok)
                return;

            details::base_reader<Request>::handle_read_done();
        }

        void OnWriteDone(bool ok) override
        {
            if (!ok)
                return;

            details::base_writer<Response>::handle_write_done();
        }

        void OnDone() override
        {
            details::base_writer<Response>::handle_on_done();
            details::base_reader<Request>::handle_on_done({});

            delete this;
        }

        void OnCancel() override
        {
            details::base_writer<Response>::handle_on_done();
            details::base_reader<Request>::handle_on_done(std::make_exception_ptr(rppgrpc::utils::reactor_failed{"OnCancel called"}));

            Base::Finish(grpc::Status::CANCELLED);
        }
    };
} // namespace rppgrpc
