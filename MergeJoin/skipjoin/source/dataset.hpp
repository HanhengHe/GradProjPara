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
#ifndef INCLUDE_DATASET_HPP
#define INCLUDE_DATASET_HPP

#include <istream>
#include <limits>
#include <string>
#include <vector>
#include "interval.hpp"

/** 
 * Translate a string to the specified unsigned type. Throw an invalid_argument
 * if the input is not a positive integer or is to big.
 */
template<class UInt>
UInt to_unsigned(const std::string value)
{
    UInt result = 0;
    for (auto c : value) {
        if (!('0' <= c && c <= '9')) {
            throw std::invalid_argument("input not a positive integer");
        }

        UInt digit = c - '0';
        if (((std::numeric_limits<UInt>::max() - digit) / 10) < result) {
            throw std::invalid_argument("input integer is too big");
        }
        result = result * 10 + digit;
    }
    return result;
}


/**
 * Read an input stream into a list of intervals. Each consecutive pair of
 * strings is interpreted as a (start, end)-pair.
 */ 
template<class UInt, class InputStream>
std::vector<interval<UInt>> read_events(InputStream& in)
{
    bool start = false;
    std::vector<interval<UInt>> data;
    std::string start_s, end_s;

    while (in >> start_s >> end_s) {
        auto start = to_unsigned<UInt>(start_s);
        auto end = to_unsigned<UInt>(end_s);
        data.emplace_back(interval<UInt>{start, end});
    }

    return data;
}

#endif