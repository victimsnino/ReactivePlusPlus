#include <rpp/rpp.hpp>
#include <iostream>

/**
 * \example group_by.cpp
 **/
int main()
{
    //! [group_by]
    rpp::source::just(1, 2, 3, 4, 5, 6, 7, 8)
        | rpp::operators::group_by([](int   v) { return v % 2 == 0; })
        | rpp::operators::subscribe([](auto grouped_observable)
        {
            auto key = grouped_observable.get_key();
            std::cout << "new grouped observable " << key << std::endl;
            grouped_observable.subscribe([key](int val)
            {
                std::cout << "key [" << key << "] Val: " << val << std::endl;
            });
        });
    // Output: new grouped observable 0
    //         key [0] Val: 1
    //         new grouped observable 1
    //         key [1] Val: 2
    //         key [0] Val: 3
    //         key [1] Val: 4
    //         key [0] Val: 5
    //         key [1] Val: 6
    //         key [0] Val: 7
    //         key [1] Val: 8
    //! [group_by]
    //
    //! [group_by selector]
    struct Person
    {
        std::string name;
        int age;
    };
    rpp::source::just(Person{"Kate", 18},
                      Person{"Alex", 25},
                      Person{"Nick", 18},
                      Person{"Jack", 25},
                      Person{"Tom", 30},
                      Person{"Vanda", 18})
        | rpp::operators::group_by([](const Person& v) { return v.age; }, [](const Person& v) { return v.name; })
        | rpp::operators::subscribe([](auto grouped_observable)
        {
            grouped_observable.subscribe([age = grouped_observable.get_key()](const std::string& name)
            {
                std::cout << "Age [" << age << "] Name: " << name << std::endl;
            });
        });

    // Output: Age [18] Name: Kate
    //         Age [25] Name: Alex
    //         Age [18] Name: Nick
    //         Age [25] Name: Jack
    //         Age [30] Name: Tom
    //         Age [18] Name: Vanda
    //! [group_by selector]
    return 0;
}
