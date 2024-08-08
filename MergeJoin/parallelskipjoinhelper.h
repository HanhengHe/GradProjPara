#pragma once
#include <thread>
#include "ctpl.h"


template <typename OutputIterator, typename EventType>
class ParallelOutputHelper
{
public:
	ParallelOutputHelper() {};

	ParallelOutputHelper(ParallelOutputHelper&&) = delete;
	ParallelOutputHelper(ParallelOutputHelper const& other) = delete;

	void merge_output(std::vector<std::pair<EventType, EventType>>& output)
	{
		for (const auto& output_list : _outputs) {
			output.insert(output.end(), output_list.cbegin(), output_list.cend());
		}
	}

	OutputIterator get_iterator()
	{
		_outputs.emplace_back();
		return std::back_inserter(_outputs.back());
	}

private:
	std::list<std::vector<std::pair<EventType, EventType>>> _outputs;
};


class JoinTaskHandler {
public:
	virtual void append_task(std::function<void(int)>&&) = 0;
	virtual void join() = 0;
};


class ThreadPoolHandler : public JoinTaskHandler
{
public:
	ThreadPoolHandler(std::size_t num_threads)
		: _p(ctpl::thread_pool((int)num_threads)) {}

	ThreadPoolHandler(const ThreadPoolHandler&) = delete;
	ThreadPoolHandler(ThreadPoolHandler&&) = delete;

	void append_task(std::function<void(int)>&& func) override
	{
#ifdef _DEBUG_NO_PARALLEL
		func(0);
#else
		_p.push(std::forward<std::function<void(int)>>(func));
#endif
	}


	void join() {
		_p.stop(true);
	}

private:
	ctpl::thread_pool _p;
};


class ThreadsHandler : public JoinTaskHandler
{
public:
	ThreadsHandler() {}
	ThreadsHandler(const ThreadsHandler&) = delete;
	ThreadsHandler(ThreadsHandler&&) = delete;


	void append_task(std::function<void(int)>&& func) override
	{
#ifdef _DEBUG_NO_PARALLEL
		func(0);
#else
		_threads.emplace_back(std::forward<std::function<void(int)>>(func), _threads_count++);
#endif
	}


	void join() {
		for (auto& t : _threads) {
			t.join();
		}
	}

private:
	int _threads_count = 0;
	std::vector<std::thread> _threads;
};