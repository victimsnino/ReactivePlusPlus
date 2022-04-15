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
    

### from
Creates observable from some another complex type:
- **iterable**
  iterates over provided iterable and emits each value (something similar to just(...)). Can be configured with memory_model and scheduler
  ```cpp
    std::vector<int> vals{ 1,2,3 };
    rpp::source::from<rpp::memory_model::use_shared>(vals, rpp::schedulers::new_thread{}).subscribe([](int v) {std::cout << v << " "; });
  ```

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
- `merge(observables...)` merge submissions of original observable with others