#ifndef __TASKQUEUE__
#define __TASKQUEUE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <syslog.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>

#define ARGS_CHECK(agrc, num) {if(argc != num){fprintf(stderr,"args error!\n");return -1;}}
#define ERROR_CHECK(ret, num, msg) {if(ret == num){perror(msg);return -1;}}

// 节点
typedef struct node_s
{
	int netfd;
	struct node_s* pNext;
} node_t;
// 任务队列
typedef struct taskQueue_s
{
	node_t* pFront;
	node_t* pRear;
	int queueSize;
} taskQueue_t;

// 初始化任务队列
int taskQueueInit(taskQueue_t* pqueue);

// 任务队列入队
void enQueue(taskQueue_t* pqueue, int netfd);

// 任务队列出队
void deQueue(taskQueue_t* pqueue);

#endif // __TASKQUEUE__