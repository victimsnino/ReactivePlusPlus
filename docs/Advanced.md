# Advanced Guide {#advanced}

## Beforehand

Before hand, please, read \ref specific_vs_dynamic

Also read this one: [Contract](https://reactivex.io/documentation/contract.html)

Let's review this one in details:
> Observables must issue notifications to observers serially (not in parallel). They may issue these notifications from different threads, but there must be a formal happens-before relationship between the notifications.

It means, that:

1. All operators implemented in RPP are following this contract and emissions from observables/operators built-in in RPP are serialized
2. All logic inside operator's callbacks and observer can be not thread-safe due to thread-safety is guaranteed. (for example, `take` operator doesn't use mutexes or atomics due to underlying observable **MUST** emit items serialized)
3. When you implement your own operator via `create` be careful to **follow this contract**!
4. it is true **EXCEPT FOR** subjects if they are used manually due to users can use subjects for its own purposes there is potentially place for breaking this concept. Be careful and use synchronized subjects! 

It means, that for example:
```cpp
    auto s1 = rpp::source::just(1).repeat().subscribe_on(rpp::schedulers::new_thread{});
    auto s2 = rpp::source::just(2).repeat().subscribe_on(rpp::schedulers::new_thread{});
    s1.merge_with(s2).map([](int v){
        std::cout << "enter " << v << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds{1});
        std::cout << "exit " << v << std::endl;
        return v;
    }).as_blocking().subscribe([](int){});
```
will never produce something like 
```
enter 1
enter 2
exit 2
exit 1
```
only serialized
```
enter 1
exit 1
enter 1
exit 1
enter 2
exit 2
enter 2
exit 2
```

## Observable

Observables are just wrappers over callback function with ability to be extended via operators. Everytime you apply some operator to observable, observable is copied (or moved). As a result, whole its state is copied/moved too:

- be ready for it, so, your callback (or any state inside operators) should be cheap enough to copy
- if you want to avoid it, you can convert your observable to dynamic: it forces to move observable to shared_ptr, as a result, no any future copies/moves
- some observables/operators have `memory_model` (\ref memory_model) parameter to change strategy of handling your variable: keep to copy/move or move to shared_ptr once

Everytime you subscribe subscriber observable just invokes callback for this subscriber and nothing else. It means, that actually observable do nothing and doesn't emit values, **callback emits values**.

To achieve better performance use `specific_observable` while it is possible. Same for the argument of callback (for example, when you use `rpp::source::create`): use `const auto&` for subscriber to avoid implicit conversion to dynamic subscriber.

## Observers

By default, functional programming deals with immutable data and "pure functions". Observer follow this principle, so, it can accept only const functions for callbacks. 

## Operators

For better compilation speed each operator placed in each own header. Due to great desire to have dot operations inside observable, observable inherits implementation of operators via `member_overload` hack: it forwards interface, but implementation placed in another file. It looks like wide-spread separation to cpp/hpp files.

## Subscriber

Subscriber is just wrapper over observer with subscription. Everytime callback received, subscriber checks for subscription state and emits value to observer if subscription is still active.
