# Advanced Guide

Before hand, please, read [Specific vs Dynamic.md](./Specific%20vs%20Dynamic.md)

## Observable

Observables is just wrappers over callback function with ability to be extended via operators. Everytime you apply some operator to observable, observable copied (or moved). As a result,  whole its state is copied/moved too:
- be ready for it, so, your callback (or any state inside operators) should be cheap enough to copy
- if you want to avoid it, you can convert your observable to dynamic: it forces to move observable to shared_ptr, as a result, no any future copies/moves
- some observables/operators have `memory_model` parameter to change strategy of handling your variable: keep to copy/move or move to shared_ptr once

Everytime you subscribe subscriber observable just invokes callback for this subscriber and nothing else. It means, that actually observable do nothing and doesn't emit values, **callback emit values**.

To achieve better performance use `specific_observable` while it is possible. Same for the argument of callback (for example, when you use `rpp::source::create`): use `auto` for subscriber to avoid implicit conversion to dynamic subscriber.

## Observers

By default, Functional programming deals with immutable data and "pure functions". Observer follow this principle, so, it can accept only const functions for callbacks. 

## Operators

For better compilation speed each operator placed in each own header. Due to great desire to have dot operations inside observable, observable inherits implementation of operators via `member_overload` hack: it forwards interface, but implementation placed in another file. It looks like wide-spread separation to cpp/h files.

Each operator is thread-safe internally and not-thread-safe externally: it means, that all internal staff is guarded or written in lock-free way, but user's types/functions and everything passed to operators/subscribers should be ready to called in parallel (if stream merge multiple streams or any under conditions). For example,
```cpp
auto s = rpp::source::from(rpp::schedulers::new_thread{}, 1,2,3,4,5,6,7,8,9);
s.merge_with(s).take(5).subscribe(user_callback);
```
there 2 sources which emits items from different threads. It means, that after `merge_with` operator it is possible to obtain two different emissions at the same time (for `user_callback`), however operator `take(5)` is guarantees to send only 5 emissions due to internal multi-threaded logic. if you need to force single-threaded emissions for `user_callback`, then you can use `serialize` or `observe_on` operators

If you know, that your stream is single-threaded - then never mind, but if it is multithreaded -> be ready for it.

## Subscriber

Subscriber is just wrapper over observer with subscription. Everytime callback received, subscriber checks for subscription state and emits value to observer if subscription is still active.
