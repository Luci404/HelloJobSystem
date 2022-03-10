#include <iostream>
#include <thread>
#include <algorithm>
#include <sstream>

class ThreadPool
{
public:
    ThreadPool()
    {
        m_ThreadCount = std::max(1u, std::thread::hardware_concurrency());

        std::cout << (std::ostringstream{} << "Thread count: " << m_ThreadCount << '\n').str();

        for (uint32_t i = 0; i < m_ThreadCount; ++i)
        {
            std::thread worker([](uint32_t threadID) {
                while (true)
                {
                    std::cout << (std::ostringstream{} << "Thread " << threadID << '\n').str();
                }
            }, i);

            worker.detach();
        }
    }

private:
    uint32_t m_ThreadCount;
};

class Application
{
public:
    Application() 
    {
        while (true)
        {
            
        }
    }

private:
    ThreadPool m_ThreadPool;
};

int main(int argc, char** argv)
{
    Application application = Application();
}   