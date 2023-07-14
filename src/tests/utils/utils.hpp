//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <vector>

template<typename T>
struct my_container_with_error : std::vector<T>
{
    using std::vector<T>::vector;
    typename std::vector<T>::const_iterator begin() const { throw std::runtime_error{"EXCEPTION ON BEGIN"}; }
};

template<typename T>
struct my_container_with_error_on_increment
{
public:
    my_container_with_error_on_increment(const T& value) : m_value{value} {}

    class iterator
    {
    public:
        iterator(const my_container_with_error_on_increment* container) : m_container{container} {}

        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;

        const value_type& operator*() const { return m_container->m_value; }
        iterator& operator++() { throw std::runtime_error{""}; }
        iterator operator++(int) { throw std::runtime_error{""}; }

        bool operator==(const iterator&) const {return false;};
        bool operator!=(const iterator&) const {return true;};

    private:
        const my_container_with_error_on_increment* m_container;
    };

    iterator begin() const { return {this}; }
    iterator end() const { return {nullptr}; }

private:
    T m_value;
};

template<typename T>
struct infinite_container
{
    struct iterator
    {
        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;

        value_type operator*() const { return 1; }
        iterator& operator++() { return *this; }
        iterator operator++(int) { return *this; }
        friend bool operator== (const iterator&, const iterator&) { return false; };
        friend bool operator!= (const iterator&, const iterator&) { return true; };
    };

    iterator begin() const { return {}; }
    iterator end() const { return {}; }
};