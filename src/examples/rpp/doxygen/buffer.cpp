#include <rpp/rpp.hpp>
#include <iostream>

std::ostream & operator<<(std::ostream & out, const std::vector<int>& list)
{
    out << "{";
    auto size = list.size();
    for (size_t i = 0; i < size; ++i)
    {
        out << list.at(i);
        if (i < size - 1)
        {
            out << ",";
        }
    }
    out << "}";

    return out;
}

/**
 * \example buffer.cpp
 **/
int main()
{
    //! [buffer]
    // The stream that uses rvalue overloads for operators
    rpp::source::just(1, 2, 3, 4, 5)
        .buffer(2)
        .subscribe(
            [](const std::vector<int>& v) { std::cout << v << "-"; },
            [](const std::exception_ptr&) {},
            []() { std::cout << "|" << std::endl; });
    // Source: -1-2-3-4-5--|
    // Output: {1,2}-{3,4}-{5}-|
    //! [buffer]
    return 0;
}
