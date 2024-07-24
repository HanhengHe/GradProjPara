#include <vector>
#include <assert.h>
#include <list>
#include <algorithm>
#include <future>
#include <functional>
#include <thread>
#include <span>		// use cpp 20 pls
#include <optional> // cpp 17, so cpp 20 pls
#include <type_traits>
#include <string>
#include "../skipjoin/source/temporal_join.hpp"

// #define __DEBUG


/// <summary>
/// not finished
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="lhs"></param>
/// <param name="rhs"></param>
/// <param name="lidx"></param>
/// <param name="ridx"></param>
/// <returns></returns>
template<typename T>
bool index_compare(std::span<T> const lhs, std::span<T> const rhs,
	std::size_t const lidx, std::size_t const ridx)
{// using max(lhs.size(), rhs.size()) as inval
	if (lidx >= lhs.size()) {
		return ridx >= rhs.size();
	}
	else if (ridx >= rhs.size()) {
		return true;
	}
	else {
		return rhs[lhs] < ridx[ridx];
	}
};


/// <summary>
/// not finished
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="m1"></param>
/// <param name="m2"></param>
/// <returns></returns>
template <typename T>
T find_median(std::span<T> const m1, std::span<T> const m2)
/* JELLE: span is a pair of pointer + size---should not be a reference. */
{
	std::size_t begin = 0;
	std::size_t end = m1.size() + m2.size();
	std::size_t e_val = end / 2;

	while (begin < end)
	{
		/** Jelle: this code seems all correct, but a bit overcomplicated witht he optionals and so. **/
		std::size_t invalue = std::max(m1.size(), m2.size());
		auto mid = (begin + end) / 2;

		std::size_t e_m_1 = invalue;
		if (e_val >= (mid + 1))
			e_m_1 = e_val - mid - 1;

		std::size_t e_m = invalue;
		if (e_val >= mid)
			e_m = e_val - mid;

		if (less_than(m1, m2, mid, e_m_1))
		{
			begin = mid + 1;
		}
		else if (less_than(m2, m1, e_m, mid))
		{
			end = mid;
		}
		else if (less_than(m2, m1, e_m_1, mid) && less_than(m1, m2, mid, e_m))
		{
			return m1[mid];
		}
		else 
		{
			std::cerr << "FindMedian falls into wrong else statement!";
		}
	}

	return FindMedian(m2, m1, cmp);
}


/**
 * Jelle: THis REALLY need more documentation!

 * Seems this function tries to set up a parallel merge step by finding a medium
 * & then starting two threads to do the merge.

 *
 * Note that this code seems to be designed to make threads until we reach lists of 1 value.
 * Is that not tooo much? What if the system only supports n = 13 threads?
 *
 * A better design:
 * HighPerfMerge(list A, list B, number of threads n) -> a function that computes
 *    (n-1) indices in A and (n-1) indices in B
 * such that the n threads will
 *    merge A[0, first-index in A] with B[0, first-index in B],
 *    merge A[first-index in A, second-index in A] with B[first-index in B, second-index in B]
 *          ...
 *    merge A[(n-1)-th-index in A, end-of-A] with B[(n-1)-th-index in B, end-of-B]

 * Note that we have std::merge() for single-thread merges :)
 */

template <typename T, typename Compare>
void HighPref_Merge(std::vector<T> &lhs, std::vector<T> &rhs,
					std::pair<std::size_t, std::size_t> const &l_range,
					std::pair<std::size_t, std::size_t> const &r_range,
					std::vector<T> &res,
					std::size_t idx,
					const Compare &cmp)
{

	/** Jelle: seems .second == 1 has some special meaning. Needs to be documented! */
	if (l_range.second == 1 && r_range.second == 1)
	{
		if (cmp(lhs[l_range.first], rhs[r_range.first]))
		{
			res[idx++] = lhs[l_range.first];
			res[idx++] = rhs[r_range.first];
		}
		else
		{
			res[idx++] = rhs[r_range.first];
			res[idx++] = lhs[l_range.first];
		}
		return;
	}
	else if (l_range.second == 1 && r_range.second == 0)
	{
		res[idx] = lhs[l_range.first];
		return;
	}
	else if (l_range.second == 0 && r_range.second == 1)
	{
		res[idx] = rhs[r_range.first];
		return;
	}

	auto lbegin = std::next(lhs.begin(), l_range.first);
	auto lend = std::next(lhs.begin(), l_range.first + l_range.second);
	auto rbegin = std::next(rhs.begin(), r_range.first);
	auto rend = std::next(rhs.begin(), r_range.first + r_range.second);

	const std::span<T> lspan(lhs);
	const std::span<T> rspan(rhs);

	auto mid = FindMedian(lspan.subspan(l_range.first, l_range.second),
						  rspan.subspan(r_range.first, r_range.second), cmp);

	auto ldist = std::distance(lbegin, std::lower_bound(
										   lbegin, lend, mid, cmp));

	auto rdist = std::distance(rbegin, std::lower_bound(
										   rbegin, rend, mid, cmp));

#ifdef __DEBUG
	HighPref_Merge(lhs, rhs,
				   std::make_pair(l_range.first, ldist),
				   std::make_pair(r_range.first, rdist),
				   res, idx, cmp);

	HighPref_Merge(lhs, rhs,
				   std::make_pair(l_range.first + ldist, l_range.second - ldist),
				   std::make_pair(r_range.first + rdist, r_range.second - rdist),
				   res, idx + ldist + rdist, cmp);
#else
	std::thread lthread(HighPref_Merge<T, Compare>, std::ref(lhs), std::ref(rhs),
						std::make_pair(l_range.first, ldist),
						std::make_pair(r_range.first, rdist),
						std::ref(res), idx, std::ref(cmp));

	std::thread rthread(HighPref_Merge<T, Compare>, std::ref(lhs), std::ref(rhs),
						std::make_pair(l_range.first + ldist, l_range.second - ldist),
						std::make_pair(r_range.first + rdist, r_range.second - rdist),
						std::ref(res), idx + ldist + rdist, std::ref(cmp));

	lthread.join();
	rthread.join();

	/*** Jelle NOTRE: we only need a single extra thread here---You start lthread and and rthread, and then wait for both of them.... That means that the thread of _this function_ is not doing anything in the mean time. --> we can simply do lthread as a thread & then do rthread as a function call. */

#endif // __DEBUG

	return;
}

