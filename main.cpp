#include <iostream>
#include <thread>
#include <algorithm>
#include <sstream>
#include <mutex>
#include <functional>
#include <vector>
#include <memory>
#include <atomic>
#include <deque>

static std::atomic<uint64_t> mtSum(0);

#pragma region SpinLock
struct SpinLock
{
	/*
	TODO: Consider using the PAUSE instruction to avoid blocking other CPU cores sharing the same load-store uint.
	Refer to the "Reducing load-store unit utilization" section of "Correctly implementing a spinlock in C++" by Erik Rigtorp (https://rigtorp.se/spinlock/).
	*/
	void Lock() noexcept
	{
		for (;;) {
			// Optimistically assume the lock is free on the first try
			if (!m_Lock.exchange(true, std::memory_order_acquire)) {
				return;
			}
			// Wait for lock to be released without generating cache misses
			while (m_Lock.load(std::memory_order_relaxed));
		}
	}

	bool TryLock() noexcept {
		// First do a relaxed load to check if lock is free in order to prevent
		// unnecessary cache misses if someone does while(!try_lock())
		return !m_Lock.load(std::memory_order_relaxed) && !m_Lock.exchange(true, std::memory_order_acquire);
	}

	void Unlock() noexcept
	{
		m_Lock.store(false, std::memory_order_release);
	}

private:
	std::atomic<bool> m_Lock = { false };
};

// Test
SpinLock testSpinLock;
uint64_t testValue;

void IncrementTestValue(SpinLock& spinLock, uint64_t& value)
{
	for (int i = 0; i < 100000; ++i)
	{
		spinLock.Lock();
		value++;
		spinLock.Unlock();
	}
}

void TestSpinLock()
{
	std::thread t1([&] { IncrementTestValue(testSpinLock, testValue); });
	std::thread t2([&] { IncrementTestValue(testSpinLock, testValue); });
	std::thread t3([&] { IncrementTestValue(testSpinLock, testValue); });

	t1.join();
	t2.join();
	t3.join();

	std::cout << testValue << std::endl;
}

#pragma endregion SpinLock

struct Task
{	
	void Execute()
	{
		std::cout << "Execute\n";
	}
};

class TaskQueue
{
public:
	inline void PushBack(const Task& task)
	{
		m_Lock.Lock();
		m_Queue.push_back(task);
		m_Lock.Unlock();
	}

	inline bool PopFront(Task& task)
	{
		m_Lock.Lock();
		if (m_Queue.empty())
		{
			m_Lock.Unlock();
			return false;
		}

		task = std::move(m_Queue.front());
		m_Queue.pop_front();

		m_Lock.Unlock();
		return true;
	}

private:
	std::deque<Task> m_Queue; // Might not be the best option here...
	SpinLock m_Lock;
};

TaskQueue taskQueue;

class TaskSystem
{
private:
};

int main()
{
	uint32_t threadCount = std::max(1u, std::thread::hardware_concurrency());

	std::cout << (std::ostringstream{} << "Thread count: " << threadCount << '\n').str();

	for (uint32_t i = 0; i < threadCount; ++i)
	{
	std::thread worker([](uint32_t threadID) {
		Task task;
		while (true)
		{
			while (taskQueue.PopFront(task))
			{
				task.Execute();
			}

			std::cout << (std::ostringstream{} << "No more tasks: " << threadID << '\n').str();
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		
		}
		}, i);

		worker.detach();
	}

	while (true)
	{
		std::cout << (std::ostringstream{} << "Thread MAIN" << '\n').str();
		taskQueue.PushBack(Task());
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	}

	// TestSpinLock();

	/*std::vector<int> numbers;
	for (int i = 0; i < 30000; ++i)
	{
		numbers.push_back(rand());
	}

	// Single threaded
	long stSum = 0;
	for (int i = 0; i < numbers.size(); ++i)
	{
		stSum += numbers[i];
	}
	auto stEnd = std::chrono::high_resolution_clock::now();

	std::cout << "Single Threaded: " << stSum << '\n';

	// Multi threaded
	auto SumNumbers = [](const std::vector<int>& numbers, int startIndex, int endIndex)
	{
		for (int i = startIndex; i <= endIndex; ++i)
		{
			mtSum += numbers[i];
		}
	};

	std::thread t1(SumNumbers, numbers, 0, 9999);
	std::thread t2(SumNumbers, numbers, 10000, 19999);
	std::thread t3(SumNumbers, numbers, 20000, 29999);

	t1.join();
	t2.join();
	t3.join();
	auto mtEnd = std::chrono::high_resolution_clock::now();

	std::cout << "Multi Threaded: " << mtSum << '\n';*/

	/*uint32_t threadCount = std::max(1u, std::thread::hardware_concurrency());

	std::cout << (std::ostringstream{} << "Thread count: " << threadCount << '\n').str();

	for (uint32_t i = 0; i < threadCount; ++i)
	{
		std::thread worker([](uint32_t threadID) {
			while (true)
			{
				std::cout << (std::ostringstream{} << "Thread " << threadID << '\n').str();
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			}
			}, i);

		worker.detach();
	}

	while (true)
	{
		std::cout << (std::ostringstream{} << "Thread MAIN" << '\n').str();
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	}*/
}