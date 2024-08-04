#pragma once

#include <optional>

#include "skipjoin/source/stab_forest.hpp"
#include "skipjoin/source/temporal_join.hpp"
#include "parallelskipjoinhelper.h"

// #define _DEBUG_NO_PARALLEL


/**
 * Standard sweep-based forward-scan join with skipping.
 */
void partial_forward_skip_join(auto lhelper, auto rhelper, auto lit, auto lend, auto rit, auto rend, auto output)
{
	if (lit == lend || rit == rend) {
		return;
	}

	using namespace temporal_join_details;

	auto stab_left_rj = make_stab_result_join<join_1>(rend, output);
	auto stab_right_rj = make_stab_result_join<join_2>(lend, output);

	while (lit != lend && rit != rend)
	{
		if (lit->start <= rit->start)
		{
			stab_left_rj.set_iterator(rit);
			if (rit->start <= lit->end)
			{
				stab_left_rj.push_back(*lit);
				++lit;
			}
			else
			{
				lhelper->stab_forward(rit->start, &lit);
			}
		}
		else
		{
			stab_right_rj.set_iterator(lit);
			if (lit->start <= rit->end)
			{
				stab_right_rj.push_back(*rit);
				++rit;
			}
			else
			{
				rhelper->stab_forward(lit->start, &rit);
			}
		}
	}
}


void spill_over_join(auto lit, auto lend, auto rit, auto rend, auto output)
{// check if rhs is sorted on start time.
	if (lit == lend || rit == rend) {
		return;
	}

	using namespace temporal_join_details;

	auto stab_rj = make_stab_result_join<join_1>(rend, output);
	stab_rj.set_iterator(rit);

	while (lit != lend)
	{
		stab_rj.push_back(*lit);
		++lit;
	}
}


static JoinTaskHandler* join_task_consumer = nullptr;


template<typename ValueType>
ValueType get_val(auto it, std::optional<std::size_t> const& offset, std::size_t const size) {
	if (!offset) {
		return std::numeric_limits<ValueType>::min();
	}
	else if (offset >= size) {
		return std::numeric_limits<ValueType>::max();
	}
	else {
		return std::next(it, *offset)->start;
	}
}


/// <summary>
/// not finished
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="lhs"></param>
/// <param name="rhs"></param>
/// <param name="lidx"></param>
/// <param name="ridx"></param>
/// <returns></returns>
template<typename Iterator>
bool index_compare(Iterator lit, Iterator rit,
	std::optional<std::size_t> const& lidx, 
	std::optional<std::size_t> const& ridx,
	std::size_t const lsize, std::size_t const rsize)
{// using max(lhs.size(), rhs.size()) as inval

	using ValueType = std::iterator_traits<Iterator>::value_type::unsigned_type;
	ValueType a = 1;

	ValueType lval = get_val<ValueType>(lit, lidx, lsize);
	ValueType rval = get_val<ValueType>(rit, ridx, rsize);

	return lval < rval;
};


/// <summary>
/// not finished
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="m1"></param>
/// <param name="m2"></param>
/// <returns></returns>
auto find_median(auto lit, auto lend, auto rit, auto rend, bool exit = false) 
{
	std::size_t m1_size = std::distance(lit, lend);
	std::size_t m2_size = std::distance(rit, rend);

	std::size_t begin = 0;
	std::size_t end = m1_size + m2_size;
	std::size_t e_val = end / 2;

	while (begin < end)
	{
		/** Jelle: this code seems all correct, but a bit overcomplicated witht he optionals and so. **/
		std::size_t invalid = std::max(m1_size, m2_size);
		auto mid = (begin + end) / 2;

		std::optional<std::size_t> e_m_1(std::nullopt);
		if (e_val >= (mid + 1))
			e_m_1 = e_val - mid - 1;

		std::optional<std::size_t> e_m(std::nullopt);
		if (e_val >= mid)
			e_m = e_val - mid;

		if (index_compare(lit, rit, { mid }, e_m_1, m1_size, m2_size)) {
			begin = mid + 1;
		} else if (index_compare(rit, lit, e_m, { mid }, m2_size, m1_size)) {
			end = mid;
		}
		else if (index_compare(rit, lit, e_m_1, { mid }, m2_size, m1_size) && index_compare(lit, rit, { mid }, e_m, m1_size, m2_size)) {
			return std::next(lit, mid)->start;
		} else {
			throw std::runtime_error("FindMedian falls into wrong else statement!");
		}
	}

	if (exit) {
		throw std::runtime_error("FindMedian did't find any result!");
	} return find_median(rit, rend, lit, lend, true);
}


