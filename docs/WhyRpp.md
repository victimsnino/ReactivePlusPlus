# Why ReactivePlusPlus? {#why_rpp}

## What about existing Reactive Extension libraries for C++?

Reactive programming is excelent programming paradigm and approach for creation of multi-threading and real-time programs which reacts on some events. Unfortunately, there is only one stable and fully-implemented library at the moment of creation of ReactivePlusPlus - [RxCpp](https://github.com/ReactiveX/RxCpp). 

[RxCpp](https://github.com/ReactiveX/RxCpp) is great and awesome library and perfect implementation of ReactiveX approach. However RxCpp has some disadvantages:
- It is a bit **"old" library written in C++11** with some parts written in the **pre-C++11 style** (mess of old-style classes and wrappers)
- **Issue** with **template parameters**:  `rxcpp::observable` contains **full chain of operators** as second template parameter... where each operator has a bunch of another template parameters itself. It forces **IDEs** works **slower** while parsing resulting type of observable. Also it forces to generate **heavier binaries and debug symbols and slower build time**.
- It has high perfomance cost due to tremendous amount of usage of heap. 
- Some parts of code written with non-effective logic (e.g. `timeout` operator - rpp version faster in about 17000-25000%...)

Another implementation of RX for c++: [another-rxcpp](https://github.com/CODIANZ/another-rxcpp). It partly solves issues of RxCpp via **eliminating of template parameter**  with help of **type-erasing** and making each callback as `std::function`. As a result issue with templates resvoled, but this approach has disadvantages related to runtime: resulting size of observers/observables becomes greater due to heavy `std::function` object, usage of heap for storing everything causes perfomance issues, implementation is just pretty simple and provides a lot of copies of passed objects.

## Why ReactivePlusPlus?

**ReactivePlusPlus** tries to solve all mentioned issues:
- **ReactivePlusPlus** written in **Modern C++ (C++20)** with concepts which makes code-base a lot more understandable and clean:
   - Concepts provide more clear errors and checks: you will understand that pass something incorrect before compilation in IDE or during compilation with understandable errors instead of _"invalid template class map_invalid_t"_
   - Everywhere while possible used deduction of template arguments, for example, type of values of observable by type of subscriber used in on_subscribe and etc
- **ReactivePlusPlus** keeps balance between performance and type-erasing mechanism: Read about it in  [**"Performance vs Flexibility: Specific vs Dynamic"**](https://victimsnino.github.io/ReactivePlusPlus/docs/html/specific_vs_dynamic.html)
- **ReactivePlusPlus** is fast: every part of code written with perfomance in mind. Starting from tests over amount of copies/move and finishing to Continous Benchmarking. Benchmarks prove that RPP faster than RxCPP in most cases: [Continous benchmarking results](https://victimsnino.github.io/ReactivePlusPlus/benchmark) (check timeout operator comparison ;-))
