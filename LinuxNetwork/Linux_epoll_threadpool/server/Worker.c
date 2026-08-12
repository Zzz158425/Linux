#include "Worker.h"
#include "ThreadPool.h"
#include "TaskQueue.h"

// 初始化
int tidArrInit(tidArr_t* ptidArr, int workerNum)
{
	// 申请内存，存储每个子线程的 tid
	ptidArr->arr = (pthread_t*)calloc(workerNum, sizeof(pthread_t));
	ptidArr->workerNum = workerNum;

	return 0;
}

// 解锁
//void unlock(void* arg)
//{
//	threadPool_t* pthreadPool = (threadPool_t*)arg;
//	printf("unlock!\n");
//	pthread_mutex_unlock(&pthreadPool->mutex);
//}

// 任务函数
void* threadFunc(void* arg)
{
	threadPool_t* pthreadPool = (threadPool_t*)arg;
	while (1) {
		pthread_mutex_lock(&pthreadPool->mutex);
		int netfd;
		// 使用 pthread_cancel 强制退出
		// 使用资源清理函数，否则退出的时候会出现子线程 wait 之后不解锁立即退出，造成死锁
		//pthread_cleanuo_push(unlock, pthreadPool);

		while (pthreadPool->exitFlag == 0 && pthreadPool->taskQueue.queueSize <= 0) {
			pthread_cond_wait(&pthreadPool->cond, &pthreadPool->mutex);
		}

			// 判断是否需要退出
			if (pthreadPool->exitFlag == 1) {
				printf("thread is gonging to exit!\n");

				pthread_mutex_unlock(&pthreadPool->mutex);
				pthread_exit(NULL);
			}
			
			// 拿取任务
			netfd = pthreadPool->taskQueue.pFront->netfd;
			printf("start work, netfd = %d\n", netfd);
			deQueue(&pthreadPool->taskQueue);

			pthread_mutex_unlock(&pthreadPool->mutex);
			//pthread_cleanuo_pop(1);

			// 执行业务
			transFile(netfd);
			printf("finish work, netfd = %d\n", netfd);

			close(netfd);
	}
}