template <typename EventType>
void recursive_join(std::size_t const f, auto lhelper, auto rhelper, auto lit, auto lend, auto rit, auto rend, auto output)
{
	if (lit == lend || rit == rend) {
		return;
	}

	if (f == 1) {
		join_task_consumer->append_task([lhelper, rhelper, lit, lend, rit, rend, output](int /*i*/ ) {
			partial_forward_skip_join(lhelper, rhelper, lit, lend, rit, rend, output);
		});
	} else { 
		using namespace temporal_join_details;

		auto m_val = find_median(lit, lend, rit, rend);

		// llow = [lit, lmid_it); lhigh = [lmid_it, lend)
		std::vector<EventType> l_range_after;  // holds all events in llow that need to join with rhigh
		auto lmid_it = lhelper->stab_search(m_val, std::back_inserter(l_range_after));

		// rlow = [rit, rmid_it); rhigh = [rmid_it, rend)
		std::vector<EventType> r_range_after;  // holds all events in rlow that need to join with lhigh
		auto rmid_it = rhelper->stab_search(m_val, std::back_inserter(r_range_after));

		// join all events in llow that need to join with rhigh
		join_task_consumer->append_task([l_range_after, rmid_it, rend, output](int /*i*/) {
			spill_over_join(l_range_after.cbegin(), l_range_after.cend(), rmid_it, rend, output);
		});
		// join all events in rlow that need to join with lhigh
		join_task_consumer->append_task([r_range_after, lmid_it, lend, output](int /*i*/) {
			spill_over_join(lmid_it, lend, r_range_after.cbegin(), r_range_after.cend(), output);
		});

		//join llow & rlow, lhigh & rhigh
		recursive_join<EventType>(f - 1, lhelper, rhelper, lit, lmid_it, rit, rmid_it, output);
		recursive_join<EventType>(f - 1, lhelper, rhelper, lmid_it, lend, rmid_it, rend, output);
	}
};


template <typename Forest, typename OutputIterator, typename JumpPolicyL, typename JumpPolicyR>
void parallel_join(std::size_t n_threads, std::size_t const f, Forest const& lhs, Forest const& rhs,
	OutputIterator output, const JumpPolicyL& policy_l, const JumpPolicyR& policy_r)
{
	if (join_task_consumer) {
		join_task_consumer->join();
		delete join_task_consumer;
	}

	// join_task_consumer = new ThreadPoolHandler(n_threads);
	join_task_consumer = new ThreadsHandler();

	using namespace temporal_join_details;
	using EventType = typename Forest::event;

	auto stab_left_rj = make_stab_result_join<join_1>(rhs.cend(), output);
	auto stab_right_rj = make_stab_result_join<join_2>(lhs.cend(), output);

	auto lhelper = lhs.stab_forward_search_shared(std::back_inserter(stab_left_rj), policy_l);
	auto rhelper = rhs.stab_forward_search_shared(std::back_inserter(stab_right_rj), policy_r);

	auto lit = (*lhelper).get_iterator();
	auto rit = (*rhelper).get_iterator();
	auto lend = lhs.cend();
	auto rend = rhs.cend();

	recursive_join<EventType>(f, lhelper, rhelper, lit, lend, rit, rend, output);
	
	join_task_consumer->join();
	delete join_task_consumer;
	join_task_consumer = nullptr;
};