# ReactivePlusPlus [![Unit tests](https://github.com/victimsnino/ReactivePlusPlus/actions/workflows/Tests.yml/badge.svg?branch=main)](https://github.com/victimsnino/ReactivePlusPlus/actions/workflows/Tests.yml)

ReactivePlusPlus is [ReactiveX](https://reactivex.io/) library for C++ language inspired by "official implementation" ([RxCpp](https://github.com/ReactiveX/RxCpp)) 

## Why one more Reactive Extension library for C++?

Reactive programmin is very cool programming paradigm and approach for creation of multi-threading and real-time programms which reacts on some events. Unfortunately, there is only one stable and fully-implemented approach at the moment of creation of this library - [RxCpp](https://github.com/ReactiveX/RxCpp). 

[RxCpp](https://github.com/ReactiveX/RxCpp) is great and awesome library and perfect implementation of ReactiveX approach. However RxCpp has some disadvantages comparing to this one:
- It is a bit "old" library written in C++11. Some parts of this library written in the pre-C++11 style. It means, that some parts of implementation looks like mess of old-style classes and wrappers. 
- RxCpp contains issue with template parameters:  rxcpp::observable contains full chain of operators as second template parameter... where each operator has a lot of parameters itself. It forces IDEs works slower while parsing resulting type of observable. Also it forces to generate heavier binaries and debug symbols and slower build time.

At the middle of creation of ReactivePlusPlus i've found another approach: [another-rxcpp](https://github.com/CODIANZ/another-rxcpp). Looks like it solves issues partly via eliminating of template parameter  with help of type-erasing and making each callback as std::function. But this approach also has disadvantages and cause runtime instead of compile-time. For example, resulting size of observers/observables becomes greater due to heavy std::function, using heap for storing everything causes perfomance issues and etc.

ReactivePlusPlus tries to solve all of this issues:
- ReactivePlusPlus written in Modern C++ (C++20) using all required modern features to make code simpler
- ReactivePlusPlus provides more flexible and predictable way of using types and type-erasing mechanism. Read about this in [Perfomance vs Flexibility]()
- ReactivePlusPlus written with the care about perfomance in mind. 
## Documentation

Repository contains doxygen documentation which generated per commit and placed on [github pages](https://victimsnino.github.io/ReactivePlusPlus/docs/html/index.html)

## Perfomance
Perfomance is really **important**! It is **doubly important** when we speak about **realtime applications and libraries**! **ReactivePlusPlus** targets as a realtime library to process and handle a tremendous volumes of data. 

This repository uses continous benchmarking! Every commit and pull request measured and diff per each benchmark provided. 

History and actual values can be viewed on [github pages](https://victimsnino.github.io/ReactivePlusPlus/benchmark)
