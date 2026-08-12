#include "Head.h"

// 进程池退出
int exitPipe[2];
void handler(int signum)
{
	// 内核会主动调用 handler 函数，并且把当前触发的信号编号（也就是 10）作为实参，传递给形参 signum
	printf("signum = %d\n", signum);
	write(exitPipe[1], "1", 1);
}

int main(int argc, char* argv[])
{
	// ./server 192.168.248.131 10000 3
	ARGS_CHECK(agrc, 4);

	// 创建子进程，获取每个子进程的状态
	int workerNum = atoi(argv[3]);
	workerdata_t* workerArr = (workerdata_t*)calloc(workerNum, sizeof(workerdata_t));
	makeWorker(workerNum, workerArr);

	// 创建一个自己和自己通信的管道，以便进程池退出
	pipe(exitPipe);

	// 注册 10 号信号，以便进程池退出
	signal(SIGUSR1, handler);
	
	// 初始化 tcp 服务端
	int sockfd;
	tcpInit(argv[1], argv[2], &sockfd);

	// 初始化服务端
	int epfd = epoll_create(1);
	epollAdd(epfd, sockfd);

	// 监听每个子进程的 pipesockfa，因为子进程要想父进程发完成任务的消息
	for (int i = 0; i < workerNum; i++)
	{
		epollAdd(epfd, workerArr[i].pipesockfd);
	}
	
	// 监听进程池推出信号
	epollAdd(epfd, exitPipe[0]);
	 
	while (1) {
		struct epoll_event readySet[1024];
		int readyNum = epoll_wait(epfd, readySet, 1024, -1);
		for (int i = 0; i < readyNum; ++i) {
			if (readySet[i].data.fd == sockfd) {
				int netfd = accept(sockfd, NULL, NULL);
				printf("client is connected!");

				// 顺序查找一个空闲的子进程
				for (int j = 0; j < workerNum; ++j) {
					if (workerArr[j].status == FREE) {
						sendfd(workerArr[j].pipesockfd, netfd, 0);
						workerArr[j].status = BUSY;

						break;
					}
				}

				// 关闭子线程连接
				close(netfd);
			}
			else if(readySet[i].data.fd == exitPipe[0]){
				// 进程池退出
				for (int j = 0; j < workerNum; ++j) {
					//kill(workerArr[j].pid, SIGUSR1); // 强制推出
					sendfd(workerArr[j].pipesockfd, 0, 1);// 等待子进程任务执行完毕退出

					printf("kill 1 worker!\n");
				}
				for (int j = 0; j < workerNum; ++j) {
					wait(NULL);
				}

				printf("All workers are killed!\n");

				exit(0);
			}
			else {
				// 某个子进程完成任务了
				int pipesockfd = readySet[i].data.fd;
				
				// 顺序查找从 fd 找到该完成任务的子进程
				for (int j = 0; j < workerNum; j++) {
					if (workerArr[j].pipesockfd == pipesockfd) {
						pid_t pid;
						recv(pipesockfd, &pid, sizeof(pid), 0);
						printf("%d worker, pid = %d\n", j, pid);
						
						workerArr[j].status = FREE;

						break;
					}
				}
			}
		}
	}

	return 0; 
}

