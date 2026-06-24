#include "TaskQueue.h"

//任务队列构造
template <typename T>
TaskQueue<T>::TaskQueue()
{
	pthread_mutex_init(&m_mutex, NULL);
}

//任务队列析构
template <typename T>
TaskQueue<T>::~TaskQueue()
{
	pthread_mutex_destroy(&m_mutex);
}

// 添加任务 1
template <typename T>
void TaskQueue<T>::addTask(Task<T> task)
{
	pthread_mutex_lock(&m_mutex);

	m_taskQ.emplace(task);

	pthread_mutex_unlock(&m_mutex);
}

// 添加任务 2
template <typename T>
void TaskQueue<T>::addTask(callback f, void* arg)
{
	pthread_mutex_lock(&m_mutex);

	m_taskQ.emplace(Task<T>(f, arg));

	pthread_mutex_unlock(&m_mutex);
}

// 取出任务
template <typename T>
Task<T> TaskQueue<T>::takeTask()
{
	Task<T> t;

	pthread_mutex_lock(&m_mutex);

	if (!m_taskQ.empty())
	{
		t = m_taskQ.front();
		m_taskQ.pop();
	}

	pthread_mutex_unlock(&m_mutex);

	return t;
}

//获取当前任务的个数
template <typename T>
size_t TaskQueue<T>::taskNumber()
{
	return m_taskQ.size();
}