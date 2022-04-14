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

## Subscriber

Subscriber is just wrapper over observer with subscription. Everytime callback received, subscriber checks for subscription state and emits value to observer if subscription is still active.
