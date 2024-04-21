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
#include <limits>
#include <random>

#include "dataset.hpp"
#include "stab_forest.hpp"

int main(int argc, char* argv[])
{
    if (argc != 4 && argc != 5) {
        return -1;
    }
    else {
        std::size_t max_size = std::numeric_limits<std::size_t>::max();
        std::ifstream input(argv[1]);
        std::ofstream output_1(argv[2]);
        std::ofstream output_2(argv[3]);

        try {
            if (argc == 5) {
                max_size = to_unsigned<std::size_t>(argv[4]);
            }
            
            if (!input) {
                throw std::invalid_argument("could not read input data file");
            }
            if (!output_1 || !output_2) {
                throw std::invalid_argument("could not write output files");
            }
        }
        catch (std::exception& ex) {
            std::cout << "error: " << ex.what() << std::endl;
            return 2;
        }

        auto events = read_events<std::uint32_t>(input);
        std::shuffle(events.begin(), events.end(), std::random_device());

        auto size = std::min<std::size_t>(events.size(), max_size);
        auto half = size / 2;
        auto begin = events.begin();
        auto mid = begin + half;
        auto end = begin + size;

        std::sort(events.begin(), mid, interval<std::uint32_t>::start_end_compare());
        
        for (; begin != mid; ++begin) {
            output_1 << begin->start << ' ' << begin->end << '\n';
        }
        for (; mid != end; ++mid) {
            output_2 << mid->start << ' ' << mid->end << '\n';
        }
    }
}