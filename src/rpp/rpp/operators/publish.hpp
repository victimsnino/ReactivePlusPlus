#pragma once

#include <rpp/operators/multicast.hpp>

namespace rpp::operators
{
/**
 * @brief Converts ordinary observable to rpp::connectable_observable with help of inline instsantiated publish subject
 * @details Connectable observable is common observable, but actually it starts emissions of items only after call "connect", "ref_count" or any other available way. Also it uses subject to multicast values to subscribers
 * @warning This overloading creates fresh `Subject<Type>` everytime new observable passed to this operator
 *
 * @tparam Subject is template teamplate typename over Subject to be created to create corresponding connectable_observable for provided observable
 * @warning #include <rpp/operators/publish.hpp>
 * 
 * @par Example
 * @snippet multicast.cpp publish
 * 
 * @ingroup connectable_operators
 * @see https://reactivex.io/documentation/operators/publish.html
 */
inline auto publish()
{
    return multicast<rpp::subjects::publish_subject>();
}
}