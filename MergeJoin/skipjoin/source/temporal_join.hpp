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
#ifndef INCLUDE_TEMPORAL_JOIN_HPP
#define INCLUDE_TEMPORAL_JOIN_HPP

#include <functional>
#include <iterator>
#include <limits>
#include <utility>

namespace temporal_join_details
{
    /**
     * Compute the cross product of the range [first, last) with the value event
     * and write it to the specified output.
     */
    struct join_1
    {
        template <class Event, class OutputIterator>
        static void join(Event lhs, Event rhs, OutputIterator output)
        {
            *output = std::make_pair(lhs, rhs);
        }
    };

    /**
     * Compute the cross product of the value event with the range [first, last)
     * and write it to the specified output.
     */
    struct join_2
    {
        template <class Event, class OutputIterator>
        static void join(Event lhs, Event rhs, OutputIterator output)
        {
            *output = std::make_pair(rhs, lhs);
        }
    };

    /**
     * Helper structure that will join single events with the currently set
     * range in a forward scan fashion. Single events are sent to this structure
     * via push_back operators, allowing this structure to be used in
     * conjunction with std::back_inserter.
     */
    template <class Join, class ConstIterator, class OutputIterator>
    struct stab_result_join
    {
        using const_iterator = ConstIterator;
        using output_iterator = OutputIterator;
        using value_type = typename std::iterator_traits<const_iterator>::value_type;

        /**
         * Construct the helper structure by specifying the end of the range
         * and an output iterator to write join results to.
         */
        stab_result_join(const_iterator end, output_iterator output) : iterator(end), end(end), output(output) {}

        /**
         * Join the provided event with the currently set range.
         */
        void push_back(const value_type &event)
        {
            auto it = iterator;
            while (it != end && it->start <= event.end)
            {
                Join::join(event, *it, output);
                ++it;
            }
        }

        /**
         * Set the start of the range.
         */
        void set_iterator(const const_iterator it)
        {
            iterator = it;
        }

    private:
        /* Current range [iterator, end[. */
        const_iterator iterator;
        const_iterator end;

        /* Output iterator for the join output. */
        output_iterator output;
    };

    /**
     * Return a stab_result_join helper structure with the specified end of the
     * range and the specified output iterator to write join results to.
     */
    template <class Join, class ConstIterator, class OutputIterator>
    stab_result_join<Join, ConstIterator, OutputIterator> make_stab_result_join(ConstIterator end, OutputIterator output)
    {
        return stab_result_join<Join, ConstIterator, OutputIterator>(end, output);
    }
}

/**
 * Standard sweep-based forward-scan join.
 */
template <class List, class OutputIterator>
void forward_scan(const List &lhs, const List &rhs, OutputIterator output)
{
    using namespace temporal_join_details;

    /* Helpers to join events. */
    auto stab_left_rj = make_stab_result_join<join_1>(rhs.cend(), output);
    auto stab_right_rj = make_stab_result_join<join_2>(lhs.cend(), output);

    /* Join while we have not reached the end of both lists. */
    auto lit = lhs.cbegin(), lend = lhs.cend();
    auto rit = rhs.cbegin(), rend = rhs.cend();
    while (lit != lend && rit != rend)
    {
        if (lit->start <= rit->start)
        {
            stab_left_rj.set_iterator(rit);
            stab_left_rj.push_back(*lit);
            ++lit;
        }
        else
        {
            stab_right_rj.set_iterator(lit);
            stab_right_rj.push_back(*rit);
            ++rit;
        }
    }
}

/**
 * Standard sweep-based forward-scan join with skipping.
 */
template <class Forest, class OutputIterator, class JumpPolicyL, class JumpPolicyR>
void forward_skip_join(const Forest &lhs, const Forest &rhs, OutputIterator output,
                       const JumpPolicyL &policy_l, const JumpPolicyR &policy_r)
{
    using namespace temporal_join_details;

    /* Helpers to join events. */
    auto stab_left_rj = make_stab_result_join<join_1>(rhs.cend(), output);
    auto stab_right_rj = make_stab_result_join<join_2>(lhs.cend(), output);

    /* Join while we have not reached the end of both lists. */
    auto lit = lhs.stab_forward_search(std::back_inserter(stab_left_rj), policy_l);
    auto lend = lhs.cend();
    auto rit = rhs.stab_forward_search(std::back_inserter(stab_right_rj), policy_r);
    auto rend = rhs.cend();
    while (lit != lend && rit != rend)
    { // parallel here?
        if (lit->start <= rit->start)
        {
            stab_left_rj.set_iterator(rit.get_iterator());
            if (rit->start <= lit->end)
            {
                stab_left_rj.push_back(*lit);
                ++lit;
            }
            else
            {
                lit.stab_forward(rit->start);
            }
        }
        else
        {
            stab_right_rj.set_iterator(lit.get_iterator());
            if (lit->start <= rit->end)
            {
                stab_right_rj.push_back(*rit);
                ++rit;
            }
            else
            {
                rit.stab_forward(lit->start);
            }
        }
    }
}

#endif