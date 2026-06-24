#pragma once
#include<queue>
#include<pthread.h>

using callback = void (*)(void* arg);

// 任务结构体
template <typename T>
struct Task
{
	Task<T>()
		: function(nullptr)
		, arg(nullptr) { }
	Task<T>(callback f, void* arg)
	{
		this->function = f;
		this->arg = (T*)arg;
	}
	callback function;
	T* arg;
};

// 任务队列
template <typename T>
class TaskQueue
{
public:
	//任务队列构造
	TaskQueue();

	//任务队列析构
	~TaskQueue();

	// 添加任务 1
	void addTask(Task<T> task);

	// 添加任务 2
	void addTask(callback f, void* arg);

	// 取出任务
	Task<T> takeTask();

	//获取当前任务的个数
	size_t taskNumber();

private:
	pthread_mutex_t m_mutex;

	std::queue<Task<T>>m_taskQ;
};