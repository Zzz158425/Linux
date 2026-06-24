#pragma once
#include "TaskQueue.h"
#include "TaskQueue.cpp"
#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>

// 线程池
template <typename T>
class ThreadPool
{
public:
	// 线程池构造
	ThreadPool(int min, int max);

	// 线程池析构
	~ThreadPool();

	// 添加任务
	void addTask(Task<T> task);

	// 获取忙线程的个数
	int getBusyNumber();

	// 获取活着的线程个数
	int getAliveNumber();

private:
	// 工作的线程的任务函数
	static void* worker(void* arg);

	// 管理者线程的任务函数
	static void* manager(void* arg);

	// 单个线程退出
	void threadExit();

private:
	TaskQueue<T>* taskQ;

	pthread_t managerID; // 管理者线程 ID
	pthread_t* threadIDs; // 工作的线程 ID

	int minNum; // 最小线程数量
	int maxNum; // 最大线程数量
	int busyNum; // 忙的线程数量
	int liveNum; // 存活的线程的个数
	int exitNum; // 要销毁的线程个数

	pthread_mutex_t mutexPool; // 锁整个的线程池
	pthread_cond_t notEmpty; // 任务队列是否空了

	static const int NUMBER = 2;

	bool shutdown; // 是否销毁线程池，销毁为 1，否则为 0
};