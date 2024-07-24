#pragma once
#include "skipjoin/source/temporal_join.hpp"
#include "Mergejoin/pmergejoin.h"


template <typename Forest>
using citerator = Forest::const_iterator;


template <typename Forest, typename OutputIterator, typename JumpPolicyL, typename JumpPolicyR>
void parallel_join(std::size_type const f, Forest const& lhs, Forest const& rhs,
	OutputIterator spill_over_x, OutputIterator spill_over_y,
	const JumpPolicyL& policy_l, const JumpPolicyR& policy_r)
{
	recursive_join(f, lhs.cbegin(), lhs.cend(), rhs.cbegin(), 
		rhs.cend(), spill_over_x, spill_over_y, policy_l, policy_r);
};


template <typename Forest, typename OutputIterator, typename JumpPolicyL, typename JumpPolicyR>
void recursive_join(std::size_type const f, 
	citerator<Forest> lit, citerator<Forest> lend,
	citerator<Forest> rlit, citerator<Forest> rend,
	OutputIterator spill_over_x, OutputIterator spill_over_y,
	const JumpPolicyL& policy_l, const JumpPolicyR& policy_r)
{
	if (f = 1) {
		// Do the join of X and Y
		// if there is a spill_over: do a stab in the original dataset
		// include the data in spill_over_x and spill_over_y in the join
		forward_skip_join(lhs, rhs, policy_l, policy_r)
	}
	else {
		auto m = find_median(lhs.subspan(lit, lend), rhs.subspan(rit, rend));

		auto Xmid = lhs.stab_search(m, spill_over_x);
		auto Ymid = rhs.stab_search(m, spill_over_y);

		recursive_join(f - 1, lit, lmid, rhs, rmid);
		recursive_join(f - 1, lmid, lend, rmid, rend, spill_over_x, spill_over_y);
		// that ends after m
	}
};