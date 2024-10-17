# Manual documentation

[TOC]

## Introduction to Reactive Programming

> [!IMPORTANT]
> It's highly recommended to read this article beforehand: [The introduction to Reactive Programming you've been missing](https://gist.github.com/staltz/868e7e9bc2a7b8c1f754)


**Reactive programming** is a *design paradigm* that focuses on building applications that can efficiently respond to asynchronous **events**.

Every application or function has two core parts: input and output. Input/output can even be empty:

```cpp
void function() { }
```

Input/output can be categorized into two types:

- **Static** - The application or function accepts input and handles it. For example, command line arguments or function arguments:

```cpp
int sum(int a, int b) { return a + b; }
```

- **Distributed in time** - The application or function doesn't know the exact length of input or when it will arrive but knows what to do when it happens:

```cpp
#include <iostream>

int main()
{
  while(true)
  {
     auto ch = ::getchar();
     std::cout << "Obtained char " << ch << std::endl;
  }
}
```

When dealing with input that is **distributed in time**, there are two ways to handle it:

- **Pulling** - You decide when you need extra data and request it. This is often a blocking operation. For example, manually checking a blog for new posts.

- **Pushing** - You register interest in a data source and react when new data becomes available. For example, subscribing to a blog and receiving notifications for new posts.

Reactive programming is a powerful way to handle input that is **distributed in time**. Instead of constantly polling for updates, reactive programming allows you to **register** callbacks to be executed **when the input becomes available**.

See <https://reactivex.io/intro.html> for more details.

## Core Concepts

Reactive Programming can be described as follows:

- An **Observer** subscribes to an **Observable**.
- The **Observable** notifies its subscribed **Observers** about new events/emissions:
  - **on_next(T)** - notifies about a new event/emission
  - **on_error(std::exception_ptr)** - notifies about an error. This is a termination event. (no more calls from this observable should be expected)
  - **on_completed()** - notifies about successful completion. This is a termination event. (no more calls from this observable should be expected)
  - **set_upstream(disposable)** - observable could pass to observer it's own disposable to provide ability for observer to terminate observable's internal actions/state
  - **is_disposed()** - checks if the observer is still interested in the source data.

During subscription, the **Observable** can provide a **Disposable** for the **Observer** to check if the observable is still alive or to terminate early if needed.

For example:

```cpp
#include <rpp/rpp.hpp>
#include <iostream>

int main()
{
    rpp::source::create<int>([](const auto& observer)
    {
        while (!observer.is_disposed())
        {
            char ch = ::getchar();
            if (!::isdigit(ch))
            {
              observer.on_error(std::make_exception_ptr(std::runtime_error{"Invalid symbol"}));
              return;
            }
            int digit = ch - '0';
            if (digit == 0)
            {
                observer.on_completed();
                return;
            }
            observer.on_next(digit);
        }
    })
    .subscribe([](int val)
               {
                   std::cout << "obtained val " << val << std::endl;
               },
                [](std::exception_ptr err)
                {
                   std::cout << "obtained error " << std::endl;
                },
               []()
               {
                   std::cout << "Completed" << std::endl;
               });
    // input: 123456d
    // output:  obtained val 1
    //          obtained val 2
    //          obtained val 3
    //          obtained val 4
    //          obtained val 5
    //          obtained val 6
    //          obtained error

    // input: 1230
    // output: obtained val 1
    //         obtained val 2
    //         obtained val 3
    //         Completed

    return 0;
}
```

There we are creating observable that emits digits from console input:In case of user promted something else it is **error** for our observable (it is expected to emit ONLY digits). In this case we are notifying observer about it and just stopping. When user prompts `0`, it means "end of observable".

See <https://reactivex.io/documentation/observable.html> for more details.

In such an way it is not powerful enough, so Reactive Programming provides a list of **operators**.

### Observable contract

\copydoc observables

### Operators

\copydoc operators

#### How Operators Work?

Example:

```cpp
rpp::source::create<int>([](const auto& observer){
  observer.on_next(1);
  observer.on_completed();
});
```

This example shows next: we create observble of `int` via operator `create`. This observable just emits to observer value `1` and then completes. Type of this observable is `rpp::observable<int, ...>` where `...` implementation defined type. So, actually it is `observable of ints`. Let's say we want to convert `int` to `std::string`. We could subscribe and then convert it or use `map` operator (also known as `transform`) to transform some original value to some another value:

```cpp
rpp::source::create<int>([](const auto& observer){
  observer.on_next(1);
  observer.on_completed();
})
| rpp::operators::map([](int v){ return std::string{v}; });
```

For now it is `observable of strings` due to it is `rpp::observable<std::string, ...>`. But what is `rpp::operators::map` then? Actually it is functor-adaptor - just functor accepting observable and returning another observable. It accepts original observable and converts it to observable of "final type". "final type" is result of invocation of passed function against original observable's type. In our case it is `decltype([](int v){ return std::string{v}; }(int{}))` is it is `std::string`. So, `map` can be implemented in the following way:

```cpp
template<typename Fn>
struct map
{
  Fn fn{};

  template<typename Type, typename Internal>
  auto operator()(const rpp::observable<Type, Internal>& observable) const {
    using FinalType = std::invoke_result_t<Fn, Type>;
    return rpp::source::create<FinalType>([observable, fn](const rpp::dynamic_observer<FinalType>& observer)
    {
      observable.subscribe([observer, fn](const auto& v) { observer.on_next(fn(v)); },
                           [observer](const std::exception_ptr& err) { observer.on_error(err); },
                           [observer]() { observer.on_completed(); });
    };);
  }
}
```

It is template for such an functor-adaptor. Provided example - is simplest possible way to implement new operators - just provide function for transformation of observable. For example, it is fully valid example:
```cpp
rpp::source::just(1)
    | [](const auto& observable) { return rpp::source::concat(observable, rpp::source::just(2)); };
```

There we convert observable to concatenation of original observable and `just(2)`.

One more posible but a bit more advanced way to implement operators - is to lift observer. To do this, your functor-adapter must to satisfy `rpp::constraint::operator_lift` concept. Actually, your class must to have:
- member function `lift` accepting downstream observer and returning new upstream observer
- inner `template<rpp::constraint::decayed_type T> struct traits` struct accepting typename of upstream and providing:
  - `using result_type =` with typename of new resulting type for new observable
  - (optionally) `struct requirements` with static_asserts over passed type

Example:
```cpp
template<typename Fn>
struct map
{
  template<rpp::constraint::decayed_type T>
  struct traits
  {
    struct requirements
    {
      static_assert(std::invocable<Fn, T>, "Fn is not invocable with T");
    };

    using result_type = std::invoke_result_t<Fn, T>;
  };

  Fn fn{};

  template<typename Upstream, typename Downstream>
  auto lift(const rpp::dynamic_observer<Downstream>& observer) const
  {
      return rpp::make_lambda_observer<Upstream>([observer, fn](const auto& v){ observer.on_next(fn(v)); },
                                                 [observer](const std::exception_ptr& err) { observer.on_error(err); },
                                                 [observer]() { observer.on_completed(); });
  }
}

```
In this case you providing logic how to convert downstream observer to upstream observer. Actually this implementation is equal to previous one, but without handling of observable - you are expressing your operator in terms of observers

**(Advanced)**
In case of implementing operator via `lift` you can control disposable strategy via `updated_optimal_disposable_strategy` parameter. It accepts disposable strategy of upstream and returns disposable strategy for downstream. It needed only for optimization and reducing disposables handling cost and it is purely advanced thing. Not sure if anyone is going to use it by its own for now =)

