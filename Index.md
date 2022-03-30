# ReactivePlusPlus

ReactivePlusPlus is [ReactiveX](https://reactivex.io/) library for C++ language inspired by "official implementation" ([RxCpp](https://github.com/ReactiveX/RxCpp)) 


## Performance vs Flexibility: Specific vs Dynamic

In general Reactive Extensions can be split into three main parts from the objects perspective:
- observable
- observer
- subscriber

Each of this objects obtains user-defined callbacks to call when some event happens (on_subscribe, on_next/on_error/on_completed and etc). To store it in C++ we have two ways with its own prons and cons:
- store it explicitly via template type
  - [+] No heap allocations
  - [+] Fast invoking (with possible inlining)
  - [-] Template parameter makes each instance "uniq"
  - [-] No way to store it somehow in containers/class members without pain
- use type-erasure mechanism 
  - [+] Avoiding of template parameters
  - [+] Easy to store in containers/class members/input arguments of functions
  - [-] In most cases heap allocation during constrution
  - [-] More usage of memory even for cheap objects (std::function or pointer to allocated type)
  - [-] Indirect invoking (most probably via pointer or virtual functions)

ReactivePlusPlus provides ability to use both of this ways to make usage experience optimal! Each of mentioned above core parts has two different specifications: `specific_` and `dynamic_`.

`specific_` is default specialization which uses fully template approach to have best performance and memory consumption: each callback/used type specific as template parameter of this class
`dynamic_` specialization uses type-erasure mechanism with all resulting peformance and memory hits, but this specialization is useful if you want to store type as class member or place it inside containers and etc. 

There detailed list with explanations:
- observables:
  - [rpp::specific_observable<T, OnSubscribeFn>](https://victimsnino.github.io/ReactivePlusPlus/docs/html/classrpp_1_1specific__observable.html) - Stores type of `on_subscribe` callback explicitly via `OnSubscribe` template type. Due to explicit type of callback such an observable can obtains ANY specialization of subscriber and ANY specialization of observers without construction of `dynamic_` specializations
  - [rpp::dynamic_observable<T>](https://victimsnino.github.io/ReactivePlusPlus/docs/html/classrpp_1_1dynamic__observable.html) - stores callback as shared_ptr to base class of `rpp::specific_observable` and calling callback via virtual function. Due to this limitation this observable can obtain only `dynamic_observer` and `dynamic_subscriber`
- observers
  - [rpp::specific_observer<T, OnNext, OnError, OnCompleted>](https://victimsnino.github.io/ReactivePlusPlus/docs/html/classrpp_1_1specific__observer.html) - Stores types of `on_next, on_error and on_completed` callbacks explicitly via `OnNext, OnError, OnCompleted` template types.
  - [rpp::dynamic_observer<T>](https://victimsnino.github.io/ReactivePlusPlus/docs/html/classrpp_1_1dynamic__observer.html) - stores callbacks as shared_ptr to base class of `rpp::specific_observer` and calling callbacks via virtual functions.
- subscribers:
  - [rpp::specific_subscriber<T, Observer>](https://victimsnino.github.io/ReactivePlusPlus/docs/html/classrpp_1_1specific__subscriber.html) - stores explicit underlying type of observer as `Observer` template type to avoid construction of [rpp::dynamic_observer](https://victimsnino.github.io/ReactivePlusPlus/docs/html/classrpp_1_1dynamic__observer.html). 
  - [rpp::dynamic_subscriber<T>](https://victimsnino.github.io/ReactivePlusPlus/docs/html/classrpp_1_1dynamic__subscriber.html) - uses [rpp::dynamic_observer](https://victimsnino.github.io/ReactivePlusPlus/docs/html/classrpp_1_1dynamic__observer.html) type as underlying type for observer.

So, to achieve best performance avoid usage of `dynamic_` specialization till you really need it or you want to avoid extra copies/moves of original objects captured inside callbacks and prefer one-time `shared_ptr` allication instead. For example, 
```cpp
auto observable = rpp::observable::create<int>([](const auto& subscriber)
{
    subscriber.on_next(1);
});

auto observer = rpp::specific_observer{[](int v){ std::cout << v;}};
observable.subscribe(observer);
// OR observable.subscribe([](int v){ std::cout << v;});
```
uses only **ONE heap allocation** for `rpp::subscription` inside subscriber to store subscription state and no any other allocation happens. On the other hand
```cpp
//1 dynamic_subscriber in observable
auto observable = rpp::observable::create<int>([](const rpp::dynamic_subscriber<int>& subscriber)
{
    subscriber.on_next(1);
});

observable.subscribe(rpp::specific_observer{[](int v){ std::cout << v;}});

//2 dynamic_observable
rpp::dynamic_observable observable = rpp::observable::create<int>([](const auto& subscriber)
{
    subscriber.on_next(1);
});

observable.subscribe(rpp::specific_observer{[](int v){ std::cout << v;}});

//3 dynamic_observer
auto observable = rpp::observable::create<int>([](const auto& subscriber)
{
    subscriber.on_next(1);
});

observable.subscribe(rpp::dynamic_observer{[](int v){ std::cout << v;}});
```

all this samples uses extra heap allocations:
1. explicitly specify type of subscriber inside observable as `dynamic_`  -> `dynamic_subscriber` created during `subscribe(...)` call which creates `dynamic_observer` underhood -> heap used to construct `dynamic_observer`
2. `dynamic_observable` constructed via heap allocation. This specizalization can obtain only `dynamic_subscriber` -> same as prev. example
3. `dynamic_observer` constructed via heap allocation.

For detailed comparison of performance of different operations for `specific_` and `dynamic_` you can find in [Continuous Benchmarking](https://victimsnino.github.io/ReactivePlusPlus/benchmark)
