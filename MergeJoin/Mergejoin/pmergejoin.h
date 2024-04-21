#include <vector>
#include <list>
#include <algorithm>
#include <future>

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


template <typename T, typename Compare>
std::vector<T> PMerge(std::vector<T> const& lhs, std::vector<T> const& rhs, std::size_t n_worker, const Compare& cmp)
{
	return {};
}