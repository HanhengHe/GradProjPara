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
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include "dataset.hpp"
#include "stab_forest.hpp"
#include "measure_join.hpp"

template<class StabForest, class EventList>
void run(const StabForest& lhs, const EventList& events)
{
    for (std::size_t i = 0; i <= 10; ++i) {
        EventList part(events.cbegin(), events.cbegin()  + ((events.size() * i) / 10));
        std::sort(part.begin(), part.end(), interval<std::uint32_t>::start_end_compare());
        StabForest rhs;
        for (auto event : part) {
            rhs.append_event(event);
        }

        std::cerr << part.size();
        std::cout << part.size() 
                  << "\t" << measure_forward_scan(lhs, rhs)
                  << "\t" << measure_forward_skip_join(lhs, rhs, stab_forward_list(), stab_forward_list())
                  << "\t" << measure_forward_skip_join(lhs, rhs, stab_forward_check(lhs, 16u), stab_forward_check(rhs, 16u))
                  << std::endl;
        std::cerr << std::endl;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        return 1;
    }
    else {
        using timestamp = std::uint32_t;
        using event = interval<timestamp>;

        std::vector<event> first_data;
        std::vector<event> second_data;
        std::size_t runs;
        try {
            runs = to_unsigned<std::size_t>(argv[3]);
            std::ifstream first_data_file(argv[1]);
            std::ifstream second_data_file(argv[2]);
            if (!first_data_file) {
                throw std::invalid_argument("could not read first data file");
            }
            if (!second_data_file) {
                throw std::invalid_argument("could not read second data file");
            }
            first_data = read_events<timestamp>(first_data_file);
            second_data = read_events<timestamp>(second_data_file);
        }
        catch (std::exception& ex) {
            std::cout << "error: " << ex.what() << std::endl;
            return 2;
        }

        stab_forest<timestamp, vector_event_list> lhs_sf;
        for (auto event : first_data) {
            lhs_sf.append_event(event);
        }

        std::cout << "size\tforward-scan\tskip-join-list\tskip-join-16c\n";
        for (std::size_t i = 0; i < runs; ++i) {
            std::cout << "run: " << i << std::endl;
            run(lhs_sf, second_data);
        }
    }
}