#ifndef __THREADPOOL__
#define __THREADPOOL__

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

#include "Worker.h"
#include "TaskQueue.h"

#define ARGS_CHECK(agrc, num) {if(argc != num){fprintf(stderr,"args error!\n");return -1;}}
#define ERROR_CHECK(ret, num, msg) {if(ret == num){perror(msg);return -1;}}

// 线程池
typedef struct threadPool_s
{
	// 记录所有子线程信息
	tidArr_t tidArr;

	// 任务队列
	taskQueue_t taskQueue;

	// 锁
	pthread_mutex_t mutex;

	// 条件变量
	pthread_cond_t cond;

	// 退出标志位
	int exitFlag;
} threadPool_t;

// 初始化线程池
int threadPoolInit(threadPool_t* pthreadPool, int workerNum);

// 创建子线程
int makeWorker(threadPool_t* pthreadPool);

// 初始化 TCP 连接
int tcpInit(const char* ip, const char* port, int* psockfd);

// 发送文件
int transFile(int netfd);

// 增加 epoll 监听
int epollAdd(int epfd, int fd);

// 删除 epoll 监听
int epollDel(int epfd, int fd);

#endif // __THREADPOOL__
