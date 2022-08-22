# Quick Start {#quick_start}

## Brief workflow
For the brief overview of the Reactive pattern read [https://reactivex.io/](https://reactivex.io/). 

In short, creation of programs with help of ReactivePlusPlus split into several parts:

### 1) Define observables
Observables are sources of your future streams. First of all you need to create some observable which emits values. You can select from some [predefined](https://victimsnino.github.io/ReactivePlusPlus/docs/html/group__creational__operators.html) or built your own.

For example, 
```cpp
rpp::source::from_callable(&::getchar)
```
observable which emits one char from `cin` via invoking of provided function once after subscription 

Action inside observable happens ONLY after subscription on this observable and ONLY for provided subscriber/observer. It means, that you can subscribe on the same observable multiple times! But actually each of this subscriber would see its "own" observable - function invoked especially for this one.

### 2) Chain observable

When you have some source of data you need to extend it somehow to make it useful! For example, let's make it infinite instead of emitting one value:
```cpp
rpp::source::from_callable(&::getchar)
   .repeat()
```

Also we can filters out digits, transform rest chars to upper. We need to add

```cpp
...
.filter(std::not_fn(&::isdigit))
.map(&::toupper)
```
How long do we want to obtain values? let's say, till '0' char. 
```cpp
...
.take_while([](char v) { return v != '0'; })
.filter(std::not_fn(&::isdigit))
.map(&::toupper)
```

### 3) Subscription
What we want to do with resulting values? Let's dump it to console. Resulting code looks like:

```cpp
rpp::source::from_callable(&::getchar)
    .repeat()
    .take_while([](char v) { return v != '0'; })
    .filter(std::not_fn(&::isdigit))
    .map(&::toupper)
    .subscribe([](char v) { std::cout << v; });
```
Subscribe function applies any from:
- [none]
- subscription
- `on_next`
- subscription, `on_next`
- `on_next`, `on_error`
- subscription, `on_next`, `on_error`
- `on_next`, `on_completed`
- subscription, `on_next`, `on_completed`
- on_next`, `on_error`  `on_completed`
- subscription, `on_next`, `on_error`  `on_completed`
- observer
- subscription, observer
