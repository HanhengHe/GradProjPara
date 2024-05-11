#include <vector>
#include <list>
#include <algorithm>
#include <future>
#include <functional>
#include <thread>

// kinda new stuff
#include <span>

// #include <boost/asio.hpp>
// #include <boost/bind.hpp>
// #include <boost/thread/thread.hpp>


template <typename T, typename Compare = std::less<T>>
void PMergeSort(std::vector<T>& arr, std::size_t n_worker, const Compare& cmp)
{
	assert(n_worker != 0);
	if (arr.empty()) {
		return;
	}
	
	// boost::asio::io_service io_service;
	// boost::asio::thread_pool thread_pool(n_worker);

	auto jobs_each_thread = arr.size() / n_worker;

	// split jobs
	std::list<std::vector<T>> jobs;

	auto it = arr.begin();
	auto end = arr.end();
	for (std::size_t count = 0; count < n_worker; count++) {  //! double check
		auto tail = end - it <= jobs_each_thread ? it + jobs_each_thread : end;
		jobs.emplace_back(it, tail);
		std::sort(jobs.back().begin(), jobs.back().end(), cmp);
		it += jobs_each_thread;
	}

	std::vector<std::vector<T>> res;
	while (jobs.size() >= 2) {
		for (std::size_t idx = 0; idx < jobs.size(); idx += 2) {
			res.emplace_back(PMerge(*jobs.begin(), *std::next(jobs.begin()), n_worker, cmp));
			jobs.erase(jobs.begin(), jobs.begin() + 2);
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


/// <summary>
/// find the median val in M1 \bigcup M2
/// return median value we dont care about index or where it from so just val
/// </summary>
template <typename T, typename Compare>
T FindMedian(std::vector<T> const& m1, std::vector<T> const& m2)
{
	auto less_than = [](std::vector<T> const& left, std::vector<T> const& right, 
						std::size_t const& lidx, std::size_t const& ridx) {
		if (lidx >= left.size() && ridx >= right.size())
			return false;
		if (lidx >= left.size())
			return false;
		if (ridx >= right.size())
			return true;

		return left[lidx] < right[ridx];
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
	std::size_t begin = 0;
	std::size_t end = m1.size() + m2.size();

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
std::vector<T> PMerge(std::vector<T> const& lhs, std::vector<T> const& rhs, std::size_t n_workers, const Compare& cmp)
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
			++right_idx
		else if (right_idx >= rhs.size())
			++left_idx
		else
			lhs[left_idx] < rhs[right_idx] ? ++left_idx : ++right_idx;

		if (count == idx * z_val / n_workers) {
				left_pieces.emplace_back(left_idx);
				right_pieces.emplace_back(right_idx);
		}
		count++;
	}

	assert(left_pieces.size() == right_pieces.size());  // ?

	std::vector<T> res;
	for (auto idx = 0; idx < left_pieces.size() - 1; idx++) {
		workers.emplace_back(HighPref_Merge, lhs, rhs,
			std::make_pair(left_pieces[idx], left_pieces[idx + 1]),
			std::make_pair(right_pieces[idx], right_pieces[idx + 1]), 
			res, left_pieces[idx + 1] + right_pieces[idx + 1], cmp);
	}

	for (auto& thread : workers) {
		thread.join();
	}

	return res;
}


template <typename T, typename Compare>
void HighPref_Merge(std::vector<T> const& lhs, std::vector<T> const& rhs, 
	std::pair<std::size_t, std::size_t> const& l_range,
	std::pair<std::size_t, std::size_t> const& r_range, 
	std::vector<T>& res,
	std::size_t idx,
	const Compare& cmp)
{
	auto lidx = l_range.first;
	auto ridx = r_range.first;

	while (lidx < l_range.second && ridx < r_range.second) {
		if (lidx == l_range.second) {
			res[idx++] = rhs[ridx++];
		}
		else if (ridx == l_range.second) {
			res[idx++] = lhs[lidx++];
		}
		else {
			if (lhs[lidx] < rhs[ridx]) {
				res[idx++] = lhs[lidx++];
			}
			else {
				res[idx++] = rhs[ridx++];
			}
		}
	}

	return;
}