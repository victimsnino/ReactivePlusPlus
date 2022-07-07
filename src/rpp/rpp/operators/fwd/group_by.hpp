#pragma once

#include <rpp/observables/details/member_overload.hpp>
#include <rpp/utils/function_traits.hpp>

#include <concepts>
#include <functional>


namespace rpp::details
{
struct group_by_tag;
}

namespace rpp::details
{
template<constraint::decayed_type  Type,
         constraint::decayed_type  TKey,
         std::invocable<Type>      KeySelector,
         std::invocable<Type>      ValueSelector,
         std::relation<TKey, TKey> KeyComparator>
auto group_by_impl(auto&& observable, KeySelector&& key_selector, ValueSelector&& value_selector, KeyComparator&& comparator);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, group_by_tag>
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
    * \return new specific_observable with the group_by operator as most recent operator.
    * \warning #include <rpp/operators/group_by.hpp>
    * \ingroup operators
    */
    template<std::invocable<Type>      KeySelector,
             std::invocable<Type>      ValueSelector = std::identity,
             typename                  TKey          = rpp::utils::decayed_invoke_result_t<KeySelector, Type>,
             std::relation<TKey, TKey> KeyComparator = std::less<TKey>>
    auto group_by(KeySelector&& key_selector, ValueSelector&& value_selector = {}, KeyComparator&& comparator = {}) const& requires is_header_included<group_by_tag, KeySelector, ValueSelector, TKey, KeyComparator>
    {
        return group_by_impl<Type, TKey>(*static_cast<const SpecificObservable*>(this), std::forward<KeySelector>(key_selector), std::forward<ValueSelector>(value_selector), std::forward<KeyComparator>(comparator));
    }

    template<std::invocable<Type>      KeySelector,
             std::invocable<Type>      ValueSelector = std::identity,
             typename                  TKey          = rpp::utils::decayed_invoke_result_t<KeySelector, Type>,
             std::relation<TKey, TKey> KeyComparator = std::less<TKey>>
    auto group_by(KeySelector&& key_selector, ValueSelector&& value_selector = {}, KeyComparator&& comparator = {}) && requires is_header_included<group_by_tag, KeySelector, ValueSelector, TKey, KeyComparator>
    {
        return group_by_impl<Type, TKey>(std::move(*static_cast<SpecificObservable*>(this)), std::forward<KeySelector>(key_selector), std::forward<ValueSelector>(value_selector), std::forward<KeyComparator>(comparator));
    }    
};
} // namespace rpp::details
