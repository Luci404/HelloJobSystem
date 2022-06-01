/*#include <iostream>
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

#include <concepts>
#include <vector>
#include <set>
#include <iostream>


class ITask {};

enum class EAccessPolicy : uint8_t
{
	Read,
	Write,
	ReadWrite
};

template<typename T, EAccessPolicy E>
struct TaskParameterInfo
{
	using Type = T;
	static const EAccessPolicy AccessPolicy = E;
};

template <typename Enumeration>
auto as_integer(Enumeration const value)
-> typename std::underlying_type<Enumeration>::type
{
	return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

template<typename T>
struct Resource
{
	using Type = T;

#ifdef DEBUG
	void NotifyAccessStart(ITask* task, EAccessPolicy VAccessPolicy)
	{
		std::cout << "NotifyAccessStart | Resource: " << reinterpret_cast<void*>(this) << ", Task: " << reinterpret_cast<void*>(task) << ", AccessPolicy: " << as_integer(VAccessPolicy) << '\n';
	}

	void NotifyAccessEnd(ITask* task, EAccessPolicy VAccessPolicy)
	{
		std::cout << "NotifyAccessEnd | Resource: " << reinterpret_cast<void*>(this) << ", Task: " << reinterpret_cast<void*>(task) << ", AccessPolicy: " << as_integer(VAccessPolicy) << '\n';
	}
#endif
};

template<typename>
struct TaskInstantiationTrait : std::false_type {};

template<typename T, EAccessPolicy E>
struct TaskInstantiationTrait<TaskParameterInfo<T, E>> : std::true_type {};

template<typename T>
concept TaskInstantiation = TaskInstantiationTrait<T>::value;

template<typename>
struct ResourceInstantiationTrait : std::false_type {};

template<typename T>
struct ResourceInstantiationTrait<Resource<T>> : std::true_type {};

template<typename T>
concept ResourceInstantiation = ResourceInstantiationTrait<T>::value;

template<typename T, typename... Ts>
struct first_variadic
{
	using type = T;
};

template<typename... Ts>
using first_variadic_t = typename first_variadic<Ts...>::type;

template<TaskInstantiation... Ts>
class Task : public ITask
{
public:
	typedef void(*FunctionPointerType)(Resource<typename Ts::Type>...);

public:
	Task(FunctionPointerType function)
		: m_Function(function)
	{
	}

#ifdef DEBUG
	template<typename... Args>
	static void NotifyAccessStartRecursive(ITask* task) {}

	template<TaskInstantiation T, TaskInstantiation... Ts2>
	static void NotifyAccessStartRecursive(ITask* task, Resource<typename T::Type> VResource, Resource<typename Ts2::Type>... VOtherResources)
	{
		VResource.NotifyAccessStart(task, T::AccessPolicy);

		if constexpr (sizeof...(VOtherResources) > 0)
		{
			NotifyAccessStartRecursive<Ts2...>(task, VOtherResources...);
		}
	}

	template<typename... Args>
	static void NotifyAccessEndRecursive(ITask* task) {}

	template<TaskInstantiation T, TaskInstantiation... Ts2>
	static void NotifyAccessEndRecursive(ITask* task, Resource<typename T::Type> VResource, Resource<typename Ts2::Type>... VOtherResources)
	{
		VResource.NotifyAccessEnd(task, T::AccessPolicy);

		if constexpr (sizeof...(VOtherResources) > 0)
		{
			NotifyAccessEndRecursive<Ts2...>(task, VOtherResources...);
		}
	}
#endif

	void Execute(Resource<typename Ts::Type>... resources)
	{
		std::cout << "Execute (debug)\n";

#ifdef DEBUG
		NotifyAccessStartRecursive<Ts...>(this, resources...);
#endif

		m_Function(resources...);

#ifdef DEBUG
		NotifyAccessEndRecursive<Ts...>(this, resources...);
#endif
	}

private:
	FunctionPointerType m_Function;
};

// Example

void TaskFunction(Resource<uint16_t> a, Resource<uint32_t> b, Resource<uint64_t> c)
{
	std::cout << "TaskFunction\n";
}

int main(int argc, char** argv)
{
	Resource<uint16_t> myResource1;
	Resource<uint32_t> myResource2;
	Resource<uint64_t> myResource3;

	Task<
		TaskParameterInfo<uint16_t, EAccessPolicy::Read>,
		TaskParameterInfo<uint32_t, EAccessPolicy::Write>,
		TaskParameterInfo<uint64_t, EAccessPolicy::ReadWrite>> task(&TaskFunction);

	task.Execute(myResource1, myResource2, myResource3);

	return 1;
}

#pragma region SpinLock
struct SpinLock
{*/
	/*
	TODO: Consider using the PAUSE instruction to avoid blocking other CPU cores sharing the same load-store uint.
	Refer to the "Reducing load-store unit utilization" section of "Correctly implementing a spinlock in C++" by Erik Rigtorp (https://rigtorp.se/spinlock/).
	*/
	/*void Lock() noexcept
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
				std::cout << (std::ostringstream{} << "Found task: " << threadID << '\n').str();
				task.Execute();
			}

		
		}
		}, i);

		worker.detach();
	}

	while (true)
	{
		std::cout << (std::ostringstream{} << "Thread MAIN" << '\n').str();
		taskQueue.PushBack(Task());
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}*/
#include <array>
#include <inttypes.h>
#include <deque>
#include <iostream>
#include <vector>

