#pragma once
#include <thread>
#include "ctpl.h"


template <typename OutputIterator, typename EventType>
class ParallelOutputIterator
{
public:
	ParallelOutputIterator(std::unordered_map<std::thread::id, OutputIterator>* it_map, std::list<std::vector<std::pair<EventType, EventType>>>* outputs, std::mutex* it_lock)
		: _it_map(it_map), _outputs(outputs), _it_lock(it_lock) {}

	ParallelOutputIterator(ParallelOutputIterator&&) = delete;

	ParallelOutputIterator(ParallelOutputIterator const& other)
	{
		if (this == &other) {
			return;
		}
		_it_lock = other._it_lock;
		_it_map = other._it_map;
		_outputs = other._outputs;
	}

	void merge_output(std::vector<std::pair<EventType, EventType>>& output)
	{
		for (const auto& output_list : *_outputs) {
			output.insert(output.end(), output_list.cbegin(), output_list.cend());
		}
	}

	auto& operator*()
	{
		return get_iterator().operator*();
	}

private:
	OutputIterator get_iterator()
	{
		auto thread_id = std::this_thread::get_id();
		auto it = _it_map->find(thread_id);
		if (it == _it_map->cend()) {
			_it_lock->lock();
			_outputs->emplace_back();
			auto output_it = std::back_inserter(_outputs->back());
			_it_map->insert({ thread_id, output_it });
			_it_lock->unlock();
			return output_it;
		}
		else {
			return it->second;
		}
	}

	std::unordered_map<std::thread::id, OutputIterator>* _it_map;
	std::list<std::vector<std::pair<EventType, EventType>>>* _outputs;
	std::mutex* _it_lock;
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