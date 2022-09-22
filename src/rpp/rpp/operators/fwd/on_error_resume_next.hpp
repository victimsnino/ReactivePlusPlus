//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//                             TC Wang 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/constraints.hpp>
#include <rpp/observables/details/member_overload.hpp>
#include <rpp/utils/exceptions.hpp>
#include <rpp/utils/function_traits.hpp>
#include <rpp/utils/functors.hpp>

#include <exception>

namespace rpp::details
{
struct on_error_resume_next_tag;
}

namespace rpp::details
{

template<typename Fn>
concept resume_callable = std::invocable<Fn, std::exception_ptr> && constraint::observable<utils::decayed_invoke_result_t<Fn, std::exception_ptr>>;

template<constraint::decayed_type Type, rpp::details::resume_callable ResumeCallable>
struct on_error_resume_next_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, on_error_resume_next_tag>
{

    /**
     * \brief Recover from an onError notification by continuing the sequence without error.
     * \details The operator intercepts an on_error notification from the source Observable and, instead of passing it through to any observers, replaces it with some other item or sequence of items. 
     * \warning This operator potentially allows the resulting Observable to terminate normally or not to terminate at all.
     *
     * \marble on_error_resume_next
       {
           source observable                       : +-1-x
           operator "on_error_resume_next: -9-9-|" : +-1-9-9-|
       }
     *
     * \param resume_callable A callable that is given an error pointer and shall return an Observable.
     * \return new specific_observable with the on_error_resume_next operator as most recent operator.
     * \warning #include <rpp/operators/on_error_resume_next.hpp>
     *
     * \par Examples
     * \snippet on_error_resume_next.cpp on_error_resume_next
     *
     * \ingroup error_handling_operators
     * \see https://reactivex.io/documentation/operators/on_error_resume_next.html
     */
    template<rpp::details::resume_callable ResumeCallable>
    auto on_error_resume_next(ResumeCallable&& resume_callable) const& requires is_header_included<on_error_resume_next_tag, ResumeCallable>
    {
        return cast_this()->template lift<Type>(on_error_resume_next_impl<Type, ResumeCallable>{std::forward<ResumeCallable>(resume_callable)});
    }

    template<rpp::details::resume_callable ResumeCallable>
    auto on_error_resume_next(ResumeCallable&& resume_callable) && requires is_header_included<on_error_resume_next_tag, ResumeCallable>
    {
        return move_this().template lift<Type>(on_error_resume_next_impl<Type, ResumeCallable>{std::forward<ResumeCallable>(resume_callable)});
    }

private:
    const SpecificObservable* cast_this() const
    {
        return static_cast<const SpecificObservable*>(this);
    }

    SpecificObservable&& move_this()
    {
        return std::move(*static_cast<SpecificObservable*>(this));
    }
};

} // namespace rpp::details
