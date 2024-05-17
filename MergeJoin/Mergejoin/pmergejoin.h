#include <vector>
#include <assert.h>
#include <list>
#include <algorithm>
#include <future>
#include <functional>
#include <thread>
#include <span>  // use cpp 20 pls
#include <optional>  // cpp 17, so cpp 20 pls
#include <type_traits>
#include <string>


// #define __DEBUG


// #include <boost/asio.hpp>
// #include <boost/bind.hpp>
// #include <boost/thread/thread.hpp>


// if T is an object, pls return a static one and maybe disable the copy constructor?
template <typename T>
T MIN()
{
	if (std::is_same<T, int>::value) {
		return INT_MIN;
	} else {
		throw std::logic_error(std::string("Please add support to type ") + std::string(typeid(T).name()));
	}
}


// if T is an object, pls return a static one and maybe disable the copy constructor?// if T is an object, pls return a static one and maybe disable the copy constructor?

template <typename T>
T MAX()
{
	if (std::is_same<T, int>::value) {
		return INT_MAX;
	}
	else {
		throw std::logic_error(std::string("Please add support to type ") + std::string(typeid(T).name()));
	}
}


/// <summary>
/// find the median val in M1 \bigcup M2
/// return median value we dont care about index or where it from so just val
/// </summary>
template <typename T, typename Compare>
T FindMedian(std::span<T> const& m1, std::span<T> const& m2, Compare const& cmp)
{
	auto less_than = [&cmp](std::span<T> const& left, std::span<T> const& right,
						std::optional<std::size_t> const& lidx, std::optional<std::size_t> const& ridx) {
		T lval, rval;

		if (!lidx)
			lval = MIN<T>();
		else if (lidx.value() >= left.size())
			lval = MAX<T>();
		else
			lval = left[lidx.value()];

		if (!ridx)
			rval = MIN<T>();
		else if (ridx.value() >= right.size())
			rval = MAX<T>();
		else
			rval = right[ridx.value()];

		return cmp(lval, rval);
	};

	// case median \in m1
	std::size_t begin = 0;
	std::size_t end = m1.size() + m2.size();
	std::size_t e_val = end / 2;

	while (begin < end) {
		auto mid = (begin + end) / 2;

		std::optional<std::size_t> e_m_1{ std::nullopt };
		if (e_val >= (mid + 1)) e_m_1 = e_val - mid - 1;

		std::optional<std::size_t> e_m{ std::nullopt };
		if (e_val >= mid) e_m = e_val - mid;

		if (less_than(m1, m2, { mid }, e_m_1)) {
			begin = mid + 1;
		} else if (less_than(m2, m1, e_m, { mid })) {
			end = mid;
		} else if (less_than(m2, m1, e_m_1, { mid }) && less_than(m1, m2, { mid }, e_m)) {
			return m1[mid];
		} else {
			throw std::runtime_error("FindMedian falls into wrong else statement!");
		}
	}

	// case median \in m2
	begin = 0;
	end = m1.size() + m2.size();
	e_val = end / 2;

	while (begin < end) {
		auto mid = (begin + end) / 2;

		std::optional<std::size_t> e_m_1{ std::nullopt };
		if (e_val >= (mid + 1)) e_m_1 = e_val - mid - 1;

		std::optional<std::size_t> e_m{ std::nullopt };
		if (e_val >= mid) e_m = e_val - mid;

		if (less_than(m2, m1, { mid }, e_m_1)) {
			begin = mid + 1;
		}
		else if (less_than(m1, m2, e_m, { mid })) {
			end = mid;
		}
		else if (less_than(m1, m2, e_m_1, { mid }) && less_than(m2, m1, { mid }, e_m)) {
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
#endif // __DEBUG

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

		if (count == idx * z_val / n_workers) {
			left_pieces.emplace_back(left_idx);
			right_pieces.emplace_back(right_idx);
			idx++;
		}
		count++;
	}

	left_pieces.emplace_back(left_idx);
	right_pieces.emplace_back(right_idx);

	assert(left_pieces.size() == right_pieces.size());  // ?

	std::vector<T> res(z_val);
	for (auto idx = 0; idx < left_pieces.size() - 1; idx++) {
#ifdef __DEBUG
		HighPref_Merge(lhs, rhs,
			std::make_pair(left_pieces[idx], left_pieces[idx + 1] - left_pieces[idx]),
			std::make_pair(right_pieces[idx], right_pieces[idx + 1] - right_pieces[idx]),
			res, left_pieces[idx] + right_pieces[idx], cmp);
#else
		workers.emplace_back(std::thread(HighPref_Merge<T, Compare>, std::ref(lhs), std::ref(rhs),
			std::make_pair(left_pieces[idx], left_pieces[idx + 1] - left_pieces[idx]),
			std::make_pair(right_pieces[idx], right_pieces[idx + 1] - right_pieces[idx]),
			std::ref(res), left_pieces[idx] + right_pieces[idx], std::ref(cmp)));
#endif // __DEBUG
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

	auto beg = arr.begin();
	auto it = arr.begin();

	for (std::size_t count = 0; count < n_worker; count++) {  //! double check
		auto tail = count == n_worker - 1 ? arr.end(): std::next(it, jobs_each_thread);
		jobs.emplace_back(it, tail);
		// use high-pref to sort???
		std::sort(jobs.back().begin(), jobs.back().end(), cmp);
		it = tail;
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