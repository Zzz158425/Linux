#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <chrono>

class taskQueue
{
public:
	taskQueue(int size) : maxSize(size) {}

	// 生产者
	void put(int num)
	{
		std::unique_lock<std::mutex> locker(m_mutex);

		while (m_queue.size() == maxSize)
		{
			std::cout << "任务队列已满！！请等待后再生产！！ " << std::this_thread::get_id() << std::endl;
			full.wait(locker);
		}
		m_queue.push(num);
		std::cout << "生产线程：" << std::this_thread::get_id() << " num = " << num << " 已入队列！！" << std::endl;
		
		std::this_thread::sleep_for(std::chrono::seconds(1));

		empty.notify_all();
	}

	// 消费者
	void take()
	{
		std::unique_lock<std::mutex> locker(m_mutex);
		
		while (!m_queue.size())
		{
			std::cout << "任务队列为空！！请等待后再消费！！ " << std::this_thread::get_id() << std::endl;
			empty.wait(locker);
		}
		int num = m_queue.front();
		m_queue.pop();
		std::cout << "消费线程：" << std::this_thread::get_id() << " num = " << num << " 已出队列！！" << std::endl;
		
		std::this_thread::sleep_for(std::chrono::seconds(1));
		
		full.notify_all();
	}

	bool isEmpty()
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		return !m_queue.size();
	}

	bool isFull()
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		return m_queue.size() == maxSize;
	}

	int size()
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		return m_queue.size();
	}

private:
	int maxSize;

	std::mutex m_mutex;

	std::condition_variable full;
	std::condition_variable empty;

	std::queue<int> m_queue;
};

int main() 
{
	taskQueue task(5);

	// 把 taskQueue 类中的成员函数绑定到具体对象 task 上，变成可以直接调用的函数对象
	// 因为 put 和 take 是 成员函数，它们必须依赖某个具体对象才能调用，不能直接像普通函数一样调用
	// 所以这里用 std::bind 把成员函数和对象 task 绑定起来
	std::function<void(int)> producer = std::bind(&taskQueue::put, &task, std::placeholders::_1);
	std::function<void(void)> consumer = std::bind(&taskQueue::take, &task);

	std::thread thP[20], thC[20];

	for (int i = 0; i < 20; ++i)
	{
		thP[i] = std::thread(producer, i + 1000);
		thC[i] = std::thread(consumer);
	}
	for (int i = 0; i < 20; ++i)
	{
		thP[i].join();
		thC[i].join();
	}

	return 0;
}