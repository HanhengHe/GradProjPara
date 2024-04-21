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
#ifndef INCLUDE_INTERVAL_HPP
#define INCLUDE_INTERVAL_HPP

#include <functional>
#include <type_traits>

/**
 * An interval [start, end].
 */
template<class UInteger>
struct interval
{
    using unsigned_type = UInteger;
    using interval_type = interval<unsigned_type>;

    static_assert(std::is_unsigned<unsigned_type>::value, "UInteger must be unsigned");

    /* Start and end of the interval. */
    unsigned_type start;
    unsigned_type end;

    /**
     * Return a predicate that compares the start-value of intervals with the
     * predefined value u using the provided comparator.
     */
    template<class Compare>
    static auto start_predicate(const unsigned_type u, Compare compare)
    {
        return [u, compare](const interval_type& i) {
            return compare(i.start, u);
        };
    }

    /**
     * Return a predicate that compares the end-value of intervals with the
     * predefined value u using the provided comparator.
     */
    template<class Compare>
    static auto end_predicate(const unsigned_type u, Compare compare)
    {
        return [u, compare](const interval_type& i) {
            return compare(i.end, u);
        };
    }

    /**
     * Return a start-only compare object that compares interval starts and
     * compares interval starts with timestamps using the provided comparator.
     */
    template<class Compare = std::less<>>
    static auto start_compare(Compare compare = Compare())
    {
        return start_compare_t<Compare>{compare};
    }

    /**
     * Return an end-only compare object that compares interval ends and
     * compares interval ends with timestamps using the provided comparator.
     */
    template<class Compare = std::less<>>
    static auto end_compare(Compare compare = Compare())
    {
        return end_compare_t<Compare>{compare};
    }


    /**
     * Return a lexicographic compare on (start, end)-values of intervals using
     * the provided comparator to compare the individual start and end fields.
     */
    template<class Compare = std::less<>>
    static auto start_end_compare(Compare compare = Compare())
    {
        return start_end_compare_t<Compare>{compare};
    }

    /**
     * Return a lexicographic compare on (end, start)-values of intervals using
     * the provided comparator to compare the individual start and end fields.
     */
    template<class Compare = std::less<>>
    static auto end_start_compare(Compare compare = Compare())
    {
        return end_start_compare_t<Compare>{compare};
    }
    
    
    /**
     * Underlying type to provide the lexicographic (end, start)-values
     * interval comparisons.
     */
    template<class Compare>
    struct start_end_compare_t
    {
        Compare compare;

        bool operator()(const interval_type& lhs, const interval_type& rhs) const
        {
            return compare(lhs.start, rhs.start) ||
                   ((lhs.start == rhs.start) && compare(lhs.end, rhs.end));
        }
    };

    /**
     * Underlying type to provide the lexicographic (end, start)-values
     * interval comparisons.
     */
    template<class Compare>
    struct end_start_compare_t
    {
        Compare compare;

        bool operator()(const interval_type& lhs, const interval_type& rhs) const
        {
            return compare(lhs.end, rhs.end) ||
                   ((lhs.end == rhs.end) && compare(lhs.start, rhs.start));
        }
    };
    
    /**
     * Underlying type to provide the start-only interval comparisons.
     */
    template<class Compare>
    struct start_compare_t
    {
        Compare compare;

        bool operator()(const interval_type& lhs, const interval_type& rhs) const
        {
            return compare(lhs.start, rhs.start);
        }

        bool operator()(const interval_type& i, const unsigned_type u) const
        {
            return compare(i.start, u);
        }

        bool operator()(const unsigned_type u, const interval_type& i) const
        {
            return compare(u, i.start);
        }
    };

    /**
     * Underlying type to provide the end-only interval comparisons.
     */
    template<class Compare>
    struct end_compare_t
    {
        Compare compare;

        bool operator()(const interval_type& lhs, const interval_type& rhs) const
        {
            return compare(lhs.end, rhs.end);
        }

        bool operator()(const interval_type& i, const unsigned_type u) const
        {
            return compare(i.end, u);
        }

        bool operator()(const unsigned_type u, const interval_type& i) const
        {
            return compare(u, i.end);
        }
    };
};

#endif

