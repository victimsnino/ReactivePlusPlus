// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <rpp/observables/constraints.h>
#include <rpp/observables/member_overload.h>
#include <rpp/observables/type_traits.h>

namespace rpp::details
{
struct merge_tag;
}
namespace rpp::operators
{
/**
 * \copydoc rpp::details::member_overload::merge
 */
template<typename ...Args>
auto merge() requires details::is_header_included<details::merge_tag, Args...>;
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, merge_tag>
{
    /**
    * \brief
    *
    * \details
    *	
    * Example:
    *
    * \see 
    *
    * \return new specific_observable with the merge operator as most recent operator.
    * \warning #include <rpp/operators/merge.h>
    * \ingroup operators
    */
    template<typename ...Args>
    auto merge() const& requires (is_header_included<merge_tag, Args...> && rpp::constraint::observable<Type>)
    {
        return static_cast<const SpecificObservable*>(this)->template lift<utils::extract_observable_type_t<Type>>(merge_impl());
    }

    template<typename ...Args>
    auto merge() && requires (is_header_included<merge_tag, Args...>&& rpp::constraint::observable<Type>)
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<utils::extract_observable_type_t<Type>>(merge_impl());
    }

private:
    static auto merge_impl();
};
} // namespace rpp::details
