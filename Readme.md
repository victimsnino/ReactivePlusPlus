# ReactivePlusPlus
[![CI v2](https://github.com/victimsnino/ReactivePlusPlus/actions/workflows/ci%20v2.yml/badge.svg?branch=v2)](https://github.com/victimsnino/ReactivePlusPlus/actions/workflows/ci%20v2.yml)
[![codecov](https://codecov.io/gh/victimsnino/ReactivePlusPlus/branch/v2/graph/badge.svg?token=INEHPRF18E)](https://app.codecov.io/gh/victimsnino/ReactivePlusPlus/tree/v2)
[![GitHub](https://img.shields.io/github/license/victimsnino/ReactivePlusPlus)](https://github.com/victimsnino/ReactivePlusPlus/blob/main/LICENSE)
[![Join the chat at https://gitter.im/ReactivePlusPlus/community](https://badges.gitter.im/ReactivePlusPlus/community.svg)](https://gitter.im/ReactivePlusPlus/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Join the chat in Discord: https://discord.gg/KWMR3RNkVz](https://img.shields.io/badge/Discord-Chat!-brightgreen.svg)](https://discord.gg/KWMR3RNkVz)

[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=victimsnino_ReactivePlusPlus&metric=ncloc&branch=v2)](https://sonarcloud.io/summary/new_code?id=victimsnino_ReactivePlusPlus&branch=v2)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=victimsnino_ReactivePlusPlus&metric=sqale_rating&branch=v2)](https://sonarcloud.io/summary/new_code?id=victimsnino_ReactivePlusPlus&branch=v2)
![GitHub commit activity (v2)](https://img.shields.io/github/commit-activity/m/victimsnino/ReactivePlusPlus/v2)

## Usage:

ReactivePlusPlus is reactive programming library for C++ language inspired by "official implementation" ([RxCpp](https://github.com/ReactiveX/RxCpp)) and original idea ([ReactiveX](https://reactivex.io/)) that only depends on standard library and C++20 features.

See the [BUILDING](BUILDING.md) document to know how to build RPP.
If you are going to know more details about developing for RPP check [HACKING](HACKING.md) document.

## Note V2:
Currently I'm working on RPP v2 (v2 branch). It follows "zero-overhead principle" and minimizes overhead during usage of RPP. v2 started from the scratch, so, each operator would be implemented from the scratch. Implementation status can be tracked [there](https://github.com/victimsnino/ReactivePlusPlus/blob/v2/docs/Implementation%20Status.md)

## Implementation status:

In short: ReactivePlusPlus is library for building asynchronous event-driven streams of data with help of sequences of primitive operators in the declarative form.

RPP v2 follows zero-overhead principle and most of the operators are (and will) minimize overhead. How? Due to elimination of heap allocations and avoiding unnecessary things. During implementatuon of v1 i've found a lot of cases where RPP does unnecessary expensive things.

Currently ReactivePlusPlus is still under development but it has a lot of implemented operators for now. List of implemented features can be found in [API Reference](https://victimsnino.github.io/ReactivePlusPlus/docs/html/group__rpp.html) with very detailed documentation for each of them

**Example**:

```cpp
rpp::source::from_callable(&::getchar)
   | rpp::operators::repeat()
   | rpp::operators::take_while([](char v) { return v != '0'; })
   | rpp::operators::filter(std::not_fn(&::isdigit))
   | rpp::operators::map(&::toupper)
   | rpp::operators::subscribe([](char v) { std::cout << v; });
```

## Features:

Main advantages of ReactivePlusPlus are that it is written in Modern C++ with Performance and Usage in mind. v2 written to follow zero-overhead principle As a result it is fast, readable, easy to use and well-documented. And it is proven with [continous benchmarking results of v2 and comparison with RxCpp](https://victimsnino.github.io/ReactivePlusPlus/v2/benchmark)

**NOTE**: ReactivePlusPlus is library for C++20. So, it works only on compilers that supports most C++20 features. List of minimal supported compilers:
- (ubuntu) gcc-10
- (ubuntu) clang-11
- (windows) visual studio 2022
- (macos) Apple Clang 14

# Useful links
- [Why ReactivePlusPlus? What about existing Reactive Extension libraries for C++?](https://victimsnino.github.io/ReactivePlusPlus/docs/html/why_rpp.html)
- [Manual and doxygen documentation](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/index.html)
- [Examples](https://github.com/victimsnino/ReactivePlusPlus/tree/main/src/examples)
- [Continous benchmarking results, comparison of `dynamic` and `specific` and comparison with RxCpp](https://victimsnino.github.io/ReactivePlusPlus/v2/benchmark)
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
- [PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
- [snitch](https://github.com/cschreib/snitch) for unit testing only, fetched automatically in case of `RPP_BUILD_TESTS` enabled
- [RxCpp](https://github.com/ReactiveX/RxCpp) only for comparison of performance between RPP and RxCpp in CI benchmarks. Used as cmake dependency under option
- [reactivex.io](https://reactivex.io) as source for insipration and definition of entities used in RPP. Some comments used in RPP source code taken from [reactivex.io](https://reactivex.io)
- [rxmarbles python](https://pypi.org/project/rxmarbles/) as generator of marbles graphs in doxygen documentation
- [cmake-init](https://github.com/friendlyanon/cmake-init) as generator for most part of initial CMakes
