#include "Worker.h"
#include "ThreadPool.h"
#include "TaskQueue.h"

// 退出线程池
int exitPipe[2];
void handler(int signum)
{
	printf("signum = %d\n", signum);
	write(exitPipe[1], "1", 1);
}

int main(int argc, char* argv[])
{
	// .server  192.168.248.131 10000 4
	ARGS_CHECK(argc, 4);

	// 创建发送退出信号的管道
	// 父进程控制是否退出，子进程实现线程池，与进程池退出机制一致
	pipe(exitPipe);
	if (fork() != 0) {
		close(exitPipe[0]);// 关闭读端
		signal(SIGUSR1, handler);// 注册 SICUSR1 信号
		wait(NULL);// 让父进程一直等待，只有子进程才能执行后续代码
		printf("The program is going to exit!\n");

		exit(0);
	}

	// 关闭子进程写端
	close(exitPipe[1]);

	// 初始化线程池
	threadPool_t threadPool;
	threadPoolInit(&threadPool, atoi(argv[3]));

	// 创建子线程
	makeWorker(&threadPool);

	// 初始化 TCP 连接
	int sockfd;
	tcpInit(argv[1], argv[2], &sockfd);

	// 创建 epoll 并把 sockfd 加入监听
	int epfd = epoll_create(1);
	epollAdd(epfd, sockfd);

	// 监听退出消息，子进程读端
	epollAdd(epfd, exitPipe[0]);

	while (1) {
		struct epoll_event readySet[1024];
		int readyNum = epoll_wait(epfd, readySet, 1024, -1);
		for (int i = 0; i < readyNum; i++) {
			if (readySet[i].data.fd == sockfd) {
				int netfd = accept(sockfd, NULL, NULL);
				printf("client is connected!");

				// 分配任务
				pthread_mutex_lock(&threadPool.mutex);

				enQueue(&threadPool.taskQueue, netfd);
				printf("assign tasks, netfd = %d\n", netfd);
				pthread_cond_signal(&threadPool.cond);

				pthread_mutex_unlock(&threadPool.mutex);
			}
			else if(readySet[i].data.fd == exitPipe[0]){
				// 线程池将要退出
				printf("threadPool is going to exit!\n");

				// 使用标志位退出，可以使任务执行完再退出
				pthread_mutex_lock(&threadPool.mutex);
				threadPool.exitFlag = 1;
				pthread_cond_broadcast(&threadPool.cond);
				pthread_mutex_unlock(&threadPool.mutex);

				// 使用 pthread_cancel 强制退出
				//for (int j = 0; j < threadPool.tidArr.workerNum; ++j) {
				//	pthread_cancel(threadPool.tidArr.arr[j]);
				//}
				  
				for (int j = 0; j < threadPool.tidArr.workerNum; ++j)
				{
					pthread_join(threadPool.tidArr.arr[j], NULL);
				}

				printf("threadPool is going to exit!\n");

				exit(0);
			}
		}
	}

	return 0; 
}

