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
#include <iostream>
#include <vector>

#include "dataset.hpp"
#include "stab_forest.hpp"
#include "measure_join.hpp"
#include "parallelskipjoin.h"

void run_gap_size(std::uint32_t gap_size)
{
    // constexpr std::uint32_t num_events = 1024 * 1024 * 64;
    constexpr std::uint32_t num_events = 1024;
    stab_forest<std::uint32_t, vector_event_list> lhs;
    stab_forest<std::uint32_t, vector_event_list> rhs;

    bool into_lhs = true;
    for (std::uint32_t i = 0, block = 0, base = 0; i < num_events; ++i, ++block) {
        if (block == gap_size) {
            base += gap_size + 1;
            block = 0;
            into_lhs = !into_lhs;
        }
        if (into_lhs) {
            lhs.append_event(base + block, base + gap_size);
        }
        else {
            rhs.append_event(base + block, base + gap_size);
        }
    }

    std::size_t n_threads = 2;
    std::size_t f = 2;

    std::cerr << gap_size;
    std::cout << gap_size
              // << "\t" << measure_forward_scan(lhs, rhs)
              // << "\t" << measure_forward_skip_join(lhs, rhs, stab_forward_list(), stab_forward_list())
              << "\t" << measure_parallel_skip_join(n_threads, f, lhs, rhs, stab_forward_list(), stab_forward_list())
              // << "\t" << measure_forward_skip_join(lhs, rhs, stab_forward_index(), stab_forward_index())
              // << "\t" << measure_forward_skip_join(lhs, rhs, stab_forward_check(lhs, 1u), stab_forward_check(rhs, 1u))
              // << "\t" << measure_forward_skip_join(lhs, rhs, stab_forward_check(lhs, 2u), stab_forward_check(rhs, 2u))
              // << "\t" << measure_forward_skip_join(lhs, rhs, stab_forward_check(lhs, 4u), stab_forward_check(rhs, 4u))
              // << "\t" << measure_forward_skip_join(lhs, rhs, stab_forward_check(lhs, 8u), stab_forward_check(rhs, 8u))
              // << "\t" << measure_forward_skip_join(lhs, rhs, stab_forward_check(lhs, 16u), stab_forward_check(rhs, 16u))
              // << "\t" << measure_forward_skip_join(lhs, rhs, stab_forward_check(lhs, 32u), stab_forward_check(rhs, 32u))
              // << "\t" << measure_forward_skip_join(lhs, rhs, stab_forward_check(lhs, 64u), stab_forward_check(rhs, 64u))
              << std::endl;
    std::cerr << std::endl;
}

void run()
{
    for (std::uint32_t gap_size = 1u; gap_size <= 1024u * 1024u; gap_size *= 2) {
        run_gap_size(gap_size);
    }
}

int main_skip_join(int argc, char* argv[])
{
    if (argc != 2) {
        return 1;
    }
    else {
        std::size_t runs;
        try {
            runs = to_unsigned<std::size_t>(argv[1]);
        }
        catch (std::exception& ex) {
            std::cout << "error: " << ex.what() << std::endl;
            return 2;
        }
        
        std::cout << "gap-size\tforward-scan\tskip-join-list\tskip-join-index\tskip-join-1c\tskip-join-2c\tskip-join-4c\tskip-join-8c\tskip-join-16c\tskip-join-32c\tskip-join-64c\n";

        for (std::size_t i = 0; i < runs; ++i) {
            std::cout << "run: " << i << std::endl;
            run();
        }
        return 0;
    }
}