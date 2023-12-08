# User Guide

## Introduction

I'm highly recommend to read this article: [The introduction to Reactive Programming you've been missing](https://gist.github.com/staltz/868e7e9bc2a7b8c1f754)

### What is Reactive Programming?

**Reactive programming** is a *design paradigm* that focuses on building applications that can efficiently respond to asynchronous **events**.

Actually, any application or function has two core parts: input and output. Input/output can even be empty:

```cpp
int main()
{
  return 0;
}
```

Input/output itself can be split into the following two types:

- **Static** - Your application or function just accepts such an input and handles it somehow. For example, arguments from the command line or arguments of your function:

```cpp
int sum(int a, int b) { return a + b; }
```

- **Distributed in time** - Your application or function doesn't know exact length of input, **when** input (or any parts of it) would arrive, but knows **what** to do when it happens:

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

- **Pulling** - You decide **when** you need extra data (e.g., to get something, request, iterate, etc.) and you are simply **checking/requesting** some data. In most cases, this is a blocking operation of requesting data and waitign to be available or periodically checking its current status. For example, if you like a blog with non-periodical posts, you may check it daily for new posts manually.

- **Pushing** - You decide **once** that you are interested in a source of data, notify this source somehow (e.g., register, subscribe, etc.), and **react** when new data **becomes available** to you. For example, you might **subscribe** to a blog and **react** to new posts only after receiving a notification on your smartphone, rather than manually checking for updates.

Reactive programming is a powerful way to handle input that is **distributed in time**. Instead of constantly polling for updates or waiting for input to arrive, reactive programming allows you to **register** callbacks to be executed **when the input becomes available**.

See <https://reactivex.io/intro.html> for more details.

## Core concepts of Reactive Programming

In short, Reactive Programming can be described as follows:

- An **Observer** subscribes to an **Observable**.
- The **Observable** automatically notifies its subscribed **Observers** about any new events/emissions. **Observable** could invoke next **observer**'s method:
  - **on_next(T)** - notifies about new event/emission
  - **on_error(std::exception_ptr)** - notified about error during work. It is termination event (no more calls from this observable should be expected)
  - **on_completed()**  - notified about successful completion.It is termination event (no more calls from this observable should be expected)
  - **set_upstream(disposable)** - observable could pass to observer it's own disposable to provide ability for observer to terminate observable's internal actions/state.
  - **is_disposed()** - observable could check if observer is still interested in this source data (==false) or disposed and not listening anymore (==true)
- During subscription, the **Observable** can return/provide a **Disposable** for **Observer** to provide ability to check if observable is still alive or make early termination (==dispose) if needed.

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

Reactive programming has [Observable Contract](https://reactivex.io/documentation/contract.html). Please, read it.

This contact has next important part:

> Observables must issue notifications to observers serially (not in parallel). They may issue these notifications from different threads, but there must be a formal happens-before relationship between the notifications

RPP follows this contract and especially this part. It means, that:

1. **All** implemented in **RPP operators** are **following this contract**:<br>
    All built-in RPP observables/operators emit emissions serially
2. Any user-provided callbacks (for operators or observers) can be not thread-safe due to thread-safety of observable is guaranteed. <br>
   For example: internal logic of `take` operator doesn't use mutexes or atomics due to underlying observable **MUST** emit items serially
3. When you implement your own operator via `create` be careful to **follow this contract**!
4. It is true **EXCEPT FOR** subjects if they are used manually due to users can use subjects for its own purposes there is potentially place for breaking this concept. Be careful and use synchronized subjects instead if can't guarantee serial emissions!

It means, that for example:

```cpp
    auto s1 = rpp::source::just(1) | rpp::operators::repeat() | rpp::operators::subscribe_on(rpp::schedulers::new_thread{});
    auto s2 = rpp::source::just(2) | rpp::operators::repeat() | rpp::operators::subscribe_on(rpp::schedulers::new_thread{});
    s1 | rpp::operators::merge_with(s2)
       | rpp::operators::map([](int v)
      {
        std::cout << "enter " << v << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds{1});
        std::cout << "exit " << v << std::endl;
        return v;
      })
      | rpp::operators::as_blocking()
      | rpp::operators::subscribe([](int){});

```

will never produce something like

```log
enter 1
enter 2
exit 2
exit 1
```

only serially

```log
enter 1
exit 1
enter 1
exit 1
enter 2
exit 2
enter 2
exit 2
```

### Operators

**Operators** are way to modify the **Observable**'s emissions to adapt values to the **Observer**.

For example, we can create observable to get chars from console input, do it till ‘0’ char, get only letters and send to observer this letters as UPPER. With operators it is pretty simple to do it in correct way:

```cpp
#include <rpp/rpp.hpp>

#include <iostream>

int main()
{
  rpp::source::from_callable(&::getchar)
    | rpp::operators::repeat()
    | rpp::operators::take_while([](char v) { return v != '0'; })
    | rpp::operators::filter(std::not_fn(&::isdigit))
    | rpp::operators::map(&::toupper)
    | rpp::operators::subscribe([](char v) { std::cout << v; });

  // input: 12345qwer5125ttqt0
  // output: QWERTTQT

  return 0;
}
```

You can check documentation for each operator on [API Reference](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/group__rpp.html) page. Below you can find details about how operator works and how to create your own custom operator in RPP.

See <https://reactivex.io/documentation/operators.html> for more details about operators concept.


#### How operator works?

Let's check this example:

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
    return rpp::source::create<FinalType>([observable, fn](const auto& observer)
    {
      observable.subscribe([observer, fn](const auto& v) { observer.on_next(fn(v)); },
                           [observer](const std::exception_ptr& err) { observer.on_error(err); },
                           [observer]() { observer.on_completed(); });
    };);
  }
}
```

It is template for such an functor-adaptor. It is actually valid except of the one thing: observer is not copyable by default and we need to handle it, for example, call `as_dynamic()`  or to place in some shared_ptr, but it is out of current explanation.

Provided example - is simplest possible way to implement new operators - just provide function for transformation of observable. For example, it is fully valid example:
```cpp
rpp::source::just(1)
    | [](const auto& observable) { return rpp::source::concat(observable, rpp::source::just(2)); };
