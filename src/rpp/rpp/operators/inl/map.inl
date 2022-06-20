/**
 * \brief Transform the items emitted by an Observable via applying a function to each item and emitting result
 * \note The Map operator can keep same type of value or change it to some another type.
 * 
 * \marble map
    {
        source observable       : +--1   -2   --3   -|
        operator "map: x=>x+10" : +--(10)-(12)--(13)-|
    }
 *
 * \param callable is callable used to provide this transformation. Should accept Type of original observable and return type for new observable
 * \return new specific_observable with the Map operator as most recent operator.
 * \warning #include <rpp/operators/map.hpp>
 * 
 * \par Example with same type:
 * \snippet map.cpp Same type
 *
 * \par Example with changed type:
 * \snippet map.cpp Changed type
 * \ingroup transforming_operators
 * \see https://reactivex.io/documentation/operators/map.html
 */
template<std::invocable<Type> Callable>
auto map(Callable&& callable) const & requires rpp::details::is_header_included<rpp::details::map_tag, Callable>
{
    return CastThis().lift<std::invoke_result_t<Callable, Type>>(rpp::details::map_impl<Type, std::decay_t<Callable>>{std::forward<Callable>(callable)});
}

template<std::invocable<Type> Callable>
auto map(Callable&& callable) && requires rpp::details::is_header_included<rpp::details::map_tag, Callable>
{
    return MoveThis().lift<std::invoke_result_t<Callable, Type>>(rpp::details::map_impl<Type, std::decay_t<Callable>>{std::forward<Callable>(callable)});
}