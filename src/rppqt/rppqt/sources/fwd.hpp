//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/utils/operator_declaration.hpp>

class QObject;

namespace rpp::details
{
struct from_event_qt_tag;
} // namespace rpp::details

namespace rppqt::observable
{
template<std::derived_from<QObject> TObject, typename R,typename ...Args>
auto from_event(const TObject& object, R (TObject::*signal)(Args...)) requires rpp::details::is_header_included<rpp::details::from_event_qt_tag, TObject, R, Args...>;
} // namespace rppqt::observable

namespace rppqt
{
namespace source = observable;
} // namespace rppqt