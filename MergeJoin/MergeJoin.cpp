// MergeJoin.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "MergeJoin/pmergejoin.h"
#include "skipjoin/source/measure_jump.cpp"


int main()
{
    stab_forest<unsigned> sf;

    interval<unsigned> fst = { 0, 10 };
    interval<unsigned> snd = { 1, 8 };
    interval<unsigned> trd = { 1, 12 };
    interval<unsigned> fth = { 2, 50 };
    interval<unsigned> n5 = { 3, 11 };
    interval<unsigned> n7 = { 12, 13 };
    interval<unsigned> n8 = { 13, 30 };

    sf.append_event(fst);
    sf.append_event(snd);
    sf.append_event(trd);
    sf.append_event(fth);
    sf.append_event(n5);
    sf.append_event(n7);
    sf.append_event(n8);

    std::vector<interval<unsigned>> output;
    auto it = sf.stab_search(12, std::back_inserter(output));

    std::cout << "The stab result is:";
    for (auto v : output) {
        std::cout << " " << "(" << v.start << ", " << v.end << ")";
    }
    std::cout << std::endl;

    std::cout << "The next event is: ";
    auto v = *it;
    std::cout << " " << "(" << v.start << ", " << v.end << ")" << std::endl;

    ///////////////////////////////////////////////////////////////////////////

    stab_forest<std::uint32_t, vector_event_list> lhs;
    stab_forest<std::uint32_t, vector_event_list> rhs;

    lhs.append_event(0, 10);
    lhs.append_event(1, 8);
    lhs.append_event(1, 12);
    lhs.append_event(2, 50);
    lhs.append_event(3, 11);
    lhs.append_event(12, 13);
    lhs.append_event(13, 30);
    rhs.append_event(12, 14);

    measure_parallel_skip_join(2, 2, lhs, rhs, stab_forward_list(), stab_forward_list());
    measure_parallel_skip_join(2, 2, lhs, rhs, stab_forward_index(), stab_forward_index());
    measure_forward_skip_join(lhs, rhs, stab_forward_list(), stab_forward_list());
    measure_forward_skip_join(lhs, rhs, stab_forward_index(), stab_forward_index());

    // run();

    // auto printer = [](auto const& arr) {
    //     for (const auto& ele : arr)
    //         std::cout << ele << "";
    // };

    // // can arr has duplicated elements?
    // std::vector<int> arr{ 2, 3, 9, 5, 1, 4, 8 };
    // std::cout << "was: ";
    // printer(arr);
    // PMergeSort(arr, 2);
    // std::cout << "\nsorted: ";
    // printer(arr);
    return 0;
}