```

There we convert observable to concatenation of original observable and `just(2)`.

One more posible but a bit more advanced way to implement operators - is to lift observer. To do this, your functor-adapter must to satisfy `rpp::constraint::operator_lift` concept. Actually, your class must to have:
- member function `lift` accepting downstream observer and returning new upstream observer
- using `result_value<T>` accepting typename of upstream and providing new value for downstream (== typename of original observable and return new resulting type for new observable)

Example:
```cpp
template<typename Fn>
struct map
{
  template<typename T>
  using result_value = std::invoke_result_t<Fn, T>;

  Fn fn{};

  template<typename Upstream, typename Downstream>
  auto lift(rpp::constraint::observer_of_type<Downstream> auto&& observer) const
  {
      return rpp::make_lambda_observer<Upstream>([observer, fn](const auto& v){ observer.on_next(fn(v)); },
                                                 [observer](const std::exception_ptr& err) { observer.on_error(err); },
                                                 [observer]() { observer.on_completed(); });
  }
}

```
In this case you providing logic how to convert downstream observer to upstream observer. Actually this implementation is equal to previous one, but without handling of observable.

**(Advanced)**
In case of implementing operator via `lift` you can control disposable strategy via `updated_disposable_strategy` parameter. It accepts disposable strategy of upstream and returns disposable strategy for downstream. It needed only for optimization and reducing disposables handling cost and it is purely advanced thing. Not sure if anyone is going to use it by its own for now =)

### Schedulers

Reactive programming becomes even more powerful when observables can operate across multiple threads, rather than being confined to the thread of creation and subscription. This allows for non-blocking, asynchronous operations and provides significant advantages over raw iteration or other pull-based approaches. To enable multithreading in your observables, you can use **Schedulers**.

By default, an **Observable** will perform its work in the thread where the **subscribe** operation occurs. However, you can change this behavior using the **subscribe_on** operator. This operator forces the observable to perform the **subscription** and any subsequent work in the specified **scheduler**. For example:

```cpp
rpp::source::just(1,2,3,4,5)                                  // 1
| rpp::operators::subscribe_on(rpp::schedulers::new_thread{}) // 2
| rpp::operators::subscribe([](auto){});                      // 3
```

<!-- In this case subscribe (`3`) would happen in current thread (where subscribe invoked). But then -->

The **observe_on** operator specifies the **scheduler** that will be used for emission during the processing of further operators after **observe_on**.

A **Scheduler** is responsible for controlling the type of multithreading behavior (or lack thereof) used in the observable. For example, a **scheduler** can utilize a new thread, a thread pool, or a raw queue to manage its processing.

Checkout [API Reference](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/group__rpp.html) to learn more about schedulers in RPP.

See <https://reactivex.io/documentation/scheduler.html> for more details about scheduler concept.

### Disposable

In reactive programming, a **disposable** is an object that represents a resource that needs to be released or disposed of when it is no longer needed. This can include things like file handles, network connections, or any other resource that needs to be cleaned up after use.

The purpose of a disposable is to provide a way to manage resources in a safe and efficient manner. By using disposables, you can ensure that resources are released in a timely manner, preventing memory leaks and other issues that can arise from resource leaks.

In most cases disposables are placed in observers. RPP's observer can use two types of disposables:

1. **Upstream disposable** - This is a disposable that the observable puts into the observer. The upstream disposable keeps some state or callback that should be disposed of when the observer is disposed. This ensures that any resources used by the observable are properly cleaned up when the observer obtains on_error/on_completed or disposed in any other way.

2. **External disposable** - This is a disposable that allows the observer to be disposed of from outside the observer itself. This can be useful in situations where you need to cancel an ongoing operation or release resources before the observable has completed its work.

### Exception guarantee

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
