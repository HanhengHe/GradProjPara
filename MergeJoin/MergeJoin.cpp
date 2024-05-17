// MergeJoin.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "MergeJoin/pmergejoin.h"


int main()
{
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

// visual studio so thoughtful <3 ¡ý
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
