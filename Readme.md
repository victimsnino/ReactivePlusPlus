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

Refer to [User Guide](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/md_docs_2readme.html) for better understanding of concepts of Reactive Programming and [API Reference of RPP](https://victimsnino.github.io/ReactivePlusPlus/v2/docs/html/group__rpp.html) itself.

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
