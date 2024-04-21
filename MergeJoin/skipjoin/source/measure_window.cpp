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
#include <fstream>
#include <iostream>
#include <vector>

#include "dataset.hpp"
#include "stab_forest.hpp"
#include "measure_join.hpp"

template<class StabForest, class PeriodList>
void run(const StabForest& flights, const PeriodList& periods)
{
    for (std::size_t i = 0; i <= periods.size(); i += (periods.size() / 10)) {
        StabForest periods_sf;
        for (std::size_t j = 0; j < i; ++j) {
            periods_sf.append_event(periods[j]);
        }

        std::cerr << i;
        std::cout << i 
                  << "\t" << measure_forward_scan(flights, periods_sf)
                  << "\t" << measure_forward_skip_join(flights, periods_sf, stab_forward_list(), stab_forward_list())
                  << "\t" << measure_forward_skip_join(flights, periods_sf, stab_forward_check(flights, 16u), stab_forward_check(periods_sf, 16u))
                  << "\t" << measure_multi_window(flights, periods.cbegin(), periods.cbegin() + i, stab_forward_check(flights, 16u))
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

        std::vector<event> flights;
        std::vector<event> periods;
        std::size_t runs;
        try {
            runs = to_unsigned<std::size_t>(argv[3]);
            std::ifstream flight_data_file(argv[1]);
            std::ifstream period_data_file(argv[2]);
            if (!flight_data_file) {
                throw std::invalid_argument("could not read flight data file");
            }
            if (!period_data_file) {
                throw std::invalid_argument("could not read period data file");
            }
            flights = read_events<timestamp>(flight_data_file);
            periods = read_events<timestamp>(period_data_file);
        }
        catch (std::exception& ex) {
            std::cout << "error: " << ex.what() << std::endl;
            return 2;
        }

        stab_forest<timestamp, vector_event_list> flight_sf;
        for (auto event : flights) {
            flight_sf.append_event(event);
        }

        std::cout << "numperiods\tforward_scan\tskip-join-list\tskip-join-16c\tmulti-window\n";
        for (std::size_t i = 0; i < runs; ++i) {
            std::cout << "run: " << i << std::endl;
            run(flight_sf, periods);
        }
    }
}