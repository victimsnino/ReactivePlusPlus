# Introduction

## What is Reactive Programming?

**Reactive programming** is a *design paradigm* where you build your application (or library, module, class, function, logic) to **react** to some asynchronous **events**.

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

## Core types