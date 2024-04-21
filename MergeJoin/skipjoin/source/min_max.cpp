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

#include "dataset.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        return -1;
    }
    else {
        std::ifstream input(argv[1]);

        if (!input) {
            std::cout << "error: could not read input data file" << std::endl;
            return 2;
        }

        using timestamp = std::uint32_t;
        using event_type = interval<timestamp>;
        auto events = read_events<timestamp>(input);
        
        timestamp min = std::numeric_limits<timestamp>::max();
        timestamp max = std::numeric_limits<timestamp>::min();
        
        event_type min_event;
        event_type max_event;
        
        for (auto event : events) {
            auto diff = event.end - event.start;
            if (diff < min) {
                min_event = event;
                min = diff;
            }
            if (max < diff) {
                max_event = event;
                max = diff;
            }
        }
        
        std::cout << "min: " << min << " [" << min_event.start << ", " << min_event.end << "]" << std::endl
                  << "max: " << max << " [" << max_event.start << ", " << max_event.end << "]" << std::endl;
    }
}