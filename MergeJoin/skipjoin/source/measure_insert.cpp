/**
 *
 * Copyright (c) 2017 Jelle Hellings.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY JELLE HELLINGS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <cstdint>
#include <chrono>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#include "dataset.hpp"
#include "performance_measure.hpp"
#include "stab_forest.hpp"

template<class Event>
void append(std::vector<Event>& container, const Event event)
{
    container.emplace_back(event);
}

template<class Event, class Compare>
void append(std::multiset<Event, Compare>& container, const Event event, std::false_type)
{
    container.emplace(event);
}

template<class Event, class Compare>
void append(std::multiset<Event, Compare>& container, const Event event, std::true_type)
{
    container.emplace_hint(container.end(), event);
}

template<class Timestamp, template<class> class EventList, class Event>
void append(stab_forest<Timestamp, EventList>& container, const Event event)
{
    container.append_event(event);
}


template<class Container, class InIt, class... Other>
void append_measure(InIt begin, InIt end, Other... other)
{
    using namespace std::chrono;

    Container container;
    auto start_mem = memory_usage();
    auto start_time = high_resolution_clock::now();
    while (begin != end) {
        append(container, *begin, other...);
        ++begin;
    }
    auto end_time = high_resolution_clock::now();
    auto end_mem = memory_usage();
    std::cout << duration_cast<milliseconds>(end_time - start_time).count()
              << '\t' << (end_mem - start_mem);
}

template<class InIt, class SizeType>
void measure(InIt begin, const SizeType n)
{
    using timestamp = std::uint32_t;
    using event = interval<timestamp>;
    using compare = typename event::start_end_compare_t<std::less<>>;

    auto end = begin + n;
    std::cout << n << '\t';
    append_measure<std::vector<event>>(begin, end);
    std::cout << '\t';
    append_measure<std::multiset<event, compare>>(begin, end, std::false_type());
    std::cout << '\t';
    append_measure<std::multiset<event, compare>>(begin, end, std::true_type());
    std::cout << '\t';
    append_measure<stab_forest<timestamp, vector_event_list>>(begin, end);
    std::cout << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        return 1;
    }
    else {
        using timestamp = std::uint32_t;
        using event = interval<timestamp>;

        std::vector<event> data;
        std::size_t increment;
        std::size_t runs;

        try {
            increment = to_unsigned<std::size_t>(argv[2]);
            runs = to_unsigned<std::size_t>(argv[3]);
            std::ifstream data_file(argv[1]);
            if (!data_file) {
                throw std::invalid_argument("could not read data file");
            }
            data = read_events<std::uint32_t>(data_file);
        }
        catch (std::exception& ex) {
            std::cout << "error: " << ex.what() << std::endl;
            return 2;
        }

        std::sort(data.begin(), data.end(), event::start_end_compare());
        auto begin = data.cbegin();
        auto size = data.size();
        std::cout << "size\tvector\tvectormem\tmultiset\tmultisetmem\tmultiset*\tmultiset*mem\tstab-forest\tstab-forestmem\n";
        for (std::size_t i = 0; i < runs; ++i) {
            std::cout << "run: " << i << "\n";
            for (auto n = increment; n < size; n += increment) {
                measure(begin, n);
            }
            if ((size % increment) != 0) {
                measure(begin, size);
            }
        }
    }
}