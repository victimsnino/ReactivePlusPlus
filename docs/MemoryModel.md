# Memory Model

In ReactivePlusPlus there is new concept unique for this implementation: `memory_model`:

Some of the operators and sources like `rpp::source::just` or `rpp::operators::start_with` accept user's variables for usage. Some of this types can be such an expensive to copy or move and it would be preferable to copy it once to heap, but some other types (like POD) is cheap enough and usage of heap would be overkill. But these variables should be saved inside somehow!

So, RPP provides ability to select strategy "how to deal with such a variables" via `rpp::memory_model` enum.

For example, `rpp::source::just`

```cpp
rpp::source::just(my_custom_variable);
```
by default `just` uses `rpp::memory_model::use_stack` and `my_custom_variable` would be copied and moved everywhere when needed. On the other hand

```cpp
rpp::source::just<rpp::memory_model::use_shared>(my_custom_variable);
```
makes only 1 copy/move to shared_ptr and then uses it instead.

As a a result, users can select preferable way of handling of their types.
