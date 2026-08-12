#include "ThreadPool.h"
#include "Worker.h"

// 初始化线程池
int threadPoolInit(threadPool_t* pthreadPool, int workerNum)
{
	// 初始化子线程信息
	tidArrInit(&pthreadPool->tidArr, workerNum);

	// 初始化任务队列
	taskQueueInit(&pthreadPool->taskQueue);

	// 初始化锁
	pthread_mutex_init(&pthreadPool->mutex, NULL);

	//初始化条件变量
	pthread_cond_init(&pthreadPool->cond, NULL);

	// 初始化退出标志位
	pthreadPool->exitFlag = 0;

	return 0;
}

// 创建子线程
int makeWorker(threadPool_t* pthreadPool)
{
	for (int i = 0; i < pthreadPool->tidArr.workerNum; ++i) {
		pthread_create(&pthreadPool->tidArr.arr[i], NULL, threadFunc, pthreadPool);
	}

	return 0;
}