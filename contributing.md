# Contributing

Hello dear contributor! Before hand thank you for your contribution to RPP! 

Below you can find some useful info about contribution to RPP project



## Code Style
There short list of code styles used in this project. Please, follow this one during development of the new features for RPP

- private member variables: start from `m_` and written in the `snake_case`. Example: `m_data`
- names of classes/functions: `snake_case`. Example: `my_class`
- template typenames: `CamelCase`. Example: `template<typename Type> struct state{};`
- brackets in fuctions/classes: from new line everytime except of inline functions. Examples:


```cpp
void my_long_function()
{
  int v;
  int b = v+2;
}

# Option 1
int my_short_function() { return 2; }

# Option 2
int my_short_function() 
{
    return 2;
}

```