### Schedulers

Reactive programming becomes even more powerful when observables can operate across multiple threads, rather than being confined to the thread of creation and subscription. This allows for non-blocking, asynchronous operations and provides significant advantages over raw iteration or other pull-based approaches. To enable multithreading in your observables, you can use **Schedulers**.

By default, an **Observable** will perform its work in the thread where the **subscribe** operation occurs. However, you can change this behavior using the **subscribe_on** operator. This operator forces the observable to perform the **subscription** and any subsequent work in the specified **scheduler**. For example:

```cpp
rpp::source::just(1,2,3,4,5)                                  // 1
| rpp::operators::subscribe_on(rpp::schedulers::new_thread{}) // 2
| rpp::operators::subscribe([](auto){});                      // 3
```

In this case subscribe to `3` happens in current thread (where subscribe invoked). But during subscription to `2` it schedules subscription to `1` to provided `new_thread` scheduler. So, subscription to final observable and it's internal logic (iterating and emitting of values) happens inside new_thread. Actually it is something like this:
```cpp
rpp::source::create<int>([](const auto& observer)
{
  rpp::schedulers::new_thread{}.create_worker([](...) {
    rpp::source::just(1,2,3,4,5).subscribe(observer);
  })
}).subscribe(...);
```

The **observe_on** operator specifies the **scheduler** that will be used for emission during the processing of further operators after **observe_on**. For example

```cpp
rpp::source::just(1,2,3,4,5)
| rpp::operators::observe_on(rpp::schedulers::new_thread{})
| rpp::operators::subscribe([](auto){});
```