#include <thread>
#include <algorithm>
#include <sstream>
#include <mutex>
#include <functional>
#include <memory>
#include <atomic>

#define MAXIMUM_TASK_DEPENDENCIES 4

enum class EDependencyType : uint8_t { Precede, Succeeds, Conditional };

class IExecutionNode
{
public:
	virtual void Execute() const = 0;
};

struct ExecutionDependency
{
	ExecutionDependency(const EDependencyType dependencyType, const IExecutionNode* dependent, const IExecutionNode* dependency)
		: DependencyType(dependencyType), Dependent(dependent), Dependency(dependency) {}

	const EDependencyType DependencyType;
	const IExecutionNode* Dependent;
	const IExecutionNode* Dependency;
};

class ExecutionTask : public IExecutionNode
{
public:
	ExecutionTask(void(*function)())
		: m_Function(function) {}

	virtual void Execute() const override { m_Function(); };

private:
	void(*m_Function)();
};

class ExecutionGraph : public IExecutionNode
{
public:
	ExecutionGraph() : m_DebugName("UNNAMED") {}
	ExecutionGraph(const std::string& debugName) : m_DebugName(debugName) {}

	template<EDependencyType DependencyType>
	void AddDependency(const IExecutionNode* dependent, const IExecutionNode* dependency)
	{
		m_ExecutionDependencies.push_back(ExecutionDependency(DependencyType, dependent, dependency));
	}

	virtual void Execute() const override
	{
		for (const IExecutionNode* executionNode : m_ExecutionNodes)
		{
			executionNode->Execute();
		}
	}

	IExecutionNode* AppendTask(void(*function)())
	{
		ExecutionTask* executionNode = new ExecutionTask(function);
		m_ExecutionNodes.push_back(static_cast<IExecutionNode*>(executionNode));
		return executionNode;
	}

	IExecutionNode* AppendGraph(ExecutionGraph* executionGraph)
	{
		m_ExecutionNodes.push_back(static_cast<IExecutionNode*>(executionGraph));
		return executionGraph;
	}

private:
	std::deque<IExecutionNode*> m_ExecutionNodes;
	std::deque<ExecutionDependency> m_ExecutionDependencies;

	const std::string m_DebugName;
};

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

class ExecutionScheduler
{
public:
	static void Run()
	{
		uint32_t threadCount = std::max(1u, std::thread::hardware_concurrency());

		std::cout << (std::ostringstream{} << "Thread count: " << threadCount << '\n').str();

		for (uint32_t i = 0; i < threadCount; ++i)
		{
			std::thread worker([](uint32_t threadIndex) {
				while (true)
				{
					// std::this_thread::sleep_for(std::chrono::milliseconds(1));
					
					// std::cout << (std::ostringstream{} << "Thread " << threadIndex << '\n').str();
					while (m_ExecutionNodes.size() > 0)
					{
						IExecutionNode* executionNode = nullptr;

						m_ExecutionNodesLock.Lock();
						if (m_ExecutionNodes.size() > 0)
						{
							executionNode = std::move(m_ExecutionNodes.front());
							m_ExecutionNodes.pop_front();
						}
						m_ExecutionNodesLock.Unlock();

						if (executionNode)
						{
							std::cout << (std::ostringstream{} << "[" << threadIndex << "] Executing Node | Node: " << reinterpret_cast<void*>(executionNode) << '\n').str();
							executionNode->Execute();
						}
					}
				}
				}, i);

			worker.detach();
		}

		while (true) {}
	}

	static void AppendGraph(ExecutionGraph* executionGraph)
	{
		m_ExecutionNodesLock.Lock();
		std::cout << (std::ostringstream{} << "Appending Graph | Address: " << reinterpret_cast<void*>(executionGraph) << '\n').str();
		m_ExecutionNodes.push_back(static_cast<IExecutionNode*>(executionGraph));
		m_ExecutionNodesLock.Unlock();
	}

private:
	static std::deque<IExecutionNode*> m_ExecutionNodes;
	static SpinLock m_ExecutionNodesLock;
};

