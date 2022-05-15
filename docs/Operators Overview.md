# Operators Overview

## Creational

### empty/never/error
Created most simple observables which:
- send `on_completed` only
- do nothing
- sends provided error


### create
Creates observable with user-defined callback function which accepts subscriber and returns nothing. Resulting observable is `rpp::specific_observable`
```cpp
rpp::source::create<int>([](const auto& sub)
        {
            while(sub.is_subscribed())
                sub.on_next(42);
            sub.on_completed();
        })
        .subscribe([](int v) { std::cout << v << std::endl; });
```

**Performance tip:** prefer usage `const auto&` as type of subscriber to avoid implicit conversion.
**Usage tip:** avoid usage of this function and prefer built-in functions
See: https://reactivex.io/documentation/operators/create.html


### just
Creates observable which emits values passed to this function.
```cpp
rpp::source::just(42, 53, 10, 1).subscribe([](int v) { std::cout << v << std::endl; });
// Output: 42 53 10 1
```
Additionally, can be configured with:
- memory model over passed values:
    ```cpp
    std::array<int, 100> cheap_to_copy_1{};
    std::array<int, 100> cheap_to_copy_2{};
    rpp::source::just<rpp::memory_model::use_shared>(cheap_to_copy_1, cheap_to_copy_2).subscribe();
    ```
    in this case values will be immediately copied/moved to shared_ptr to prevent future copies
- scheduler
    ```cpp
    rpp::source::just(rpp::schedulers::new_thread{}, 42, 53).subscribe();
    ```
    in this case values will be sent via scheduler when next value will be scheduled after previous one is successfuly sent.
See: https://reactivex.io/documentation/operators/just.html

### from
Creates observable from some another complex type:
- **iterable**
  iterates over provided iterable and emits each value (something similar to just(...)). Can be configured with memory_model and scheduler
  ```cpp
    std::vector<int> vals{ 1,2,3 };
    rpp::source::from_iterable<rpp::memory_model::use_shared>(vals, rpp::schedulers::new_thread{}).subscribe([](int v) {std::cout << v << " "; });
  ```
- **callable**
  invoke callable and sends returning value Can be configured with memory_model
  ```cpp
    rpp::source::from_callable<rpp::memory_model::use_shared>([](){return 42;}).subscribe([](int v) {std::cout << v << " "; });
  ```
See: https://reactivex.io/documentation/operators/from.html

## Transforming
### map
Operator with ability to transform obtained value and return modified value
```cpp
rpp::source::create<int>([](const auto& sub)
    {
        sub.on_next(42);
    })
    .map([](int value)
    {
        return std::to_string(value) + " VAL";
    })
    .subscribe([](std::string v) { std::cout << v << std::endl; });
```
See: https://reactivex.io/documentation/operators/map.html

## Filtering
### filter
Operator which emits only items satisfied some condition
```cpp
rpp::source::create<int>([](const auto& sub)
        {
            for (int i = 0; i < 10; ++i)
                sub.on_next(i);
        })
        .filter([](int    v) { return v % 2 == 0; })
        .subscribe([](int v) { std::cout << v << " "; });
// Output: 0 2 4 6 8
```
See: https://reactivex.io/documentation/operators/filter.html

### take
Limits amount of submission to provided count and then sends `on_completed`
```cpp
rpp::source::create<int>([](const auto& sub)
        {
            for (int i = 0; i < 10; ++i)
                sub.on_next(i);
        })
        .take(2)
        .subscribe([](int v) { std::cout << v << " "; });
// Output: 0 1
```
See: https://reactivex.io/documentation/operators/take.html

### take_while
Emits items from underlying observable while predicate satisfied. On first false sends `on_completed`
```cpp
rpp::source::create<int>([](const auto& sub)
        {
            for (int i = 0; i < 10; ++i)
                sub.on_next(i);
        })
        .take_while([](int v) { return v != 5; })
        .subscribe([](int  v) { std::cout << v << " "; });
// Output: 0 1 2 3 4
```
See: https://reactivex.io/documentation/operators/takewhile.html

## Combining
### merge
Combines emissions of multiple observables. Subscribes on all of them at the same time and emit items immediately as the appears.

There is two overloadings:
- `merge()` for observable (1) of observables (2): merges emissions from (2) observables
  for example
  ```cpp
  rpp::source::just(rpp::source::just(1).as_dynamic(),
                    rpp::source::never<int>().as_dynamic(),
                    rpp::source::just(2).as_dynamic())
        .merge()
        .subscribe([](int v) { std::cout << v << " "; });
  // Output: 1 2
  ```
- `merge_with(observables...)` merge submissions of original observable with others
  ```cpp
  rpp::source::just(1)
        .merge_with(rpp::source::just(2))
        .subscribe([](int v) { std::cout << v << " "; });
  // Output: 1 2
  ```
See: https://reactivex.io/documentation/operators/merge.html

## Utility
### observe_on
Transfers emissions of items to provided scheduler with goal to provided multithreaded behaviour
```cpp
std::cout << std::this_thread::get_id() << std::endl;
rpp::source::just(10, 15, 20)
        .observe_on(rpp::schedulers::new_thread{})
        .as_blocking()
        .subscribe([](int v) { std::cout << "[" << std::this_thread::get_id() << "] : " << v << "\n"; });
// Template for output:
// TH1
// [TH2]: 10
// [TH2]: 15
// [TH2]: 20
```
See: https://reactivex.io/documentation/operators/observeon.html

### repeat
Forces observable to re-subscribe observer to this observable after on_completed...
- with provided count
  ```cpp
  rpp::source::just(1, 2, 3)
          .repeat(2)
          .subscribe([](int v) { std::cout << v << " "; },
                      []()
                      {
                          std::cout << "completed" << std::endl;
                      });
  // Output: 1 2 3 1 2 3 completed
  ```
- infinitely
  ```cpp
  rpp::source::just(1, 2, 3)
            .repeat()
            .take(10)
            .subscribe([](int v) { std::cout << v << " "; },
                       []()
                       {
                           std::cout << "completed" << std::endl;
                       });
    // Output: 1 2 3 1 2 3 1 2 3 1 completed
  ```
See: https://reactivex.io/documentation/operators/repeat.html

## Connectable
### multicast
Converts common Observable to ConnectableObservable via provided subject. It means, that each submission will be multicasted to all observers subscribed till current moment + emissions will be started only after call of connect, ref_count or something like this.
```cpp
auto subject = rpp::subjects::publish_subject<int>{};
auto observable = rpp::source::just(1, 2, 3).multicast(subject);
observable.subscribe([](int v) {std::cout << "#1 " << v << std::endl; });
observable.subscribe([](int v) {std::cout << "#2 " << v << std::endl; });
observable.connect();
// Output:
// #1 1
// #2 1
// #1 2
// #2 2
// #1 3
// #2 3
```
See: https://reactivex.io/documentation/operators/publish.html

### publish
Same as multicast, but it uses publish_subject by default.
See: https://reactivex.io/documentation/operators/publish.html

### ref_count
Forces connectable observable to behave like common observable.  Connects Connectable Observable on the first subscription and unsubscribes on last unsubscription
```cpp
auto observable = rpp::source::just(1, 2, 3).publish();
observable.subscribe([](int v) {std::cout << "#1 " << v << std::endl; });
// No Output

observable.ref_count().subscribe([](int v) {std::cout << "#2 " << v << std::endl; });
// Output:
// #1 1
// #2 1
// #1 2
// #2 2
// #1 3
// #2 3
```
See: https://reactivex.io/documentation/operators/refcount.html
