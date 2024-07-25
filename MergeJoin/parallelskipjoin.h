#pragma once
#include "skipjoin/source/stab_forest.hpp"
#include "skipjoin/source/temporal_join.hpp"


/**
 * Standard sweep-based forward-scan join with skipping.
 */
template <typename OutputIterator, typename JumpPolicyL, typename JumpPolicyR>
void partial_forward_skip_join(auto& lhelper, auto& rhelper, auto lit, auto lend, auto rit, auto rend,
	OutputIterator output, const JumpPolicyL& policy_l, const JumpPolicyR& policy_r)
{
	using namespace temporal_join_details;

	auto stab_left_rj = make_stab_result_join<join_1>(rend, output);
	auto stab_right_rj = make_stab_result_join<join_2>(lend, output);

	while (lit != lend && rit != rend)
	{ // parallel here?
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
				lhelper.stab_forward(rit->start);
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
				rhelper.stab_forward(lit->start);
			}
		}
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
bool index_compare(Iterator const& lit, Iterator const& rit,
	std::size_t const lidx, std::size_t const ridx, 
	std::size_t const lsize, std::size_t const rsize)
{// using max(lhs.size(), rhs.size()) as inval
	if (lidx >= lsize) {
		return ridx >= rsize;
	}
	else if (ridx >= rsize) {
		return true;
	}
	else {
		return std::next(lit, lidx) < std::next(rit, ridx);
	}
};


/// <summary>
/// not finished
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="m1"></param>
/// <param name="m2"></param>
/// <returns></returns>
template <typename OutputIterator, typename JumpPolicyL, typename JumpPolicyR>
auto find_median(auto const& lit, auto lend, auto const& rit, auto rend) 
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

		std::size_t e_m_1 = invalid;
		if (e_val >= (mid + 1))
			e_m_1 = e_val - mid - 1;

		std::size_t e_m = invalid;
		if (e_val >= mid)
			e_m = e_val - mid;

		if (index_compare(lit, rit, mid, e_m_1, m1_size, m2_size))
		{
			begin = mid + 1;
		}
		else if (index_compare(rit, lit, e_m, mid, m2_size, m1_size))
		{
			end = mid;
		}
		else if (index_compare(rit, lit, e_m_1, mid, m2_size, m1_size) && index_compare(lit, rit, mid, e_m, m1_size, m2_size))
		{
			return std::next(lit, mid)->start;  // start???
		}
		else
		{
			std::cerr << "FindMedian falls into wrong else statement!";
		}
	}

	return find_median<OutputIterator, JumpPolicyL, JumpPolicyR>(rit, rend, lit, lend);
}


template <typename OutputIterator, typename JumpPolicyL, typename JumpPolicyR>
void recursive_join(std::size_t const f, auto& lhelper, auto& rhelper, auto lit, auto lend, auto rit, auto rend,
	OutputIterator output, const JumpPolicyL& policy_l, const JumpPolicyR& policy_r)
{
	if (f == 1) {
		// Do the join of X and Y
		// if there is a spill_over: do a stab in the original dataset
		// include the data in spill_over_x and spill_over_y in the join
		partial_forward_skip_join<OutputIterator, JumpPolicyL, JumpPolicyR>(lhelper, rhelper, lit, lend, rit, rend, output, policy_l, policy_r);
	}
	else {
		using namespace temporal_join_details;

		auto m = find_median<OutputIterator, JumpPolicyL, JumpPolicyR>(lit, lend, rit, rend);

		auto stab_left_rj = make_stab_result_join<join_1>(rend, output);
		auto stab_right_rj = make_stab_result_join<join_2>(lend, output);

		auto lmid = lhelper.stab_search(m, /*spill_over_x*/std::back_inserter(stab_left_rj));
		auto rmid = rhelper.stab_search(m, /*spill_over_y*/std::back_inserter(stab_right_rj));

		recursive_join<OutputIterator, JumpPolicyL, JumpPolicyR>(f - 1, lhelper, rhelper, lit, lmid, rit, rmid, output, policy_l, policy_r);
		recursive_join<OutputIterator, JumpPolicyL, JumpPolicyR>(f - 1, lhelper, rhelper, lmid, lend, rmid, rend, output, policy_l, policy_r);
		// that ends after m
	}
};


template <typename Forest, typename OutputIterator, typename JumpPolicyL, typename JumpPolicyR>
void parallel_join(std::size_t const f, Forest const& lhs, Forest const& rhs,
	OutputIterator output, const JumpPolicyL& policy_l, const JumpPolicyR& policy_r)
{
	using namespace temporal_join_details;

	auto stab_left_rj = make_stab_result_join<join_1>(rhs.cend(), output);
	auto stab_right_rj = make_stab_result_join<join_2>(lhs.cend(), output);

	auto lhelper = lhs.stab_forward_search(std::back_inserter(stab_left_rj), policy_l);
	auto lend = lhs.cend();
	auto rhelper = rhs.stab_forward_search(std::back_inserter(stab_right_rj), policy_r);
	auto rend = rhs.cend();

	recursive_join(f, lhelper, rhelper, lhelper.get_iterator(), lend, rhelper.get_iterator(), rend, output, policy_l, policy_r);
};