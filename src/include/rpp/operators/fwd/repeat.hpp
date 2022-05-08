#pragma once

#include <rpp/observables/member_overload.hpp>

namespace rpp::details
{
struct repeat_tag;
}
namespace rpp::operators
{
/**
 * \copydoc rpp::details::member_overload::repeat
 */
template<typename ...Args>
auto repeat(size_t count) requires details::is_header_included<details::repeat_tag, Args...>;

/**
 * \copydoc rpp::details::member_overload::repeat
 */
template<typename ...Args>
auto repeat() requires details::is_header_included<details::repeat_tag, Args...>;
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, repeat_tag>
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
    * \return new specific_observable with the repeat operator as most recent operator.
    * \warning #include <rpp/operators/repeat.h>
    * \ingroup operators
    */
    template<typename...Args>
    auto repeat(size_t count) const& requires is_header_included<repeat_tag, Args...>
    {
        return repeat_impl(*static_cast<const SpecificObservable*>(this), count);
    }

    template<typename...Args>
    auto repeat(size_t count) && requires is_header_included<repeat_tag, Args...>
    {
        return repeat_impl(std::move(*static_cast<SpecificObservable*>(this)), count);
    }

    /**
    * \brief
    *
    * \details
    *
    * Example:
    *
    * \see
    *
    * \return new specific_observable with the repeat operator as most recent operator.
    * \warning #include <rpp/operators/repeat.h>
    * \ingroup operators
    */
    template<typename...Args>
    auto repeat() const& requires is_header_included<repeat_tag, Args...>
    {
        return repeat_impl(*static_cast<const SpecificObservable*>(this));
    }

    template<typename...Args>
    auto repeat() && requires is_header_included<repeat_tag, Args...>
    {
        return repeat_impl(std::move(*static_cast<SpecificObservable*>(this)));
    }

private:
    template<constraint::decayed_same_as<SpecificObservable> TThis>
    static auto repeat_impl(TThis&& observable, size_t count);

    template<constraint::decayed_same_as<SpecificObservable> TThis>
    static auto repeat_impl(TThis&& observable);
};
} // namespace rpp::details