template <typename T, typename Compare>
std::vector<T> PMerge(std::vector<T> &lhs, std::vector<T> &rhs, std::size_t n_workers, const Compare &cmp)
{
	std::vector<std::thread> workers;
	workers.reserve(n_workers);

	if (lhs.empty())
	{
		return rhs;
	}
	else if (rhs.empty())
	{
		return lhs;
	}

	const auto z_val = lhs.size() + rhs.size();

	// finding value v_0, .., v_n need a traverse
	std::vector<std::size_t> left_pieces, right_pieces;
	left_pieces.emplace_back(0);
	right_pieces.emplace_back(0);

	auto count = 0;
	auto idx = 1;
	auto left_idx = 0;
	auto right_idx = 0;

	while (count < z_val)
	{
		if (left_idx >= lhs.size())
			++right_idx;
		else if (right_idx >= rhs.size())
			++left_idx;
		else
			cmp(lhs[left_idx], rhs[right_idx]) ? ++left_idx : ++right_idx;

		if (count == idx * z_val / n_workers)
		{
			left_pieces.emplace_back(left_idx);
			right_pieces.emplace_back(right_idx);
			idx++;
		}
		count++;
	}

	left_pieces.emplace_back(left_idx);
	right_pieces.emplace_back(right_idx);

	assert(left_pieces.size() == right_pieces.size()); // ?

	std::vector<T> res(z_val);
	for (auto idx = 0; idx < left_pieces.size() - 1; idx++)
	{
#ifdef __DEBUG
		HighPref_Merge(lhs, rhs,
					   std::make_pair(left_pieces[idx], left_pieces[idx + 1] - left_pieces[idx]),
					   std::make_pair(right_pieces[idx], right_pieces[idx + 1] - right_pieces[idx]),
					   res, left_pieces[idx] + right_pieces[idx], cmp);
#else

		/**
		 ** Jelle: SO here we already make threads here, but HighPref_Merge makes even more threads---So how many threads will we end up with? Think this logic can be simplified. */

		workers.emplace_back(std::thread(HighPref_Merge<T, Compare>, std::ref(lhs), std::ref(rhs),
										 std::make_pair(left_pieces[idx], left_pieces[idx + 1] - left_pieces[idx]),
										 std::make_pair(right_pieces[idx], right_pieces[idx + 1] - right_pieces[idx]),
										 std::ref(res), left_pieces[idx] + right_pieces[idx], std::ref(cmp)));
#endif // __DEBUG
	}

	for (auto &thread : workers)
	{
		thread.join();
	}

	return res;
}

template <typename Dataset, typename Compare = std::less<Dataset>>
void PMergeSort(Dataset& dat, std::size_t n_worker, const Compare &cmp = Compare())
{
	assert(n_worker != 0);
	if (dat.empty())
	{
		return;
	}

	std::size_t jobs_each_thread = dat.size() / n_worker;

	// split jobs
	std::list<std::vector<T>> jobs;

	auto beg = dat.begin();
	auto it = dat.begin();

	for (std::size_t count = 0; count < n_worker; count++)
	{ //! double check
		auto tail = count == n_worker - 1 ? dat.end() : std::next(it, jobs_each_thread);
		jobs.emplace_back(it, tail);
		// use high-pref to sort???
		std::sort(jobs.back().begin(), jobs.back().end(), cmp);
		it = tail;
	}

	std::list<std::vector<T>> res;

	while (jobs.size() >= 2)
	{
		/***  Make this section a function,
			then we can simply remove res from this function & replace the std::swap by:
			jobs = function(jobs).
		***/
		for (std::size_t idx = 0; idx < jobs.size(); idx += 2)
		{
			res.emplace_back(PMerge(*jobs.begin(), *std::next(jobs.begin()), n_worker, cmp));
			jobs.erase(jobs.begin(), std::next(jobs.begin(), 2));

			/** Jelle: why are we erasing at the front---that is super costly!. We wont use jobs afterwards anyways. So lets just not remove! */
		}

		if (!jobs.empty())
		{ /* Jelle: we can change this into not erasing Jobs and simply checking: jobs.size() is even. THen: move the last one. */
			res.emplace_back(std::move(jobs.front()));
		}

		std::swap(jobs, res);
		res.clear();
	}

	dat = jobs.front(); //! copy???
	return;
}