#include <Rpp/rpp.h>

#include <iostream>

int main()
{
    //! [from]
    std::vector<int> vals{ 1,2,3 };
    rpp::source::from(vals).subscribe([](int v) {std::cout << v << " "; });
    //! [from]

    //! [from with model]
    std::vector<int> vals{ 1,2,3 };
    rpp::source::from<rpp::memory_model::use_shared>(vals).subscribe([](int v) {std::cout << v << " "; });
    //! [from with model]

    //! [from with scheduler]
    std::vector<int> vals{ 1,2,3 };
    rpp::source::from(vals, rpp::schedulers::new_thread{}).subscribe([](int v) {std::cout << v << " "; });
    //! [from with scheduler]
    return 0;
}
