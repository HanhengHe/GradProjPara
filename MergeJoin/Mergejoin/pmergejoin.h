#include <vector>
#include <list>
#include <algorithm>
#include <future>
#include <functional>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>


template <typename T, typename Compare = std::less<T>>
void PMergeSort(std::vector<T>& arr, std::size_t n_worker, const Compare& cmp)
{
	assert(n_worker != 0);
	if (arr.empty()) {
		return;
	}
	
	boost::asio::io_service io_service;
	boost::asio::thread_pool thread_pool(n_worker);

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
std::vector<T> PMerge(std::vector<T> const& lhs, std::vector<T> const& rhs, std::size_t n_worker, const Compare& cmp)
{
	return {};
}