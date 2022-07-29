# ReactivePlusPlus
[![Unit tests](https://github.com/victimsnino/ReactivePlusPlus/actions/workflows/Tests.yml/badge.svg?branch=main)](https://github.com/victimsnino/ReactivePlusPlus/actions/workflows/Tests.yml) 
[![codecov](https://codecov.io/gh/victimsnino/ReactivePlusPlus/branch/main/graph/badge.svg?token=INEHPRF18E)](https://codecov.io/gh/victimsnino/ReactivePlusPlus) 
[![GitHub](https://img.shields.io/github/license/victimsnino/ReactivePlusPlus)](https://github.com/victimsnino/ReactivePlusPlus/blob/main/LICENSE)
[![Join the chat at https://gitter.im/ReactivePlusPlus/community](https://badges.gitter.im/ReactivePlusPlus/community.svg)](https://gitter.im/ReactivePlusPlus/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) 

[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=victimsnino_ReactivePlusPlus&metric=ncloc)](https://sonarcloud.io/summary/new_code?id=victimsnino_ReactivePlusPlus)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=victimsnino_ReactivePlusPlus&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=victimsnino_ReactivePlusPlus)
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/victimsnino/ReactivePlusPlus)


ReactivePlusPlus is reactive programming library for C++ language inspired by "official implementation" ([RxCpp](https://github.com/ReactiveX/RxCpp)) and original idea ([ReactiveX](https://reactivex.io/))

In short: ReactivePlusPlus is library for building asynchronous event-driven streams of data with help of sequences of primitive operators in the declarative form. For example:
```cpp
rpp::source::from_callable(&::getchar)
   .repeat()
   .take_while([](char v) { return v != '0'; })
   .filter(std::not_fn(&::isdigit))
   .map(&::toupper)
   .subscribe([](char v) { std::cout << v; });
```


Main advantages of ReactivePlusPlus are that it is written in Modern C++ with Performance and Usage in mind. As a result it is fast, readable, easy to use and well-documented.

## What about existing Reactive Extension libraries for C++?

Reactive programming is excelent programming paradigm and approach for creation of multi-threading and real-time programs which reacts on some events. Unfortunately, there is only one stable and fully-implemented library at the moment of creation of ReactivePlusPlus - [RxCpp](https://github.com/ReactiveX/RxCpp). 

[RxCpp](https://github.com/ReactiveX/RxCpp) is great and awesome library and perfect implementation of ReactiveX approach. However RxCpp has some disadvantages:
- It is a bit **"old" library written in C++11** with some parts written in the **pre-C++11 style** (mess of old-style classes and wrappers)
- **Issue** with **template parameters**:  `rxcpp::observable` contains **full chain of operators** as second template parameter... where each operator has a bunch of another template parameters itself. It forces **IDEs** works **slower** while parsing resulting type of observable. Also it forces to generate **heavier binaries and debug symbols and slower build time**.
- It has high perfomance cost due to tremendous amount of usage of heap

Another implementation of RX for c++: [another-rxcpp](https://github.com/CODIANZ/another-rxcpp). It partly solves issues of RxCpp via **eliminating of template parameter**  with help of **type-erasing** and making each callback as `std::function`. As a result issue with templates resvoled, but this approach has disadvantages related to runtime: resulting size of observers/observables becomes greater due to heavy `std::function` object, usage of heap for storing everything causes perfomance issues, implementation is just pretty simple and provides a lot of copies of passed objects.

## Why ReactivePlusPlus?

**ReactivePlusPlus** tries to solve all mentioned issues:
- **ReactivePlusPlus** written in **Modern C++ (C++20)** with concepts which makes code-base a lot more understandable and clean:
   - Concepts provide more clear errors and checks: you will understand that pass something incorrect before compilation in IDE or during compilation with understandable errors instead of _"invalid template class map_invalid_t"_
   - Everywhere while possible used deduction of template arguments, for example, type of values of observable by type of subscriber used in on_subscribe and etc
- **ReactivePlusPlus** keeps balance between performance and type-erasing mechanism: Read about it in  [**"Performance vs Flexibility: Specific vs Dynamic"**](https://victimsnino.github.io/ReactivePlusPlus/docs/html/specific_vs_dynamic.html)
- **ReactivePlusPlus** is fast: every part of code written with perfomance in mind. Starting from tests over amount of copies/move and finishing to Continous Benchmarking. Benchmarks show that RPP faster that RxCPP in most cases: [Continous benchmarking results](https://victimsnino.github.io/ReactivePlusPlus/benchmark)

## Installation and usage

To use ReactivePlusPlus just add it as submodule/place folder and add to project via CMake's `add_subfolder`.

ReactivePlusPlus's CMake has several options to set:

- RPP_BUILD_TESTS - build unit tests (default OFF)
- RPP_BUILD_SAMPLES - build samples of usage of RPP (default OFF)
- RPP_BUILD_SFML_CODE - build RPP code related to SFML or not (default OFF) - requires SFML to be installed

In source files add
```cpp
#include <rpp/rpp.hpp>
```
or include each required part separately in any way
```cpp
#include <rpp/observables/specific_observable.hpp> // include specific class implementation
#include <rpp/observers.hpp>                       // include family of classes/functions
#include <rpp/operators/fwd.h>                     // include forwarding of family of classes/functions
```

**IMPORTANT**: rpp is header-only library, so, in cmake terms it is `INTERFACE` target. As a result, most types of "intellisense" parsers fails to parse this library it `rpp` **is not linked to any other static library or executable**. So, for better developer's experience firstly link it with your target library. If you developing something inside rpp I'm strongly recommend you to enable samples `-DRPP_BUILD_SAMPLES=1` or tests `-DRPP_BUILD_TESTS=1` to have correct intellisense results.

## Useful links
- [Manual and doxygen documentation](https://victimsnino.github.io/ReactivePlusPlus/docs/html/index.html)
- [Continous benchmarking results, comparison of `dynamic` and `specific` and comparison with RxCpp](https://victimsnino.github.io/ReactivePlusPlus/benchmark)
- [Articles](https://github.com/victimsnino/ReactivePlusPlus/blob/main/docs/Articles.md)

## Credits:
ReactivePlusPlus library uses:
- [Catch2](https://github.com/catchorg/Catch2) for unit testing only, you can avoid cloning it if you don't need unit-tests
- [RxCpp](https://github.com/ReactiveX/RxCpp) only for comparison of performance between RPP and RxCpp in CI benchmarks. Used as cmake dependency under option
- [reactivex.io](https://reactivex.io) as source for insipration and definition of entities used in RPP. Some comments used in RPP source code taken from [reactivex.io](https://reactivex.io)
- [rxmarbles python](https://pypi.org/project/rxmarbles/) as generator of marbles graphs in doxygen documentation

