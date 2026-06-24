#include "ThreadPool.h"


// 线程池构造
template <typename T>
ThreadPool<T>::ThreadPool(int min, int max)
	: minNum(min)
	, maxNum(max)
	, busyNum(0)
	, liveNum(min)
	, exitNum(0)
	, shutdown(false)
{
	// 实例化任务队列
	do
	{
		taskQ = new TaskQueue<T>;
		if (taskQ == nullptr)
		{
			std::cout << "malloc taskQ fail..." << std::endl;
			break;
		}

		threadIDs = new pthread_t[max]();
		if (threadIDs == nullptr)
		{
			std::cout << "malloc threadIDs fail..." << std::endl;
			break;
		}

		// pthread 系列函数一般返回 0 表示成功，返回非 0 表示失败
		if (pthread_mutex_init(&mutexPool, NULL) != 0 ||
			pthread_cond_init(&notEmpty, NULL) != 0)
		{
			std::cout << "mutex or condition init fail..." << std::endl;
			break;
		}

		// 创建线程
		pthread_create(&managerID, NULL, manager, this);
		for (int i = 0; i < min; i++)
		{
			pthread_create(&threadIDs[i], NULL, worker, this);
		}

		return;

	} while (false);

	// 初始化失败，释放资源
	if (threadIDs)
	{
		delete[]threadIDs;
	}
	if (taskQ)
	{
		delete taskQ;
	}
}

// 线程池析构
template <typename T>
ThreadPool<T>::~ThreadPool()
{
	// 关闭线程池
	shutdown = true;

	// 阻塞回收管理者线程
	pthread_join(managerID, NULL);

	// 唤醒阻塞的消费者线程
	for (int i = 0; i < liveNum; i++)
	{
		pthread_cond_signal(&notEmpty);
	}

	// 等待 worker 真正退出
	for (int i = 0; i < maxNum; i++)
	{
		if (threadIDs[i] != 0)
		{
			pthread_join(threadIDs[i], NULL);
		}
	}

	//释放堆内存
	if (taskQ)
	{
		delete taskQ;
	}
	if (threadIDs)
	{
		delete[]threadIDs;
	}

	pthread_mutex_destroy(&mutexPool);
	pthread_cond_destroy(&notEmpty);
}

// 添加任务
template <typename T>
void ThreadPool<T>::addTask(Task<T> task)
{
	if (shutdown)
	{
		return;
	}

	taskQ->addTask(task);// 添加任务

	pthread_cond_signal(&notEmpty);

}

// 获取忙线程的个数
template <typename T>
int ThreadPool<T>::getBusyNumber()
{
	pthread_mutex_lock(&mutexPool);

	busyNum = this->busyNum;

	pthread_mutex_unlock(&mutexPool);

	return busyNum;
}

// 获取活着的线程个数
template <typename T>
int ThreadPool<T>::getAliveNumber()
{
	pthread_mutex_lock(&mutexPool);

	liveNum = this->liveNum;

	pthread_mutex_unlock(&mutexPool);

	return liveNum;
}

// 工作的线程的任务函数
template <typename T>
void* ThreadPool<T>::worker(void* arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);

	while (true)
	{
		pthread_mutex_lock(&pool->mutexPool);

		// 当前任务队列是否为空
		while (pool->taskQ->taskNumber() == 0 && !pool->shutdown)
		{
			// 阻塞工作线程
			pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);

			// 判断是不是要销毁线程
			if (pool->exitNum > 0)
			{
				pool->exitNum--;
				if (pool->liveNum > pool->minNum)
				{
					pool->liveNum--;

					pthread_mutex_unlock(&pool->mutexPool);

					pool->threadExit();
				}
			}
		}

		// 判断线程池是否被关闭了
		if (pool->shutdown)
		{
			pthread_mutex_unlock(&pool->mutexPool);
			pool->threadExit();
		}

		// 从任务队列中取出一个任务
		Task<T> task;
		task = pool->taskQ->takeTask();

		pool->busyNum++;

		pthread_mutex_unlock(&pool->mutexPool);

		std::cout << "thread: " << std::to_string(pthread_self()) << " start working..." << std::endl;

		task.function(task.arg);// 执行任务
		delete task.arg;// 释放 main.cpp 中 new 出来的资源，即 int* num = new int(i + 100);
		task.arg = nullptr;

		std::cout << "thread: " << std::to_string(pthread_self()) << " end working..." << std::endl;

		pthread_mutex_lock(&pool->mutexPool);

		pool->busyNum--;

		pthread_mutex_unlock(&pool->mutexPool);
	}

	return nullptr;
}

// 管理者线程的任务函数
template <typename T>
void* ThreadPool<T>::manager(void* arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);

	while (!pool->shutdown)
	{
		// 每 3s 检测一次
		sleep(3);

		// 取出线程池中任务的数量、当前线程的数量和忙的线程的数量
		pthread_mutex_lock(&pool->mutexPool);

		int queueSize = pool->taskQ->taskNumber();
		int liveNum = pool->liveNum;
		int busyNum = pool->busyNum;

		pthread_mutex_unlock(&pool->mutexPool);

		// 添加线程
		// 任务的个数 > 存活的线程个数 && 存活的线程数 < 最大线程数
		if (queueSize > liveNum && liveNum < pool->maxNum)
		{
			pthread_mutex_lock(&pool->mutexPool);

			// 创建新线程的个数由多因素决定
			int counter = 0;
			for (int i = 0; i < pool->maxNum && counter < NUMBER
				&& pool->liveNum < pool->maxNum; i++)
			{
				if (pool->threadIDs[i] == 0)
				{
					pthread_create(&pool->threadIDs[i], NULL, worker, pool);
					counter++;
					pool->liveNum++;
				}
			}

			pthread_mutex_unlock(&pool->mutexPool);
		}

		// 销毁线程
		// 忙的线程 * 2 < 存活的线程个数 && 存活的线程数 > 最小线程数
		if (busyNum * 2 < liveNum && liveNum > pool->minNum)
		{
			pthread_mutex_lock(&pool->mutexPool);

			pool->exitNum = NUMBER;

			pthread_mutex_unlock(&pool->mutexPool);

			// 让工作的线程自杀
			for (int i = 0; i < NUMBER; i++)
			{
				pthread_cond_signal(&pool->notEmpty);
			}
		}
	}
	return nullptr;
}

// 单个线程退出
template <typename T>
void ThreadPool<T>::threadExit()
{
	pthread_t tid = pthread_self();

	for (int i = 0; i < maxNum; i++)
	{
		if (threadIDs[i] == tid)
		{
			threadIDs[i] = 0;
			std::cout << "threadExit() called, " << std::to_string(tid) << "exiting..." << std::endl;
			break;
		}
	}

	pthread_exit(NULL);
}

