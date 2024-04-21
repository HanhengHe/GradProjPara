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
#ifndef INCLUDE_ALGORITHM_HPP
#define INCLUDE_ALGORITHM_HPP

#include <algorithm>

/**
 * Three-way-merge algorithm that merges three sorted lists into a single sorted
 * list. On equal values (wuth respect to compare) list [first1, last1[ will\
 * precede [first2, last2[ will precede [first3, last3[. The one-past-last
 * position written to in out is returned.
 */
template<class InIt1, class InIt2, class InIt3, class OutIt, class Compare>
OutIt merge_three_way(InIt1 first1, InIt1 last1,
                      InIt2 first2, InIt2 last2,
                      InIt3 first3, InIt2 last3,
                      OutIt out, Compare compare)
{
    while (first1 != last1 && first2 != last2 && first3 != last3) {
        if (compare(*first2, *first1)) {
            if (compare(*first3, *first2)) {
                *out++ = *first3++;
            }
            else {
                *out++ = *first2++;
            }
        }
        else {
            if (compare(*first3, *first1)) {
                *out++ = *first3++;
            }
            else {
                *out++ = *first1++;
            }
        }
    }

    if (first1 == last1) {
        return std::merge(first2, last2, first3, last3, out, compare);
    }
    else if (first2 == last2) {
        return std::merge(first1, last1, first3, last3, out, compare);
    }
    else {
        return std::merge(first1, last1, first2, last2, out, compare);
    }
}

#endif