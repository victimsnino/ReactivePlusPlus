# User Guide

## Introduction

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
- **Distributed in time** - Your application or function doesn't know **when** input (or parts of input) will arrive, but knows **what** to do when it happens:
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

- **Pulling** - You decide **when** you need extra data (e.g., to get something, request, iterate, etc.) and you are simply **checking/requesting** some data. In most cases, this is a blocking operation where you request data and wait for it to be available or periodically check its current status. For example, if you like a blog with non-periodical posts, you may check it daily for new posts manually.

- **Pushing** - You decide **once** that you are interested in a source of data, notify this source somehow (e.g., register, subscribe, etc.), and **react** when new data **becomes available** to you. For example, you might **subscribe** to a blog and **react** to new posts only after receiving a notification on your smartphone, rather than manually checking for updates.

Reactive programming is a powerful way to handle input that is distributed in time. Instead of constantly polling for updates or waiting for input to arrive, reactive programming allows you to register callbacks to be executed when the input becomes available.

See https://reactivex.io/intro.html for more details.

### Core concepts of Reactive Programming

In short, Reactive Programming can be described as follows:
- An **Observer** subscribes to an **Observable**.
- The **Observable** automatically notifies its subscribed **Observers** of any new events/emissions. **Observable** could invoke next **observer**'s method:
  - **on_next(T)** - notifies about new event/emission
  - **on_error(std::exception_ptr)** - notified about error during work. It is termination event (no more calls from this observable should be expected)
  - **on_completed()**  - notified about successful completion.It is termination event (no more calls from this observable should be expected)
- During subscription, the **Observable** can return a **Disposable** (== subscription), which gives the ability to track and dispose of the subscription.

For example:

```cpp
#include <rpp/rpp.hpp>

#include <iostream>

int main()
{
    rpp::source::create<int>([](const auto& observer)
    {
        while (true)
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

See https://reactivex.io/documentation/observable.html for more details.

In such an way it is not powerful enough, so Reactive Programming provides a list of **operators**.

### Operators

**Operators** are ways to modify the **Observable**'s emissions to adapt values to the **Observer**.

For example, we can create observable which: get chars from console input, do it till ‘0’ char, get only letters and send to observer this letters as UPPER. With operators it is pretty simple to do it in correct way:

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

See https://reactivex.io/documentation/operators.html for more details about operators concept.

### Schedulers

Reactive programming becomes even more powerful when observables can operate across multiple threads, rather than being confined to the thread of creation and subscription. This allows for non-blocking, asynchronous operations and provides significant advantages over raw iteration or other pull-based approaches. To enable multithreading in your observables, you can use **Schedulers**.

By default, an **Observable** will perform its work in the thread where the **subscribe** operation occurs. However, you can change this behavior using the **subscribe_on** operator. This operator forces the observable to perform the **subscription** and any subsequent work in the specified **scheduler**.

The **observe_on** operator specifies the **scheduler** that will be used for emission during the processing of further operators after **observe_on**.

A **Scheduler** is responsible for controlling the type of multithreading behavior (or lack thereof) used in the observable. For example, a **scheduler** can utilize a new thread, a thread pool, or a raw queue to manage its processing.


Checkout [API Reference](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/group__rpp.html) to learn more about schedulers in RPP.

See https://reactivex.io/documentation/scheduler.html for more details about scheduler concept.

### Disposable

In reactive programming, a **disposable** is an object that represents a resource that needs to be released or disposed of when it is no longer needed. This can include things like file handles, network connections, or any other resource that needs to be cleaned up after use.

The purpose of a disposable is to provide a way to manage resources in a safe and efficient manner. By using disposables, you can ensure that resources are released in a timely manner, preventing memory leaks and other issues that can arise from resource leaks.

In most cases disposables are placed in observers. RPP's observer can use two types of disposables:

1. **Upstream disposable** - This is a disposable that the observable puts into the observer. The upstream disposable keeps some state or callback that should be disposed of when the observer is disposed. This ensures that any resources used by the observable are properly cleaned up when the observer obtains on_error/on_completed or disposed in any other way.

2. **External disposable** - This is a disposable that allows the observer to be disposed of from outside the observer itself. This can be useful in situations where you need to cancel an ongoing operation or release resources before the observable has completed its work.

## Advanced

Reactive programming has [Observable Contract](https://reactivex.io/documentation/contract.html). Please, read it.

This contact has next important part:

> Observables must issue notifications to observers serially (not in parallel). They may issue these notifications from different threads, but there must be a formal happens-before relationship between the notifications

RPP follows this contract and especially this part. It means, that:

1. **All** implemented in **RPP operators** are **following this contract**:<br>
    All built-in RPP observables/operators emit emission serially
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
```
enter 1
enter 2
exit 2
exit 1
```
only serially
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