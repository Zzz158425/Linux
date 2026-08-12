#include "TaskQueue.h"
#include<string.h>

// 初始化任务队列
int taskQueueInit(taskQueue_t* pqueue)
{
	bzero(pqueue, sizeof(taskQueue_t));

	return 0;
}

// 任务队列入队
void enQueue(taskQueue_t* pqueue, int netfd)
{
	// 初始化要插入的节点
	node_t* pNew = (node_t*)calloc(1, sizeof(node_t));
	pNew->netfd = netfd;

	// 尾插
	if (pqueue->queueSize == 0) {
		pqueue->pFront = pNew;
		pqueue->pRear = pNew;
	}
	else {
		pqueue->pRear->pNext = pNew;
		pqueue->pRear = pNew;
	}

	++pqueue->queueSize;
}

// 任务队列出队
void deQueue(taskQueue_t* pqueue)
{
	node_t* pCur = pqueue->pFront;
	pqueue->pFront = pCur->pNext;
	
	// 头删
	if (pqueue->queueSize == 1) {
		pqueue->pRear = NULL;
	}

	free(pCur);

	--pqueue->queueSize;
}