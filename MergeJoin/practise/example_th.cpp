#include <algorithm>
#include <vector>
#include <thread>
#include <iostream>

/*
 * Sort the range @{begin}:@{end}.
 */
constexpr auto sort_thread = [](auto begin, auto end)
{
    std::sort(begin, end);
};

/*
 * Take input @{input}, divide it into @{num_threads} equal-sized pieces, and
 * sort each piece. We return the size of the first $@{num_threads} - 1$ pieces
 * (the last part is the left overs).
 */
template<class T>
std::size_t initial_sort(std::vector<T>& input, unsigned num_threads)
{
    std::vector<std::thread> workers;

    std::size_t slice_size = input.size() / num_threads;
    std::size_t offset = 0;

    /* Start @{num_threads} sorters. */
    for (unsigned i = 0; i < num_threads; ++i) {
        auto begin_offset = offset;
        auto end_offset = std::min(begin_offset + slice_size, input.size());
        sort_thread(input.begin() + begin_offset, input.begin() + end_offset);
        offset += slice_size;
    }

    /* Wait until all sorters are done. */
    for (auto& thread : workers) {
        thread.join();
    }

    return slice_size;
}

template<class T>
void print(const std::vector<T>& v)
{
    std::cout << "[";
    auto it = v.begin();
    if (it != v.end()) {
        std::cout << *it++;
    }
    while (it != v.end()) {
        std::cout << ", " << *it++;
    }
    std::cout << "]";
}

int main(int argc, char* argv[])
{
    std::vector<int> example = { 12, 34, 21, 6, 23, 6,
                                 12, 12, 23, 1, 5, 6,
                                 9, 23, 54, 43, 32, 23 };
    
    std::cout << "before:\t";
    print(example);
    std::cout << std::endl;

    auto ss = initial_sort(example, 4);

    std::cout << "after:\t";
    print(example);
    std::cout << " (slice size: " << ss << ")" << std::endl;
}