In this case whole subscription flow happens in thread of subscription, but emission of values transfers to another thread. Actually it is something like this:
```cpp
rpp::source::create<int>([](const auto& observer)
{
  auto worker = rpp::schedulers::new_thread{}.create_worker();
  rpp::source::just(1,2,3,4,5).subscribe([](int v) {
    worker.scheduler([](...) {
      observer.on_next(v);
    }
  })
}).subscribe(...);
```

A **Scheduler** is responsible for controlling the type of multithreading behavior (or lack thereof) used in the observable. For example, a **scheduler** can utilize a new thread, a thread pool, or a raw queue to manage its processing.


Check the [API Reference](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/group__schedulers.html) for more details about schedulers.

See <https://reactivex.io/documentation/scheduler.html> for more details about schedulers.

### Disposable

In reactive programming, a **disposable** is an object that represents a resource that needs to be released or disposed of when it is no longer needed. This can include things like file handles, network connections, or any other resource that needs to be cleaned up after use.

The purpose of a disposable is to provide a way to manage resources in a safe and efficient manner. By using disposables, you can ensure that resources are released in a timely manner, preventing memory leaks and other issues that can arise from resource leaks.

In most cases disposables are placed in observers. RPP's observer can use two types of disposables:

1. **Upstream disposable** - This is a disposable that the observable puts into the observer. The upstream disposable keeps some state or callback that should be disposed of when the observer is disposed. This ensures that any resources used by the observable are properly cleaned up when the observer obtains on_error/on_completed or disposed in any other way.

2. **External disposable** - This is a disposable that allows the observer to be disposed of from outside the observer itself. This can be useful in situations where you need to cancel an ongoing operation or release resources before the observable has completed its work.

### Exception Guarantee

In non-reactive programming functions/modules throws exception in case of something invalid. As a result, user can catch it and handle it somehow while internal state of objects can be in some state (invalid/untouched/partly valid) and etc.

In reactive programming there is another way of exception mechanism: throwing exception as is from original place is useless. Notification about "something goes wrong" need to receive observer/subscriber, not owner of callstack. As a result, ANY exception obtained during emitting items and etc WOULD be delivered to subscriber/observer via `on_error` function and then unsubscribe happens. As a result, no any raw exceptions would be throws during using RPP. In case of emitting `on_error` whole internal state of observable keeps valid but it doesn't matter - whole chain would be destroyed due to `on_error` forces unsubscribe. Reactive catching mechanisms like `catch` or `retry` **re-subscribes** on observable. it means, that new chain with new states would be created, not re-used existing one.

### Memory Model

In ReactivePlusPlus there is new concept unique for this implementation: rpp::memory_model:

Some of the operators and sources like `rpp::source::just` or `rpp::operators::start_with` accepts user's variables for usage. Some of this types can be such an expensive to copy or move and it would be preferable to copy it once to heap, but some other types (like POD) are cheap enough and usage of heap would be overkill. But these variables should be saved inside somehow!

So, RPP provides ability to select strategy "how to deal with such a variables" via `rpp::memory_model` enum.

#### Examples

For example, `rpp::source::just`

```cpp
rpp::source::just(my_custom_variable);
```
by default `just` uses `rpp::memory_model::use_stack` and `my_custom_variable` would be copied and moved everywhere when needed. On the other hand

```cpp
rpp::source::just<rpp::memory_model::use_shared>(my_custom_variable);
```
makes only 1 copy/move to shared_ptr and then uses it instead.

As a a result, users can select preferable way of handling of their types.


## Advanced details

### Disposable

Rpp has following disposables related classes:
- `interface_disposable` - is base inerface for all disposables in RPP. Simplest ever disposable with `dispose()` and `is_disposed()` method. This type of disposable observable is passing to observer.
  - `callback_disposable` - is just **noexcept** to be called on dispose. Can be constructed like this:
  ```cpp
  auto d = rpp::make_callback_disposable([]() noexcept { std::cout << "DISPOSED! " << std::endl; });
  ```
- `interface_composite_disposable` - is base interface for disposables able to keep dependent disposables inside: main difference - new method `add` accepting another dispoable inhereting from `interface_disposable`. Main idea: `interface_composite_disposable` is aggregating other disposables inside and during `dispose()` method calling `dispose()` method of its dependents.
  - `composite_disposable` - is concrete realization of `interface_composite_disposable`
  - `refcount_disposable` - is variant of `composite_disposable` but it keeps refcounter inside. This counter can be incremented with help of `add_ref()` method returning new dependent `composite_disposable`. Idea is simple: original `refcount_disposable` would be disposed IF all of its dependents disposables (created via `add_ref()` ) `dispose()` methods were called.