std::deque<IExecutionNode*> ExecutionScheduler::m_ExecutionNodes = std::deque<IExecutionNode*>();
SpinLock ExecutionScheduler::m_ExecutionNodesLock = SpinLock();

/// EXAMPLE

class RendererModule
{
public:
	static void BuildSceneData() { std::cout << "RendererModule::BuildSceneData()" << std::endl; }
	static void BuildCommandBuffers() { std::cout << "RendererModule::BuildCommandBuffers()" << std::endl; }
	static void RenderScene() { std::cout << (std::ostringstream{} << "RendererModule::RenderScene()" << '\n').str(); }
	static void QueueNextFrame()
	{
		std::cout << "RendererModule::QueueNextFrame()" << std::endl;
		ExecutionScheduler::AppendGraph(&m_RendererLoopExecutionGraph);
	}

	static void Initialize()
	{
		IExecutionNode* buildSceneDataNode = m_RendererLoopExecutionGraph.AppendTask(&RendererModule::BuildSceneData);
		IExecutionNode* buildCommandBuffersNode = m_RendererLoopExecutionGraph.AppendTask(&RendererModule::BuildCommandBuffers);
		IExecutionNode* renderSceneNode = m_RendererLoopExecutionGraph.AppendTask(&RendererModule::RenderScene);
		IExecutionNode* queueNextFrameNode = m_RendererLoopExecutionGraph.AppendTask(&RendererModule::QueueNextFrame);
		m_RendererLoopExecutionGraph.AddDependency<EDependencyType::Precede>(buildCommandBuffersNode, buildSceneDataNode);
		m_RendererLoopExecutionGraph.AddDependency<EDependencyType::Precede>(renderSceneNode, buildCommandBuffersNode);
		m_RendererLoopExecutionGraph.AddDependency<EDependencyType::Precede>(queueNextFrameNode, renderSceneNode);
		//m_RendererLoopExecutionGraph.AddDependency<EDependencyType::Conditional>(queueNextFrameNode, RendererModule::ShouldQueueNextFrame);

		ExecutionScheduler::AppendGraph(&m_RendererLoopExecutionGraph);
	}

	static void Terminate() { /* ... */ }

private:
	static ExecutionGraph m_RendererLoopExecutionGraph;
};

ExecutionGraph RendererModule::m_RendererLoopExecutionGraph = ExecutionGraph("Renderer Loop");

class SceneModule
{
public:
	static void Initialize() { /* ... */ }

	static void Terminate() { /* ... */ }
};

class Application
{
public:
	void Run()
	{
		IExecutionNode* rendererModuleInitializationNode = m_ApplicationInitializationExecutionGraph.AppendTask(&RendererModule::Initialize);
		IExecutionNode* sceneModuleInitializationNode = m_ApplicationInitializationExecutionGraph.AppendTask(&SceneModule::Initialize);
		m_ApplicationInitializationExecutionGraph.AddDependency<EDependencyType::Precede>(rendererModuleInitializationNode, sceneModuleInitializationNode);

		IExecutionNode* rendererModuleTerminationNode = m_ApplicationTerminationExecutionGraph.AppendTask(&RendererModule::Terminate);
		IExecutionNode* sceneModuleTerminationNode = m_ApplicationTerminationExecutionGraph.AppendTask(&SceneModule::Terminate);
		m_ApplicationTerminationExecutionGraph.AddDependency<EDependencyType::Succeeds>(rendererModuleTerminationNode, sceneModuleTerminationNode);

		IExecutionNode* initializationNode = m_ApplicationMasterExecutionGraph.AppendGraph(&m_ApplicationInitializationExecutionGraph);
		IExecutionNode* terminationNode = m_ApplicationMasterExecutionGraph.AppendGraph(&m_ApplicationTerminationExecutionGraph);
		//m_ApplicationMasterExecutionGraph.AddDependency<EDependencyType::Conditional>(terminationNode, &Core::ShouldClose);
		m_ApplicationMasterExecutionGraph.AddDependency<EDependencyType::Precede>(terminationNode, initializationNode);

		ExecutionScheduler::AppendGraph(&m_ApplicationMasterExecutionGraph);
		ExecutionScheduler::Run();
	}

private:
	ExecutionGraph m_ApplicationInitializationExecutionGraph = ExecutionGraph("Application Initialization");
	ExecutionGraph m_ApplicationTerminationExecutionGraph = ExecutionGraph("Application Termination");
	ExecutionGraph m_ApplicationMasterExecutionGraph = ExecutionGraph("Application Master");
};

int main(int argc, char** argv)
{
	Application app;
	app.Run();
}