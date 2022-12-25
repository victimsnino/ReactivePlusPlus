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
template<constraint::decayed_type           Type,
         constraint::decayed_type           TKey,
         std::invocable<Type>               KeySelector,
         std::invocable<Type>               ValueSelector,
         std::strict_weak_order<TKey, TKey> KeyComparator>
auto group_by_impl(auto&& observable, KeySelector&& key_selector, ValueSelector&& value_selector, KeyComparator&& comparator);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, group_by_tag>
{
    /**
    * \brief Divide original observable into multiple observables where each new observable emits some group of values from original observable.
    *
    * \marble group_by
       {
            source observable              : +--1-2-3-4-5-6-|
            operator "group_by(x=>x%2==0)" :
            {
                                             ..+1---3---5---|
                                             ....+2---4---6-|
            }
       }
    *
    *
    * \details Actually this operator applies `key_selector` to emission to obtain key, place rpp::grouped_observable to map with corresponding map and then send observable with this key (if not yet). Original values emitted via this grouped_observables
    *
    * \param key_selector Function which determines key for provided item
    * \param value_selector Function which determines value to be emitted to grouped observable
    * \param comparator Function to provide strict_weak_order between key types
    *
    * \return new specific_observable with the group_by operator as most recent operator.
    * \warning #include <rpp/operators/group_by.hpp>
    *
    * \par Example:
    * \snippet group_by.cpp group_by
    * \snippet group_by.cpp group_by selector
    *
    * \par Implementation details:
    * - <b>On subscribe</b>
    *    - Allocates one `shared_ptr` to keep map<key, grouped_observable>
    * - <b>OnNext</b>
    *    - Applies key_selector to obtained emission
    *    - For calculated key create new entry in map (if not yet)
    *    - Emit value via grouped_observable from map for corresponding key
    * - <b>OnError</b>
    *    - Just forwards original on_error to both subscribers of observable of grouped observables and grouped observables
    * - <b>OnCompleted</b>
    *    - Just forwards original on_completed to both subscribers of observable of grouped observables and grouped observables
    *
    * \ingroup transforming_operators
    * \see https://reactivex.io/documentation/operators/groupby.html
    */
    template<std::invocable<Type>               KeySelector,
             std::invocable<Type>               ValueSelector = std::identity,
             typename                           TKey          = rpp::utils::decayed_invoke_result_t<KeySelector, Type>,
             std::strict_weak_order<TKey, TKey> KeyComparator = std::less<TKey>>
    auto group_by(KeySelector&& key_selector, ValueSelector&& value_selector = {}, KeyComparator&& comparator = {}) const& requires is_header_included<group_by_tag, KeySelector, ValueSelector, TKey, KeyComparator>
    {
        return group_by_impl<Type, TKey>(*static_cast<const SpecificObservable*>(this), std::forward<KeySelector>(key_selector), std::forward<ValueSelector>(value_selector), std::forward<KeyComparator>(comparator));
    }

    template<std::invocable<Type>               KeySelector,
             std::invocable<Type>               ValueSelector = std::identity,
             typename                           TKey          = rpp::utils::decayed_invoke_result_t<KeySelector, Type>,
             std::strict_weak_order<TKey, TKey> KeyComparator = std::less<TKey>>
    auto group_by(KeySelector&& key_selector, ValueSelector&& value_selector = {}, KeyComparator&& comparator = {}) && requires is_header_included<group_by_tag, KeySelector, ValueSelector, TKey, KeyComparator>
    {
        return group_by_impl<Type, TKey>(std::move(*static_cast<SpecificObservable*>(this)), std::forward<KeySelector>(key_selector), std::forward<ValueSelector>(value_selector), std::forward<KeyComparator>(comparator));
    }    
};
} // namespace rpp::details