All disposable in RPP should be created and used via `rpp::disposable_wrapper_impl<T>` wrapper. For simplicity usage it has 2 base aliases:
- `disposable_wrapper` - wrapper over `interface_disposable`
- `composite_disposable_wrapper` - wrapper over `interface_composite_disposable`

`disposable_wrapper` is kind of smart_pointer (like std::unique_ptr) but for disposables. So, default constructed wrapper is empty wrapper.
```cpp
auto d = rpp::disposable_wrapper{};
```
Comparing to unique_ptr wrapper's methods are safe to use for empty wrapper.
To construct wrapper you have to use `make` method:
```cpp
auto d = rpp::disposable_wrapper::make<SomeSpecificDisposableType>(some_arguments, to_construct_it);
```

Wrapper has popluar methods to work with disposable: `dispose()`, `is_disposed()` and `add()`/`remove()`/`clear()` (for `interface_composite_disposable`).

In case of you want to obtain original disposable, you can use `lock()` method returning shared_ptr.

`disposable_wrapper` can be strong and weak:
- strong (it is default behavior) is keeping disposable as shared_ptr, so, such an instance of wrapper is extending life-time is underlying disposable
- weak (disposable_wrapper can be forced to weak via `as_weak()` method) is keeping disposable as weak_ptr, so, such an instance of wrapper is **NOT** extendning life-time is underlying disposable

This wrapper is needed for 2 goals:
- provide safe usage of disposables avoiding manual handling of empty/weak disposables
- automatically call `dispose()` during destruction of any disposable

To achieve desired performance RPP is avoiding to returning disposable by default. So, it is why `subscribe` method is not returning anything by default. If you want to attach disposable to observer you can use overloading method accepting disposable as first argument like this:
```cpp
auto d = rpp::composite_disposable_wrapper::make();
observable.subscribe(d, [](int v){});
```
or use `subscribe_with_disposable` method instead
```cpp
auto d = observable.subscribe_with_disposable([](int){});
```

### dynamic_* versions to keep classes as variables

Most of the classes inside rpp library including `observable`, `observer` and others are heavy-templated classes. It means, it could has a lot of template params. In most cases you shouldn't worry about it due to it is purely internal problem.

But in some cases you want to keep observable or observer inside your classes or return it from function. In most cases I strongly recommend you to use `auto` to deduce type automatically. But in some cases it is not possible (for example, to keep observable as member variable). For such an usage you could use `dynamic_observable` and `dynamic_observer`:
- they are type-erased wrappers over regular observable/observer with goal to hide all unnecessary stuff from user's code. For example, you can easily use it as:
```cpp
#include <rpp/rpp.hpp>
#include <iostream>

struct some_data
{
  rpp::dynamic_observable<int> observable;
  rpp::dynamic_observer<int> observer;
};

int main() {
    some_data v{rpp::source::just(1,2,3),
                rpp::make_lambda_observer([](int value){
                    std::cout << value << std::endl;
                })};

    v.observable.subscribe(v.observer);
}
```
- to convert observable/observer to dynamic_* version you could manually call `as_dynamic()` member function or just pass them to ctor
- actually they are similar to rxcpp's `observer<T>` and `observable<T>` but provides EXPLICIT definition of `dynamic` fact
- due to type-erasure mechanism `dynamic_` provides some minor performance penalties due to extra usage of `shared_ptr` to keep internal state + indirect calls. It is not critical in case of storing it as member function, but could be important in case of using it on hot paths like this:
```cpp
rpp::source::just(1,2,3)
| rpp::ops::map([](int v) { return rpp::source::just(v); })
| rpp::ops::flat_map([](rpp::dynamic_observable<int> observable) {
  return observable | rpp::ops::filter([](int v){ return v % 2 == 0;});
});
```
^^^ while it is fully valid code, `flat_map` have to convert observable to dynamic version via extra heap, but it is unnecessary. It is better to use `auto` in this case.
```cpp
rpp::source::just(1,2,3)
| rpp::ops::map([](int v) { return rpp::source::just(v); })
| rpp::ops::flat_map([](const rpp::constraint::observable_of_type<int> auto& observable) { // or just `const auto& observable`
return observable | rpp::ops::filter([](int v){ return v % 2 == 0;});
});
```

## Extensions:

RPP is library to build reactive streams. But in general applicaton uses some another framework/library to build core logic of application. With some of them RPP can be unified to build much more better software.
Below you can find list of extensions for RPP with adaption to external frameworks for much more easiser integeration with RPP. These extensions are part of RPP library:

### rppqt

\copydoc rppqt
Check API reference of \link rppqt \endlink for more details

### rppgrpc

\copydoc rppgrpc
Check API reference of \link rppgrpc \endlink for more details

### rppasio

\copydoc rppasio
Check API reference of \link rppasio \endlink for more details
