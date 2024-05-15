#include <vector>
#include <assert.h>
#include <list>
#include <algorithm>
#include <future>
#include <functional>
#include <thread>
#include <span>  // use cpp 20 pls

// #include <boost/asio.hpp>
// #include <boost/bind.hpp>
// #include <boost/thread/thread.hpp>


/// <summary>
/// find the median val in M1 \bigcup M2
/// return median value we dont care about index or where it from so just val
/// </summary>
template <typename T, typename Compare>
T FindMedian(std::span<T> const& m1, std::span<T> const& m2, Compare const& cmp)
{
	auto less_than = [&cmp](std::span<T> const& left, std::span<T> const& right,
						std::size_t const& lidx, std::size_t const& ridx) {
		if (lidx >= left.size() && ridx >= right.size())
			return false;
		if (lidx >= left.size())
			return false;
		if (ridx >= right.size())
			return true;

		return cmp(left[lidx], right[ridx]);
	};

	// case median \in m1
	std::size_t begin = 0;
	std::size_t end = m1.size() + m2.size();
	std::size_t e_val = end / 2;

	while (begin < end) {
		auto mid = (begin + end) / 2;
		if (less_than(m1, m2, mid, e_val - mid - 1)) {
			begin = mid + 1;
		} else if (less_than(m2, m1, e_val - mid, mid)) {
			end = mid;
		} else if (less_than(m2, m1, e_val - mid - 1, mid) && less_than(m1, m2, mid, e_val - mid)) {
			return m1[mid];
		}
	}

	// case median \in m2
	begin = 0;
	end = m1.size() + m2.size();
	e_val = end / 2;

	while (begin < end) {
		auto mid = (begin + end) / 2;
		if (less_than(m2, m1, mid, e_val - mid - 1)) {
			begin = mid + 1;
		}
		else if (less_than(m1, m2, e_val - mid, mid)) {
			end = mid;
		}
		else if (less_than(m1, m2, e_val - mid - 1, mid) && less_than(m2, m1, mid, e_val - mid)) {
			return m2[mid];
		}
	}

	throw std::runtime_error("FindMedian didnt find median in both m1 and m2!");
}


template <typename T, typename Compare>
void HighPref_Merge(std::vector<T>& lhs, std::vector<T>& rhs, 
	std::pair<std::size_t, std::size_t> const& l_range,
	std::pair<std::size_t, std::size_t> const& r_range, 
	std::vector<T>& res,
	std::size_t idx,
	const Compare& cmp)
{
	if (l_range.second == 1 && r_range.second == 1) {
		if (cmp(lhs[l_range.first], rhs[r_range.first])) {
			res[idx++] = lhs[l_range.first];
			res[idx++] = rhs[r_range.first];
		}
		else {
			res[idx++] = rhs[r_range.first];
			res[idx++] = lhs[l_range.first];
		}
		return;
	}
	else if (l_range.second  == 1 && r_range.second == 0) {
		res[idx] = lhs[l_range.first];
		return;
	}
	else if (l_range.second == 0 && r_range.second == 1) {
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

	return;
}


template <typename T, typename Compare>
std::vector<T> PMerge(std::vector<T>& lhs, std::vector<T>& rhs, std::size_t n_workers, const Compare& cmp)
{
	std::vector<std::thread> workers;
	workers.reserve(n_workers);

	if (lhs.empty()) {
		return rhs;
	}
	else if (rhs.empty()) {
		return lhs;
	}

	const auto z_val = lhs.size() + rhs.size();
	auto group_size = z_val / n_workers;

	// finding value v_0, .., v_n need a traverse
	std::vector<std::size_t> left_pieces, right_pieces;

	auto count = 0;
	auto idx = 0;
	auto is_left = lhs.front() < rhs.front();
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

		if (count == idx * z_val / n_workers) {
			left_pieces.emplace_back(left_idx);
			right_pieces.emplace_back(right_idx);
		}
		count++;
	}

	assert(left_pieces.size() == right_pieces.size());  // ?

	std::vector<T> res;
	for (auto idx = 0; idx < left_pieces.size() - 1; idx++) {
		workers.emplace_back(std::thread(HighPref_Merge<T, Compare>, std::ref(lhs), std::ref(rhs),
			std::make_pair(left_pieces[idx], left_pieces[idx + 1] - left_pieces[idx]),
			std::make_pair(right_pieces[idx], right_pieces[idx + 1] - right_pieces[idx]),
			std::ref(res), left_pieces[idx + 1] + right_pieces[idx + 1], std::ref(cmp)));
	}

	for (auto& thread : workers) {
		thread.join();
	}

	return res;
}


template <typename T, typename Compare = std::less<T>>
void PMergeSort(std::vector<T>& arr, std::size_t n_worker, const Compare& cmp = Compare())
{
	assert(n_worker != 0);
	if (arr.empty()) {
		return;
	}

	// boost::asio::io_service io_service;
	// boost::asio::thread_pool thread_pool(n_worker);

	std::size_t jobs_each_thread = arr.size() / n_worker;

	// split jobs
	std::list<std::vector<T>> jobs;

	auto it = arr.begin();
	auto end = arr.end();
	for (std::size_t count = 0; count < n_worker; count++) {  //! double check
		auto tail = (std::size_t)(end - it) <= jobs_each_thread ? it + jobs_each_thread : end;
		jobs.emplace_back(it, tail);
		// use high-pref to sort???
		std::sort(jobs.back().begin(), jobs.back().end(), cmp);
		it += jobs_each_thread;
	}

	std::list<std::vector<T>> res;
	while (jobs.size() >= 2) {
		for (std::size_t idx = 0; idx < jobs.size(); idx += 2) {
			res.emplace_back(PMerge(*jobs.begin(), *std::next(jobs.begin()), n_worker, cmp));
			jobs.erase(jobs.begin(), std::next(jobs.begin(), 2));
		}

		if (!jobs.empty()) {
			res.emplace_back(std::move(jobs.front()));
		}

		std::swap(jobs, res);
		res.clear();
	}

	arr = jobs.front();  //! copy???
	return;
}