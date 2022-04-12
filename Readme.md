# ReactivePlusPlus
[![Unit tests](https://github.com/victimsnino/ReactivePlusPlus/actions/workflows/Tests.yml/badge.svg?branch=main)](https://github.com/victimsnino/ReactivePlusPlus/actions/workflows/Tests.yml) 
[![codecov](https://codecov.io/gh/victimsnino/ReactivePlusPlus/branch/main/graph/badge.svg?token=INEHPRF18E)](https://codecov.io/gh/victimsnino/ReactivePlusPlus)

ReactivePlusPlus is [ReactiveX](https://reactivex.io/) library for C++ language inspired by "official implementation" ([RxCpp](https://github.com/ReactiveX/RxCpp)) 

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
   - Concepts provide more clear errors and checks
   - Everywhere while possible used deduction of template arguments, for example, type of values of observable by type of subscriber used in on_subscribe and etc
- **ReactivePlusPlus** keeps balance between performance and type-erasing mechanism: Read about it in  [**"Performance vs Flexibility: Specific vs Dynamic"**](./docs/Specific%20vs%20Dynamic.md)
- **ReactivePlusPlus** is fast: every part of code written with perfomance in mind. Starting from tests over amount of copies/move and finishing to Continous Benchmarking

## Example:
```cpp
observable.create
```

## Implementation Status

Current implementation status of different operators and future plans can be found [there](docs/Implementation%20Status.md)

## Documentation

Doxygen documentation generated per each commit can be found [here](https://victimsnino.github.io/ReactivePlusPlus/docs/html/index.html)

Manual documentation can be found in [docs folder](docs/Readme.md).

## Performance
Performance is really **important**! It is **doubly important** when we speak about **realtime applications and libraries**! **ReactivePlusPlus** targets as a realtime library to process and handle a tremendous volumes of data. 

This repository uses continous benchmarking: every commit and pull request measured and diff per each benchmark provided. Graphs over benchmark results can be found [here](https://victimsnino.github.io/ReactivePlusPlus/benchmark)

## Credits:

ReactivePlusPlus library uses:
- [Catch2](https://github.com/catchorg/Catch2) for unit testing only, you can avoid cloning it if you don't need unit-tests
- [RxCpp](https://github.com/ReactiveX/RxCpp) only for comparison of performance between RPP and RxCpp in CI benchmarks. Used as cmake dependency under option
- [reactivex.io](reactivex.io) as source for insipration and definition of entities used in RPP. Some comments used in RPP source code taken from [reactivex.io](reactivex.io)

