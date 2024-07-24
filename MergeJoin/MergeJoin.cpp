// MergeJoin.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "MergeJoin/pmergejoin.h"
#include "skipjoin/source/measure_jump.cpp"


int main()
{
    run();

    auto printer = [](auto const& arr) {
        for (const auto& ele : arr)
            std::cout << ele << "";
    };

    // can arr has duplicated elements?
    std::vector<int> arr{ 2, 3, 9, 5, 1, 4, 8 };
    std::cout << "was: ";
    printer(arr);
    PMergeSort(arr, 2);
    std::cout << "\nsorted: ";
    printer(arr);
}
