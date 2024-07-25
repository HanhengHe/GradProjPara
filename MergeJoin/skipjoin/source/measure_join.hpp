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
#ifndef INCLUDE_MEASURE_JOIN_HPP
#define INCLUDE_MEASURE_JOIN_HPP

#include <chrono>
#include <iterator>
#include <utility>
#include <vector>
#include "temporal_join.hpp"

template<class StabForest>
typename std::chrono::milliseconds::rep
measure_forward_scan(const StabForest& lhs, const StabForest& rhs)
{
    using namespace std::chrono;
    using event = typename StabForest::event;

    std::vector<std::pair<event, event>> output;
    auto output_it = std::back_inserter(output);

    auto start = high_resolution_clock::now();
    forward_scan(lhs, rhs, output_it);
    auto end = high_resolution_clock::now();
    std::cerr << '\t' << output.size();
    return duration_cast<milliseconds>(end - start).count();
}

template<class StabForest, class JumpPolicyL, class JumpPolicyR>
typename std::chrono::milliseconds::rep
measure_forward_skip_join(const StabForest& lhs, const StabForest& rhs,
                  const JumpPolicyL& policy_l, const JumpPolicyR& policy_r)
{
    using namespace std::chrono;
    using event = typename StabForest::event;

    std::vector<std::pair<event, event>> output;
    auto output_it = std::back_inserter(output);

    auto start = high_resolution_clock::now();
    forward_skip_join(lhs, rhs, output_it, policy_l, policy_r);
    auto end = high_resolution_clock::now();
    std::cerr << '\t' << output.size();
    return duration_cast<milliseconds>(end - start).count();
}

template<class StabForest, class JumpPolicyL, class JumpPolicyR>
typename std::chrono::milliseconds::rep
measure_parallel_skip_join(const size_t f, const StabForest& lhs, const StabForest& rhs,
    const JumpPolicyL& policy_l, const JumpPolicyR& policy_r)
{
    using namespace std::chrono;
    using event = typename StabForest::event;

    std::vector<std::pair<event, event>> output;
    auto output_it = std::back_inserter(output);

    auto start = high_resolution_clock::now();
    parallel_join(f, lhs, rhs, output_it, policy_l, policy_r);
    auto end = high_resolution_clock::now();
    std::cerr << '\t' << output.size();
    return duration_cast<milliseconds>(end - start).count();
}

template<class StabForest, class WindowIt, class JumpPolicy>
typename std::chrono::milliseconds::rep
measure_multi_window(const StabForest& sf, const WindowIt first, const WindowIt last, const JumpPolicy& policy)
{
    using namespace std::chrono;
    using event = typename StabForest::event;

    std::vector<event> output;
    auto output_it = std::back_inserter(output);

    auto start = high_resolution_clock::now();
    auto it = sf.stab_forward_search(output_it, policy);
    for (auto wit = first; wit != last; ++wit) {
        it.stab_forward(wit->start);
        while (it  != sf.cend() && it->start <= wit->end) {
            *output_it = *it;
            ++output_it;
            ++it;
        }
    }
    auto end = high_resolution_clock::now();
    std::cerr << '\t' << output.size();
    return duration_cast<milliseconds>(end - start).count();
}

#endif