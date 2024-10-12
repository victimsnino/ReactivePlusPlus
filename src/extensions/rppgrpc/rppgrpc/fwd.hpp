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

#include <rpp/utils/constraints.hpp>

/**
 * @defgroup rppgrpc rppgrpc
 * @brief RppGrpc is extension of RPP which enables support of grpc library.
 */

/**
 * @defgroup rppgrpc_reactors gRPC reactors
 * @brief Reactors for gRPC to connect it to RPP properly
 * @ingroup rppgrpc
 */

namespace rppgrpc
{
    template<rpp::constraint::decayed_type Request, rpp::constraint::decayed_type Response>
    class client_bidi_reactor;

    template<rpp::constraint::decayed_type Request>
    class client_write_reactor;

    template<rpp::constraint::decayed_type Response>
    class client_read_reactor;

    template<rpp::constraint::decayed_type Request, rpp::constraint::decayed_type Response>
    class server_bidi_reactor;

    template<rpp::constraint::decayed_type Request>
    class server_write_reactor;

    template<rpp::constraint::decayed_type Response>
    class server_read_reactor;
} // namespace rppgrpc
