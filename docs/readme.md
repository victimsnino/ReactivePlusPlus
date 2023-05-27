# User Guide

## What is Reactive Programming?

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

## Core concepts of Reactive Programming

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

## Operators

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

## Schedulers

It would be not powerful enough, if observable have to work in thread of creation and subscription - it is blocking pull-based operation, so, there is no any real advantages over raw iteration or something like this. To make your observables multithreaded you can use **Schedulers**.

By default **Observable** will do its work in thread where **subscribe** happens. **subscribe_on** operator changes this behavior and forces observable to do **subscription** and any further work in specified **scheduler**. 

**observe_on** operator specifies **scheduler** which would be used for emission during passing further operators.

**Scheduler** itself controls some type of multhreaded (or not) behavior. For example, **scheduler** can use new thread, thread pool or raw queue to process it.

Checkout [API Reference](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/group__rpp.html) to learn more about schedulers in RPP.

See https://reactivex.io/documentation/scheduler.html for more details about scheduler concept.

