# ReactivePlusPlus
[![CI](https://github.com/victimsnino/ReactivePlusPlus/actions/workflows/ci.yml/badge.svg)](https://github.com/victimsnino/ReactivePlusPlus/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/victimsnino/ReactivePlusPlus/branch/main/graph/badge.svg?token=INEHPRF18E)](https://codecov.io/gh/victimsnino/ReactivePlusPlus) 
[![GitHub](https://img.shields.io/github/license/victimsnino/ReactivePlusPlus)](https://github.com/victimsnino/ReactivePlusPlus/blob/main/LICENSE)
[![Join the chat at https://gitter.im/ReactivePlusPlus/community](https://badges.gitter.im/ReactivePlusPlus/community.svg)](https://gitter.im/ReactivePlusPlus/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) 
[![Join the chat in Discord: https://discord.gg/KWMR3RNkVz](https://img.shields.io/badge/Discord-Chat!-brightgreen.svg)](https://discord.gg/KWMR3RNkVz)

[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=victimsnino_ReactivePlusPlus&metric=ncloc)](https://sonarcloud.io/summary/new_code?id=victimsnino_ReactivePlusPlus)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=victimsnino_ReactivePlusPlus&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=victimsnino_ReactivePlusPlus)
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/victimsnino/ReactivePlusPlus)

## Usage:

ReactivePlusPlus is reactive programming library for C++ language inspired by "official implementation" ([RxCpp](https://github.com/ReactiveX/RxCpp)) and original idea ([ReactiveX](https://reactivex.io/)) that only depends on standard library and C++20 features.

See the [BUILDING](BUILDING.md) document to know how to build RPP.
If you are going to know more details about developing for RPP check [HACKING](HACKING.md) document.

## Note:
Curretnly I'm working on RPP v2 (`v2` branch). It follows "zero-overhead principle" and minimizes overhead during usage of RPP.

## Implementation status:

In short: ReactivePlusPlus is library for building asynchronous event-driven streams of data with help of sequences of primitive operators in the declarative form.

Currently ReactivePlusPlus is still under development but it has a lot of implemented operators for now. List of implemented features can be found in [API Reference](https://victimsnino.github.io/ReactivePlusPlus/docs/html/group__rpp.html) with very detailed documentation for each of them

**Example**:

```cpp
rpp::source::from_callable(&::getchar)
   .repeat()
   .take_while([](char v) { return v != '0'; })
   .filter(std::not_fn(&::isdigit))
   .map(&::toupper)
   .subscribe([](char v) { std::cout << v; });
```

### QT Support

On a par with this ReactivePlusPlus provides native support for QT via RppQt target (extension over original RPP). List of implemented features can be found in [QT API Reference](https://victimsnino.github.io/ReactivePlusPlus/docs/html/group__rppqt.html) with very detailed documentation for each of them.

Currently it provides ability to create observable from QT signal of some QObject and scheduler to schedule emissions to "main" thread.

**Example**:
```cpp
QLabel      label{};
QPushButton button{"CLick me"};

auto amount_of_clicks = rppqt::source::from_signal(button, &QPushButton::pressed) // <-------
                          .scan(size_t{}, [](size_t seed, const auto&) { return seed + 1; })
                          .start_with(size_t{})
                          .publish()
                          .ref_count();

amount_of_clicks
  .observe_on(rpp::schedulers::new_thread{})
  .tap([](const auto&)
      {
          std::cout << "Some long computation...." << std::endl;
          std::this_thread::sleep_for(std::chrono::seconds{1});
      })
  .observe_on(rppqt::schedulers::main_thread_scheduler{}) // <---------------
  .combine_latest([](size_t slow_clicks, size_t fast_clicks)
                  {
                      return QString{"Slow clicks are %1. Fast clicks are %2"}.arg(slow_clicks).arg(fast_clicks);
                  },
                  amount_of_clicks)
  .subscribe([&](const QString& text) { label.setText(text); });
```

## Features:

Main advantages of ReactivePlusPlus are that it is written in Modern C++ with Performance and Usage in mind. As a result it is fast, readable, easy to use and well-documented. And it is proven with [continous benchmarking results and comparison with RxCpp](https://victimsnino.github.io/ReactivePlusPlus/benchmark)

**NOTE**: ReactivePlusPlus is library for C++20. So, it works only on compilers that supports most C++20 features. List of minimal supported compilers:
- (ubuntu) gcc-10
- (ubuntu) clang-11
- (windows) visual studio 2022
- (macos) Apple Clang 14

# Useful links
- [Why ReactivePlusPlus? What about existing Reactive Extension libraries for C++?](https://victimsnino.github.io/ReactivePlusPlus/docs/html/why_rpp.html)
- [Manual and doxygen documentation](https://victimsnino.github.io/ReactivePlusPlus/docs/html/index.html)
- [Examples](https://github.com/victimsnino/ReactivePlusPlus/tree/main/src/examples)
- [Continous benchmarking results, comparison of `dynamic` and `specific` and comparison with RxCpp](https://victimsnino.github.io/ReactivePlusPlus/benchmark)
- [Articles/Turorials/Guides](https://github.com/victimsnino/ReactivePlusPlus/blob/main/docs/Articles.md)
- [CONTRIBUTING](CONTRIBUTING.md) document.

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
- [Catch2](https://github.com/catchorg/Catch2) for unit testing only, fetched (or used via `find_package`) automatically in case of `RPP_BUILD_TESTS` enabled
- [RxCpp](https://github.com/ReactiveX/RxCpp) only for comparison of performance between RPP and RxCpp in CI benchmarks. Used as cmake dependency under option
- [reactivex.io](https://reactivex.io) as source for insipration and definition of entities used in RPP. Some comments used in RPP source code taken from [reactivex.io](https://reactivex.io)
- [rxmarbles python](https://pypi.org/project/rxmarbles/) as generator of marbles graphs in doxygen documentation
- [cmake-init](https://github.com/friendlyanon/cmake-init) as generator for most part of initial CMakes
