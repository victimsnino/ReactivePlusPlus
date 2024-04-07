# ReactivePlusPlus
[![GitHub](https://img.shields.io/github/license/victimsnino/ReactivePlusPlus)](https://github.com/victimsnino/ReactivePlusPlus/blob/main/LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-green.svg)](https://isocpp.org/std/the-standard )
[![CI v2](https://github.com/victimsnino/ReactivePlusPlus/actions/workflows/ci%20v2.yml/badge.svg?branch=v2)](https://github.com/victimsnino/ReactivePlusPlus/actions/workflows/ci%20v2.yml)
[![Join the chat at https://gitter.im/ReactivePlusPlus/community](https://badges.gitter.im/ReactivePlusPlus/community.svg)](https://gitter.im/ReactivePlusPlus/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Join the chat in Discord: https://discord.gg/KWMR3RNkVz](https://img.shields.io/badge/Discord-Chat!-brightgreen.svg)](https://discord.gg/KWMR3RNkVz)

[![codecov](https://codecov.io/gh/victimsnino/ReactivePlusPlus/branch/v2/graph/badge.svg?token=INEHPRF18E)](https://app.codecov.io/gh/victimsnino/ReactivePlusPlus/tree/v2)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=victimsnino_ReactivePlusPlus&metric=ncloc&branch=v2)](https://sonarcloud.io/summary/new_code?id=victimsnino_ReactivePlusPlus&branch=v2)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=victimsnino_ReactivePlusPlus&metric=sqale_rating&branch=v2)](https://sonarcloud.io/summary/new_code?id=victimsnino_ReactivePlusPlus&branch=v2)
![GitHub commit activity (v2)](https://img.shields.io/github/commit-activity/m/victimsnino/ReactivePlusPlus/v2)

## Usage:

ReactivePlusPlus is reactive programming library for C++20 language inspired by "official implementation" ([RxCpp](https://github.com/ReactiveX/RxCpp)) and original idea ([ReactiveX](https://reactivex.io/)) that only depends on standard library and C++20 features (mostly on [concepts](https://en.cppreference.com/w/cpp/language/constraints)).

See the [BUILDING](BUILDING.md) document to know how to build/install RPP.
If you are going to know more details about developing for RPP check [HACKING](HACKING.md) document.

Try out rpp on [godbolt.org](https://godbolt.org/z/48fh1hcbn)!

## Documentation:

Check [User Guide](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/md_docs_2readme.html) and [API Reference of RPP](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/group__rpp.html).

## Note about V2:
Currently I'm working on RPP v2 (`v2` branch). RPP v2 follows [**"zero-overhead principle"**](https://en.cppreference.com/w/cpp/language/Zero-overhead_principle) and most of the operators are (and will) minimize overhead.

**How?** Due to elimination of heap allocations and avoiding unnecessary things. During implementatuon of `v1` I've found a lot of cases where RPP does unnecessary expensive things. As a result, `v2` does only required things and nothing else.

For example, `v1`'s `create+map+subscribe` spends about `63.7768ns`, while `v2` is about `0.4ns`.

v2 started from the scratch, so, each operator would be re-implemented from the scratch too. Implementation status can be tracked in [#324](https://github.com/victimsnino/ReactivePlusPlus/issues/324)

You still can use previous implementation. It placed in `v1` branch

## Overview:

In short: ReactivePlusPlus is library for building asynchronous event-driven streams of data with help of sequences of primitive operators in the declarative form.

Currently ReactivePlusPlus is still under development but it has a lot of implemented operators for now. List of implemented features can be found in [API Reference](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/group__rpp.html) with very detailed documentation for each of them.

Main advantages of ReactivePlusPlus are that it is written in Modern C++ with Performance and Usage in mind. v2 written to follow zero-overhead principle As a result it is fast, readable, easy to use and well-documented. And it is proven with [continous benchmarking results of v2 and comparison with RxCpp](https://victimsnino.github.io/ReactivePlusPlus/v2/benchmark)

**NOTE**: ReactivePlusPlus is library for C++20. So, it works only on compilers that supports most C++20 features. List of minimal supported compilers:
- (ubuntu) gcc-10
- (ubuntu) clang-11
- (windows) visual studio 2022
- (macos) Apple Clang 14

## Example:

```cpp
rpp::source::from_callable(&::getchar)
   | rpp::operators::repeat()
   | rpp::operators::take_while([](char v) { return v != '0'; })
   | rpp::operators::filter(std::not_fn(&::isdigit))
   | rpp::operators::map(&::toupper)
   | rpp::operators::subscribe([](char v) { std::cout << v; });
```

There we are creating observable which emits value via invoking of `getchar` function, then `repeat`s it infinite amount of time till termination event happes. It emits values while symbol is not equal to `0`, takes only not digits, maps them to upper case and then just prints to console.

Also it supports QT out of box. Checkout [RPPQT reference](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/group__rppqt.html)

## Why do you need it?

Check the [User Guide](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/md_docs_2readme.html) for a detailed overview of the Reactive Programming concept and RPP itself.

In short, RPP can help you build complex pipelines to distribute values over time, connect "some sources of data" without directly connecting them.

Take a look at the example code for  QT. Here, you can see how to connect a button to a label and update it based on the number of clicks.
```cpp
auto button          = new QPushButton("Click me!");
auto clicks_count_label = new QLabel();
QObject::connect(button, &QPushButton::clicked, [&clicks_count_label, count = 0]() mutable {
   clicks_count_label->setText(QString{"Clicked %1 times!"}.arg(++count));
});
```

In this example, the button is directly connected to the label. What if you want to link another widget to the same button?

```cpp
auto button          = new QPushButton("Click me!");
auto clicks_count_label = new QLabel();
auto clicks_duration_label = new QLabel();
QObject::connect(button, &QPushButton::clicked, [&clicks_count_label, count = 0]() mutable {
   clicks_count_label->setText(QString{"Clicked %1 times!"}.arg(++count));
});
QObject::connect(button, &QPushButton::clicked, [&clicks_duration_label, now = std::chrono::high_resolution_clock::now()]() mutable {
   const auto old = std::exchange(now, std::chrono::high_resolution_clock::now());
   clicks_duration_label->setText(QString{"MS since last click %1!"}.arg(std::chrono::duration_cast<std::chrono::milliseconds>(now-old).count()));
});
```
Again directly connected... and it becomes a bit complex.. what if i want to accumulate two buttons at the same time? should i make a separate variable for this case? Build complex state to track it? Ideally it would be nice also to update "MS since last click %1!" at runtime each 1ms... So, looks like each label have to depend on multiple sources of data. It is not a trivial case. In this case it is nice opportunity to try RPP!
```cpp
auto button_1              = new QPushButton("Click me!");
auto button_2              = new QPushButton("Click me!");
auto clicks_count_label    = new QLabel();
auto clicks_duration_label = new QLabel();

const auto clicks_1      = rppqt::source::from_signal(*button_1, &QPushButton::clicked);
const auto clicks_2      = rppqt::source::from_signal(*button_2, &QPushButton::clicked);
const auto merged_clicks = clicks_1 | rpp::operators::merge_with(clicks_2);

const auto total_clicks     = merged_clicks | rpp::operators::scan(0, [](int seed, auto) { return ++seed; });
const auto click_times      = merged_clicks | rpp::operators::map([](auto) { return std::chrono::high_resolution_clock::now(); });
const auto time_since_click = rpp::source::interval(std::chrono::milliseconds{1}, rppqt::schedulers::main_thread_scheduler{})
                              | rpp::operators::with_latest_from([](auto, const auto click_time) { return std::chrono::high_resolution_clock::now() - click_time; }, click_times);

// .....

total_clicks.subscribe([&clicks_count_label](int clicks)
{
   clicks_count_label->setText(QString{"Clicked %1 times in total!"}.arg(clicks));
});

time_since_click.subscribe([&clicks_duration_label](std::chrono::high_resolution_clock::duration ms) {
   clicks_duration_label->setText(QString{"MS since last click %1!"}.arg(std::chrono::duration_cast<std::chrono::milliseconds>(ms).count()));
});
```
Now we have separate observables for separate sources of dynamic data like clicks itself, clicks count and time of clicks. As a result, we can combine them in any way we want. At the same time now observables and actions for events can be separated easily - we have "some observable of some clicks or any counted event" and "some observable of durations". How this observables was obtained - doesn't matter. Also we easily built a much more complex pipeline without any difficulties.

## What about existing Reactive Extension libraries for C++?

Reactive programming is excelent programming paradigm and approach for creation of multi-threading and real-time programs which reacts on some events. Unfortunately, there is only one stable and fully-implemented library at the moment of creation of ReactivePlusPlus - [RxCpp](https://github.com/ReactiveX/RxCpp).

[RxCpp](https://github.com/ReactiveX/RxCpp) is great and awesome library and perfect implementation of ReactiveX approach. However RxCpp has some disadvantages:
- It is a bit **"old" library written in C++11** with some parts written in the **pre-C++11 style** (mess of old-style classes and wrappers)
- **Issue** with **template parameters**:  `rxcpp::observable` contains **full chain of operators** as second template parameter... where each operator has a bunch of another template parameters itself. It forces **IDEs** works **slower** while parsing resulting type of observable. Also it forces to generate **heavier binaries and debug symbols and slower build time**.
- It has high perfomance cost due to tremendous amount of usage of heap.
- Some parts of code written with non-effective logic

Another implementation of RX for c++: [another-rxcpp](https://github.com/CODIANZ/another-rxcpp). It partly solves issues of RxCpp via **eliminating of template parameter**  with help of **type-erasing** and making each callback as `std::function`. As a result issue with templates resvoled, but this approach has disadvantages related to runtime: resulting size of observers/observables becomes greater due to heavy `std::function` object, usage of heap for storing everything causes perfomance issues, implementation is just pretty simple and provides a lot of copies of passed objects.

### Why ReactivePlusPlus?

**ReactivePlusPlus** tries to solve all mentioned issues:
- **ReactivePlusPlus** written in **Modern C++ (C++20)** with concepts which makes code-base a lot more understandable and clean:
   - Concepts provide more clear errors and checks: you will understand that pass something incorrect before compilation in IDE or during compilation with understandable errors instead of _"invalid template class map_invalid_t"_
   - Everywhere while possible used deduction of template arguments, for example, type of values of observable by type of subscriber used in on_subscribe and etc
- **ReactivePlusPlus** keeps balance between performance and type-erasing mechanism
- **ReactivePlusPlus** is fast: every part of code written with perfomance in mind. Starting from tests over amount of copies/move and finishing to Continous Benchmarking. Benchmarks prove that RPP faster than RxCPP in most cases: [Continous benchmarking results and comparison with RxCpp](https://victimsnino.github.io/ReactivePlusPlus/v2/benchmark)


# Useful links
- [User Guide](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/md_docs_2readme.html)
- [API Reference](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/group__rpp.html)
- [Examples](https://github.com/victimsnino/ReactivePlusPlus/tree/v2/src/examples)
- [reactivex.io](https://reactivex.io) describes everything related to reactive programming.
- [rxmarbles](https://rxmarbles.com/) interactive diagrams of observables/operators
- [Continous benchmarking results and comparison with RxCpp](https://victimsnino.github.io/ReactivePlusPlus/v2/benchmark)
- [BUILDING](BUILDING.md)
- [CONTRIBUTING](CONTRIBUTING.md)

# Licensing

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

# Credits:
ReactivePlusPlus library uses:
- [PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
- [snitch](https://github.com/cschreib/snitch) for unit testing only, fetched automatically in case of `RPP_BUILD_TESTS` enabled
- [RxCpp](https://github.com/ReactiveX/RxCpp) only for comparison of performance between RPP and RxCpp in CI benchmarks. Used as cmake dependency under option
- [reactivex.io](https://reactivex.io) as source for insipration and definition of entities used in RPP. Some comments used in RPP source code taken from [reactivex.io](https://reactivex.io)
- [rxmarbles python](https://pypi.org/project/rxmarbles/) as generator of marbles graphs in doxygen documentation
- [cmake-init](https://github.com/friendlyanon/cmake-init) as generator for most part of initial CMakes
